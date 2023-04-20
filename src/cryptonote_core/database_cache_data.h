// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cryptonote_core/blockchain_cache.h>
#include <map>

namespace cryptonote
{

    struct KeyOutputInfo
    {
        crypto::PublicKey publicKey;
        crypto::Hash transactionHash;
        uint64_t unlockTime;
        uint16_t outputIndex;

        void serialize(cryptonote::ISerializer &s);
    };

    // inherit here to avoid breaking IBlockchainCache interface
    struct ExtendedTransactionInfo : CachedTransactionInfo
    {
        // CachedTransactionInfo tx;
        std::map<IBlockchainCache::Amount, std::vector<IBlockchainCache::GlobalOutputIndex>> amountToKeyIndexes; // global key output indexes spawned in this transaction
        void serialize(ISerializer &s);
    };

}
