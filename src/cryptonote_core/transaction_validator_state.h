// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once
#include <set>
#include <unordered_set>
#include "cached_transaction.h"
#include <cryptonote.h>
#include <crypto/crypto.h>

namespace cryptonote
{
    struct TransactionValidatorState {
      std::unordered_set<Crypto::KeyImage> spentKeyImages;
    };

    void mergeStates(TransactionValidatorState& destination, const TransactionValidatorState& source);
    bool hasIntersections(const TransactionValidatorState& destination, const TransactionValidatorState& source);
    void excludeFromState(TransactionValidatorState& state, const CachedTransaction& transaction);
}
