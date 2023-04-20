// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <vector>

#include "cached_transaction.h"
#include "cryptonote.h"
#include "cryptonote_tools.h"

namespace cryptonote
{
    namespace Utils
    {

        bool restoreCachedTransactions(const std::vector<BinaryArray> &binaryTransactions, std::vector<CachedTransaction> &transactions);

    } // namespace Utils
} // namespace cryptonote
