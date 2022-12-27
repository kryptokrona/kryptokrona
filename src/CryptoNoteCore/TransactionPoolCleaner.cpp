// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "TransactionPoolCleaner.h"
#include "Core.h"
#include "Mixins.h"

#include "Common/StringTools.h"

#include <System/InterruptedException.h>
#include <System/Timer.h>

namespace CryptoNote
{

    using json = nlohmann::json;

    TransactionPoolCleanWrapper::TransactionPoolCleanWrapper(
        std::unique_ptr<ITransactionPool> &&transactionPool,
        std::unique_ptr<ITimeProvider> &&timeProvider,
        std::shared_ptr<Logging::ILogger> logger,
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

    const CachedTransaction &TransactionPoolCleanWrapper::getTransaction(const Crypto::Hash &hash) const
    {
        return transactionPool->getTransaction(hash);
    }

    bool TransactionPoolCleanWrapper::removeTransaction(const Crypto::Hash &hash)
    {
        return transactionPool->removeTransaction(hash);
    }

    size_t TransactionPoolCleanWrapper::getTransactionCount() const
    {
        return transactionPool->getTransactionCount();
    }

    std::vector<Crypto::Hash> TransactionPoolCleanWrapper::getTransactionHashes() const
    {
        return transactionPool->getTransactionHashes();
    }

    bool TransactionPoolCleanWrapper::checkIfTransactionPresent(const Crypto::Hash &hash) const
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

    uint64_t TransactionPoolCleanWrapper::getTransactionReceiveTime(const Crypto::Hash &hash) const
    {
        return transactionPool->getTransactionReceiveTime(hash);
    }

    std::vector<Crypto::Hash> TransactionPoolCleanWrapper::getTransactionHashesByPaymentId(const Crypto::Hash &paymentId) const
    {
        return transactionPool->getTransactionHashesByPaymentId(paymentId);
    }

    std::string TransactionPoolCleanWrapper::hex2ascii(std::string hex)
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

    json TransactionPoolCleanWrapper::trimExtra(std::string extra)
    {
        try
        {
            std::string payload = hex2ascii(extra.substr(66));
            json payload_json = json::parse(payload);
            return payload_json;
        }
        catch (std::exception &e)
        {
            logger(Logging::DEBUGGING) << "Unable to trim extra data with 66";
        }

        try
        {
            std::string payload = hex2ascii(extra.substr(78));
            json payload_json = json::parse(payload);
            return payload_json;
        }
        catch (std::exception &e)
        {
            logger(Logging::DEBUGGING) << "Unable to trim extra data with 78";
        }

        // returning empty json object if try/catch does not return anything
        std::string payload = "{ 't': 0 }";
        json payload_json = json::parse(payload);
        return payload_json;
    }

    std::vector<Crypto::Hash> TransactionPoolCleanWrapper::clean(const uint32_t height)
    {
        try
        {
            uint64_t currentTime = timeProvider->now();
            auto transactionHashes = transactionPool->getTransactionHashes();
            std::vector<Crypto::Hash> deletedTransactions;
            uint64_t boxed_transaction_age;
            uint64_t transaction_extra_data_size;

            for (const auto &hash : transactionHashes)
            {
                logger(Logging::INFO) << "Checking transaction " << Common::podToHex(hash);
                uint64_t transactionAge = currentTime - transactionPool->getTransactionReceiveTime(hash);
                boxed_transaction_age = 0;
                transaction_extra_data_size = transactionPool->getTransaction(hash).getTransaction().extra.size();
                logger(Logging::DEBUGGING) << "Transaction extra size: " << transactionPool->getTransaction(hash).getTransaction().extra.size();

                CachedTransaction transaction = transactionPool->getTransaction(hash);
                std::vector<CachedTransaction> transactions;
                transactions.emplace_back(transaction);

                auto [success, error] = Mixins::validate(transactions, height);

                if (!success)
                {
                    logger(Logging::INFO) << "Deleting invalid mixin transaction " << Common::podToHex(hash) << " from pool..." << error;
                    recentlyDeletedTransactions.emplace(hash, currentTime);
                    transactionPool->removeTransaction(hash);
                    deletedTransactions.emplace_back(std::move(hash));
                }
                else if (transaction_extra_data_size >= 128)
                {
                    try
                    {
                        // check transaction age
                        std::string extraData = Common::toHex(
                            transactionPool->getTransaction(hash).getTransaction().extra.data(),
                            transaction_extra_data_size);

                        // parse the json
                        json j = trimExtra(extraData);
                        uint64_t box_time = j.at("t").get<uint64_t>();
                        boxed_transaction_age = currentTime - box_time;

                        if (
                            (transactionAge >= CryptoNote::parameters::CRYPTONOTE_MEMPOOL_TX_LIVETIME || boxed_transaction_age >= CryptoNote::parameters::CRYPTONOTE_MEMPOOL_TX_LIVETIME) &&
                            transaction_extra_data_size > CryptoNote::parameters::MAX_EXTRA_SIZE_BLOCK)
                        {
                            logger(Logging::INFO) << "Deleting hugin transaction...";
                            recentlyDeletedTransactions.emplace(hash, currentTime);
                            transactionPool->removeTransaction(hash);
                            deletedTransactions.emplace_back(std::move(hash));
                        }
                    }
                    catch (std::exception &e)
                    {
                        logger(Logging::INFO) << "Unable to remove hugin transaction, not a valid hugin transaction.";
                    }
                }
                else if (transactionAge >= timeout)
                {
                    logger(Logging::INFO) << "Deleting transaction " << Common::podToHex(hash) << " from pool.";
                    recentlyDeletedTransactions.emplace(hash, currentTime);
                    transactionPool->removeTransaction(hash);
                    deletedTransactions.emplace_back(std::move(hash));
                }
                else
                {
                    logger(Logging::DEBUGGING) << "Transaction " << Common::podToHex(hash) << " is cool...";
                }
            }

            logger(Logging::DEBUGGING) << "Length of deleted transactions: " << deletedTransactions.size();

            cleanRecentlyDeletedTransactions(currentTime);
            return deletedTransactions;
        }
        catch (System::InterruptedException &)
        {
            throw;
        }
        catch (std::exception &e)
        {
            logger(Logging::WARNING) << "Caught an exception: " << e.what() << ", stopping cleaning procedure cycle";
            throw;
        }
    }

    bool TransactionPoolCleanWrapper::isTransactionRecentlyDeleted(const Crypto::Hash &hash) const
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

}
