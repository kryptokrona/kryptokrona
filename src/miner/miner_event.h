// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

namespace miner
{
    enum class MinerEventType : uint8_t
    {
        BLOCK_MINED,
        BLOCKCHAIN_UPDATED,
    };

    struct MinerEvent
    {
        MinerEventType type;
    };
}
