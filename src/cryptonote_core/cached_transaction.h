// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <boost/optional.hpp>
#include <cryptonote.h>

namespace cryptonote
{

    class CachedTransaction
    {
    public:
        explicit CachedTransaction(Transaction &&transaction);
        explicit CachedTransaction(const Transaction &transaction);
        explicit CachedTransaction(const BinaryArray &transactionBinaryArray);
        const Transaction &getTransaction() const;
        const crypto::Hash &getTransactionHash() const;
        const crypto::Hash &getTransactionPrefixHash() const;
        const BinaryArray &getTransactionBinaryArray() const;
        uint64_t getTransactionFee() const;

    private:
        Transaction transaction;
        mutable boost::optional<BinaryArray> transactionBinaryArray;
        mutable boost::optional<crypto::Hash> transactionHash;
        mutable boost::optional<crypto::Hash> transactionPrefixHash;
        mutable boost::optional<uint64_t> transactionFee;
    };

}
