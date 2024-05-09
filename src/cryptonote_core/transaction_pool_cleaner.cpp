// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "transaction_pool_cleaner.h"
#include "core.h"
#include "mixins.h"

#include "common/string_tools.h"

#include <syst/interrupted_exception.h>
#include <syst/timer.h>

namespace cryptonote
{

    using json = nlohmann::json;

    TransactionPoolCleanWrapper::TransactionPoolCleanWrapper(
        std::unique_ptr<ITransactionPool> &&transactionPool,
        std::unique_ptr<ITimeProvider> &&timeProvider,
        std::shared_ptr<logging::ILogger> logger,
        uint64_t timeout)
        : transactionPool(std::move(transactionPool)),
          timeProvider(std::move(timeProvider)),
          logger(logger, "TransactionPoolCleanWrapper"),
          timeout(timeout)
    {

        assert(this->timeProvider);
    }

    TransactionPoolCleanWrapper::~TransactionPoolCleanWrapper()
    {
    }

    bool TransactionPoolCleanWrapper::pushTransaction(CachedTransaction &&tx, TransactionValidatorState &&transactionState)
    {
        return !isTransactionRecentlyDeleted(tx.getTransactionHash()) && transactionPool->pushTransaction(std::move(tx), std::move(transactionState));
    }

    const CachedTransaction &TransactionPoolCleanWrapper::getTransaction(const crypto::Hash &hash) const
    {
        return transactionPool->getTransaction(hash);
    }

    bool TransactionPoolCleanWrapper::removeTransaction(const crypto::Hash &hash)
    {
        return transactionPool->removeTransaction(hash);
    }

    size_t TransactionPoolCleanWrapper::getTransactionCount() const
    {
        return transactionPool->getTransactionCount();
    }

    std::vector<crypto::Hash> TransactionPoolCleanWrapper::getTransactionHashes() const
    {
        return transactionPool->getTransactionHashes();
    }

    bool TransactionPoolCleanWrapper::checkIfTransactionPresent(const crypto::Hash &hash) const
    {
        return transactionPool->checkIfTransactionPresent(hash);
    }

    const TransactionValidatorState &TransactionPoolCleanWrapper::getPoolTransactionValidationState() const
    {
        return transactionPool->getPoolTransactionValidationState();
    }

    std::vector<CachedTransaction> TransactionPoolCleanWrapper::getPoolTransactions() const
    {
        return transactionPool->getPoolTransactions();
    }

    uint64_t TransactionPoolCleanWrapper::getTransactionReceiveTime(const crypto::Hash &hash) const
    {
        return transactionPool->getTransactionReceiveTime(hash);
    }

    std::vector<crypto::Hash> TransactionPoolCleanWrapper::getTransactionHashesByPaymentId(const crypto::Hash &paymentId) const
    {
        return transactionPool->getTransactionHashesByPaymentId(paymentId);
    }

    std::string TransactionPoolCleanWrapper::hex2ascii(const std::string &hex)
    {
        std::string ascii;

        for (size_t i = 0; i < hex.length(); i += 2)
        {
            // taking two characters from hex string
            std::string part = hex.substr(i, 2);
            // changing it into base 16
            char ch = stoul(part, nullptr, 16);
            // putting it into the ASCII string
            ascii += ch;
        }

        return ascii;
    }

    std::vector<crypto::Hash> TransactionPoolCleanWrapper::clean(const uint32_t height)
    {
        try
        {
            uint64_t currentTime = timeProvider->now();
            auto transactionHashes = transactionPool->getTransactionHashes();
            std::vector<crypto::Hash> deletedTransactions;
            uint64_t tx_extra_data_size;
            for (const auto &hash : transactionHashes)
            {
                logger(logging::DEBUGGING) << "Checking transaction " << common::podToHex(hash);
                uint64_t transactionAge = currentTime - transactionPool->getTransactionReceiveTime(hash);
                logger(logging::DEBUGGING) << "Transaction extra size: " << transactionPool->getTransaction(hash).getTransaction().extra.size();
                tx_extra_data_size = transactionPool->getTransaction(hash).getTransaction().extra.size();
                CachedTransaction transaction = transactionPool->getTransaction(hash);
                std::vector<CachedTransaction> transactions;
                transactions.emplace_back(transaction);

                auto [success, error] = Mixins::validate(transactions, height);

                if (!success)
                {
                    logger(logging::INFO) << "Deleting invalid mixin transaction " << common::podToHex(hash) << " from pool..." << error;
                    recentlyDeletedTransactions.emplace(hash, currentTime);
                    transactionPool->removeTransaction(hash);
                    deletedTransactions.emplace_back(std::move(hash));
                }
                if (tx_extra_data_size > 128)
                {
                    if (transactionAge >= cryptonote::parameters::CRYPTONOTE_MEMPOOL_TX_LIVETIME)
                    {
                        logger(logging::DEBUGGING) << "Deleting hugin transaction...";
                        recentlyDeletedTransactions.emplace(hash, currentTime);
                        transactionPool->removeTransaction(hash);
                        deletedTransactions.emplace_back(std::move(hash));
                    }
                }
                else if (transactionAge >= timeout)
                {
                    logger(logging::DEBUGGING) << "Deleting transaction " << common::podToHex(hash) << " from pool.";
                    recentlyDeletedTransactions.emplace(hash, currentTime);
                    transactionPool->removeTransaction(hash);
                    deletedTransactions.emplace_back(std::move(hash));
                }
                else
                {
                    logger(logging::DEBUGGING) << "Transaction " << common::podToHex(hash) << " is cool...";
                }
            }

            logger(logging::DEBUGGING) << "Length of deleted transactions: " << deletedTransactions.size();

            cleanRecentlyDeletedTransactions(currentTime);
            return deletedTransactions;
        }
        catch (syst::InterruptedException &)
        {
            throw;
        }
        catch (std::exception &e)
        {
            logger(logging::WARNING) << "Caught an exception: " << e.what() << ", stopping cleaning procedure cycle";
            throw;
        }
    }

    bool TransactionPoolCleanWrapper::isTransactionRecentlyDeleted(const crypto::Hash &hash) const
    {
        auto it = recentlyDeletedTransactions.find(hash);
        return it != recentlyDeletedTransactions.end() && it->second >= timeout;
    }

    void TransactionPoolCleanWrapper::cleanRecentlyDeletedTransactions(uint64_t currentTime)
    {
        for (auto it = recentlyDeletedTransactions.begin(); it != recentlyDeletedTransactions.end();)
        {
            if (currentTime - it->second >= timeout)
            {
                it = recentlyDeletedTransactions.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    void TransactionPoolCleanWrapper::flush()
    {
        return transactionPool->flush();
    }
}
