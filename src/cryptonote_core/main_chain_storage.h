// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "imain_chain_storage.h"
#include "currency.h"
#include "swapped_vector.h"

namespace cryptonote
{

    class MainChainStorage : public IMainChainStorage
    {
    public:
        MainChainStorage(const std::string &blocksFilame, const std::string &indexesFilename);
        virtual ~MainChainStorage();

        virtual void pushBlock(const RawBlock &rawBlock) override;
        virtual void popBlock() override;

        virtual RawBlock getBlockByIndex(uint32_t index) const override;
        virtual uint32_t getBlockCount() const override;

        virtual void clear() override;

    private:
        mutable SwappedVector<RawBlock> storage;
    };

    std::unique_ptr<IMainChainStorage> createSwappedMainChainStorage(const std::string &dataDir, const Currency &currency);

}
