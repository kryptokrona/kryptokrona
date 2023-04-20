// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <memory>
#include "itransaction.h"

namespace cryptonote
{
    std::unique_ptr<ITransaction> createTransaction();
    std::unique_ptr<ITransaction> createTransaction(const BinaryArray &transactionBlob);
    std::unique_ptr<ITransaction> createTransaction(const Transaction &tx);

    std::unique_ptr<ITransactionReader> createTransactionPrefix(const TransactionPrefix &prefix, const crypto::Hash &transactionHash);
    std::unique_ptr<ITransactionReader> createTransactionPrefix(const Transaction &fullTransaction);
}
