// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cryptonote.h>

namespace cryptonote
{

    class IMainChainStorage
    {
    public:
        virtual ~IMainChainStorage() {}

        virtual void pushBlock(const RawBlock &rawBlock) = 0;
        virtual void popBlock() = 0;

        virtual RawBlock getBlockByIndex(uint32_t index) const = 0;
        virtual uint32_t getBlockCount() const = 0;

        virtual void clear() = 0;
    };

}
