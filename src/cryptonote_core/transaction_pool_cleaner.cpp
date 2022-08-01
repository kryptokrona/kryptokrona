// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "transaction_pool_cleaner.h"
#include "core.h"
#include "mixins.h"

#include "common/string_tools.h"

#include <system/interrupted_exception.h>
#include <system/timer.h>

namespace cryptonote
{
    TransactionPoolCleanWrapper::TransactionPoolCleanWrapper(
      std::unique_ptr<ITransactionPool>&& transactionPool,
      std::unique_ptr<ITimeProvider>&& timeProvider,
      std::shared_ptr<logging::ILogger> logger,
      uint64_t timeout)
      :
      transactionPool(std::move(transactionPool)),
      timeProvider(std::move(timeProvider)),
      logger(logger, "TransactionPoolCleanWrapper"),
      timeout(timeout) {

      assert(this->timeProvider);
    }

    TransactionPoolCleanWrapper::~TransactionPoolCleanWrapper() {
    }

    bool TransactionPoolCleanWrapper::pushTransaction(CachedTransaction&& tx, TransactionValidatorState&& transactionState) {
      return !isTransactionRecentlyDeleted(tx.getTransactionHash()) && transactionPool->pushTransaction(std::move(tx), std::move(transactionState));
    }

    const CachedTransaction& TransactionPoolCleanWrapper::getTransaction(const crypto::Hash& hash) const {
      return transactionPool->getTransaction(hash);
    }

    bool TransactionPoolCleanWrapper::removeTransaction(const crypto::Hash& hash) {
      return transactionPool->removeTransaction(hash);
    }

    size_t TransactionPoolCleanWrapper::getTransactionCount() const {
      return transactionPool->getTransactionCount();
    }

    std::vector<crypto::Hash> TransactionPoolCleanWrapper::getTransactionHashes() const {
      return transactionPool->getTransactionHashes();
    }

    bool TransactionPoolCleanWrapper::checkIfTransactionPresent(const crypto::Hash& hash) const {
      return transactionPool->checkIfTransactionPresent(hash);
    }

    const TransactionValidatorState& TransactionPoolCleanWrapper::getPoolTransactionValidationState() const {
      return transactionPool->getPoolTransactionValidationState();
    }

    std::vector<CachedTransaction> TransactionPoolCleanWrapper::getPoolTransactions() const {
      return transactionPool->getPoolTransactions();
    }

    uint64_t TransactionPoolCleanWrapper::getTransactionReceiveTime(const crypto::Hash& hash) const {
      return transactionPool->getTransactionReceiveTime(hash);
    }

    std::vector<crypto::Hash> TransactionPoolCleanWrapper::getTransactionHashesByPaymentId(const crypto::Hash& paymentId) const {
      return transactionPool->getTransactionHashesByPaymentId(paymentId);
    }

    std::vector<crypto::Hash> TransactionPoolCleanWrapper::clean(const uint32_t height) {
      try {
        uint64_t currentTime = timeProvider->now();
        auto transactionHashes = transactionPool->getTransactionHashes();


        std::vector<crypto::Hash> deletedTransactions;
        for (const auto& hash: transactionHashes) {
          logger(logging::INFO) << "Checking transaction " << common::podToHex(hash);
          uint64_t transactionAge = currentTime - transactionPool->getTransactionReceiveTime(hash);
          if (transactionAge >= timeout) {
            logger(logging::INFO) << "Deleting transaction " << common::podToHex(hash) << " from pool";
            recentlyDeletedTransactions.emplace(hash, currentTime);
            transactionPool->removeTransaction(hash);
            deletedTransactions.emplace_back(std::move(hash));
          } else {
            logger(logging::INFO) << "Transaction " << common::podToHex(hash) << " is cool";
          }

          CachedTransaction transaction = transactionPool->getTransaction(hash);
          std::vector<CachedTransaction> transactions;
          transactions.emplace_back(transaction);

          auto [success, error] = Mixins::validate(transactions, height);

          if (!success)
          {
            logger(logging::INFO) << "Deleting invalid transaction " << common::podToHex(hash) << " from pool." <<
              error;
            recentlyDeletedTransactions.emplace(hash, currentTime);
            transactionPool->removeTransaction(hash);
            deletedTransactions.emplace_back(std::move(hash));
          }
        }

        cleanRecentlyDeletedTransactions(currentTime);
        return deletedTransactions;
      } catch (System::InterruptedException&) {
        throw;
      } catch (std::exception& e) {
        logger(logging::WARNING) << "Caught an exception: " << e.what() << ", stopping cleaning procedure cycle";
        throw;
      }
    }

    bool TransactionPoolCleanWrapper::isTransactionRecentlyDeleted(const crypto::Hash& hash) const {
      auto it = recentlyDeletedTransactions.find(hash);
      return it != recentlyDeletedTransactions.end() && it->second >= timeout;
    }

    void TransactionPoolCleanWrapper::cleanRecentlyDeletedTransactions(uint64_t currentTime) {
      for (auto it = recentlyDeletedTransactions.begin(); it != recentlyDeletedTransactions.end();) {
        if (currentTime - it->second >= timeout) {
          it = recentlyDeletedTransactions.erase(it);
        } else {
          ++it;
        }
      }
    }
}
