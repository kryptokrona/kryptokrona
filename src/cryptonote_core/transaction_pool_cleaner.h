// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include "itransaction_pool_cleaner.h"

#include <chrono>
#include <unordered_map>

#include "crypto/crypto.h"

#include "cryptonote_core/itime_provider.h"
#include "itransaction_pool.h"
#include "logging/ilogger.h"
#include "logging/logger_ref.h"

#include <system/context_group.h>

namespace cryptonote
{
    class TransactionPoolCleanWrapper: public ITransactionPoolCleanWrapper {
    public:
      TransactionPoolCleanWrapper(
        std::unique_ptr<ITransactionPool>&& transactionPool,
        std::unique_ptr<ITimeProvider>&& timeProvider,
        std::shared_ptr<Logging::ILogger> logger,
        uint64_t timeout);

      TransactionPoolCleanWrapper(const TransactionPoolCleanWrapper&) = delete;
      TransactionPoolCleanWrapper(TransactionPoolCleanWrapper&& other) = delete;

      TransactionPoolCleanWrapper& operator=(const TransactionPoolCleanWrapper&) = delete;
      TransactionPoolCleanWrapper& operator=(TransactionPoolCleanWrapper&&) = delete;

      virtual ~TransactionPoolCleanWrapper();

      virtual bool pushTransaction(CachedTransaction&& tx, TransactionValidatorState&& transactionState) override;
      virtual const CachedTransaction& getTransaction(const Crypto::Hash& hash) const override;
      virtual bool removeTransaction(const Crypto::Hash& hash) override;

      virtual size_t getTransactionCount() const override;
      virtual std::vector<Crypto::Hash> getTransactionHashes() const override;
      virtual bool checkIfTransactionPresent(const Crypto::Hash& hash) const override;

      virtual const TransactionValidatorState& getPoolTransactionValidationState() const override;
      virtual std::vector<CachedTransaction> getPoolTransactions() const override;

      virtual uint64_t getTransactionReceiveTime(const Crypto::Hash& hash) const override;
      virtual std::vector<Crypto::Hash> getTransactionHashesByPaymentId(const Crypto::Hash& paymentId) const override;

      virtual std::vector<Crypto::Hash> clean(const uint32_t height) override;

    private:
      std::unique_ptr<ITransactionPool> transactionPool;
      std::unique_ptr<ITimeProvider> timeProvider;
      Logging::LoggerRef logger;
      std::unordered_map<Crypto::Hash, uint64_t> recentlyDeletedTransactions;
      uint64_t timeout;

      bool isTransactionRecentlyDeleted(const Crypto::Hash& hash) const;
      void cleanRecentlyDeletedTransactions(uint64_t currentTime);
    };
}
