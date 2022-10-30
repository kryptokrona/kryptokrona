// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once
#include <unordered_map>

#include "crypto/crypto.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "itransaction_pool.h"
#include <logging/logger_message.h>
#include <logging/logger_ref.h>
#include "transaction_validator_state.h"

namespace cryptonote
{
    class TransactionPool : public ITransactionPool {
    public:
      TransactionPool(std::shared_ptr<logging::ILogger> logger);

      virtual bool pushTransaction(CachedTransaction&& transaction, TransactionValidatorState&& transactionState) override;
      virtual const CachedTransaction& getTransaction(const crypto::Hash& hash) const override;
      virtual bool removeTransaction(const crypto::Hash& hash) override;

      virtual size_t getTransactionCount() const override;
      virtual std::vector<crypto::Hash> getTransactionHashes() const override;
      virtual bool checkIfTransactionPresent(const crypto::Hash& hash) const override;

      virtual const TransactionValidatorState& getPoolTransactionValidationState() const override;
      virtual std::vector<CachedTransaction> getPoolTransactions() const override;

      virtual uint64_t getTransactionReceiveTime(const crypto::Hash& hash) const override;
      virtual std::vector<crypto::Hash> getTransactionHashesByPaymentId(const crypto::Hash& paymentId) const override;
    private:
      TransactionValidatorState poolState;

      struct PendingTransactionInfo {
        uint64_t receiveTime;
        CachedTransaction cachedTransaction;
        boost::optional<crypto::Hash> paymentId;

        const crypto::Hash& getTransactionHash() const;
      };

      struct TransactionPriorityComparator {
        // lhs > hrs
        bool operator()(const PendingTransactionInfo& lhs, const PendingTransactionInfo& rhs) const;
      };

      struct TransactionHashTag {};
      struct TransactionCostTag {};
      struct PaymentIdTag {};

      typedef boost::multi_index::ordered_non_unique<
        boost::multi_index::tag<TransactionCostTag>,
        boost::multi_index::identity<PendingTransactionInfo>,
        TransactionPriorityComparator
      > TransactionCostIndex;

      typedef boost::multi_index::hashed_unique<
        boost::multi_index::tag<TransactionHashTag>,
        boost::multi_index::const_mem_fun<
          PendingTransactionInfo,
          const crypto::Hash&,
          &PendingTransactionInfo::getTransactionHash
        >
      > TransactionHashIndex;

      struct PaymentIdHasher {
        size_t operator() (const boost::optional<crypto::Hash>& paymentId) const;
      };

      typedef boost::multi_index::hashed_non_unique<
        boost::multi_index::tag<PaymentIdTag>,
        BOOST_MULTI_INDEX_MEMBER(PendingTransactionInfo, boost::optional<crypto::Hash>, paymentId),
        PaymentIdHasher
      > PaymentIdIndex;

      typedef boost::multi_index_container<
        PendingTransactionInfo,
        boost::multi_index::indexed_by<
          TransactionHashIndex,
          TransactionCostIndex,
          PaymentIdIndex
        >
      > TransactionsContainer;

      TransactionsContainer transactions;
      TransactionsContainer::index<TransactionHashTag>::type& transactionHashIndex;
      TransactionsContainer::index<TransactionCostTag>::type& transactionCostIndex;
      TransactionsContainer::index<PaymentIdTag>::type& paymentIdIndex;

      logging::LoggerRef logger;
    };
}
