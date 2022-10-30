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
#include "cached_transaction.h"

namespace cryptonote
{
    struct TransactionValidatorState;

    class ITransactionPool {
    public:
      virtual ~ITransactionPool() {};

      virtual bool pushTransaction(CachedTransaction&& tx, TransactionValidatorState&& transactionState) = 0;
      virtual const CachedTransaction& getTransaction(const crypto::Hash& hash) const = 0;
      virtual bool removeTransaction(const crypto::Hash& hash) = 0;

      virtual size_t getTransactionCount() const = 0;
      virtual std::vector<crypto::Hash> getTransactionHashes() const = 0;
      virtual bool checkIfTransactionPresent(const crypto::Hash& hash) const = 0;

      virtual const TransactionValidatorState& getPoolTransactionValidationState() const = 0;
      virtual std::vector<CachedTransaction> getPoolTransactions() const = 0;

      virtual uint64_t getTransactionReceiveTime(const crypto::Hash& hash) const = 0;
      virtual std::vector<crypto::Hash> getTransactionHashesByPaymentId(const crypto::Hash& paymentId) const = 0;
    };
}
