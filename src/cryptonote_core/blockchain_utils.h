// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <vector>
#include <iterator>
#include <unordered_set>
#include "cached_transaction.h"
#include "cryptonote.h"
#include "cryptonote_tools.h"

namespace cryptonote
{
    namespace Utils
    {

        bool restoreCachedTransactions(const std::vector<BinaryArray> &binaryTransactions, std::vector<CachedTransaction> &transactions);
        /* Verify that the items in a collection are all unique */
        template <typename T>
        bool is_unique(T begin, T end)
        {
            std::unordered_set<typename T::value_type> set{};

            for (; begin != end; ++begin)
            {
                if (!set.insert(*begin).second)
                {
                    return false;
                }
            }

            return true;
        }
    } // namespace Utils
} // namespace cryptonote
