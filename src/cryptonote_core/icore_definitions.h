// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <vector>
#include <cryptonote.h>
#include <crypto_types.h>
#include <wallet_types.h>

namespace cryptonote
{

    struct BlockFullInfo : public RawBlock
    {
        crypto::Hash block_id;
    };

    struct TransactionPrefixInfo
    {
        crypto::Hash txHash;
        TransactionPrefix txPrefix;
    };

    struct BlockShortInfo
    {
        crypto::Hash blockId;
        BinaryArray block;
        std::vector<TransactionPrefixInfo> txPrefixes;
    };

    void serialize(BlockFullInfo &, ISerializer &);
    void serialize(TransactionPrefixInfo &, ISerializer &);
    void serialize(BlockShortInfo &, ISerializer &);

    void serialize(wallet_types::WalletBlockInfo &walletBlockInfo, ISerializer &s);
    void serialize(wallet_types::RawTransaction &rawTransaction, ISerializer &s);
    void serialize(wallet_types::RawCoinbaseTransaction &rawCoinbaseTransaction, ISerializer &s);
    void serialize(wallet_types::KeyOutput &keyOutput, ISerializer &s);

}
