// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "iblockchain_cache_factory.h"

#include "blockchain_cache.h"

namespace cryptonote
{

    class MemoryBlockchainCacheFactory : public IBlockchainCacheFactory
    {
    public:
        MemoryBlockchainCacheFactory(const std::string &filename, std::shared_ptr<logging::ILogger> logger);
        virtual ~MemoryBlockchainCacheFactory() override;

        std::unique_ptr<IBlockchainCache> createRootBlockchainCache(const Currency &currency) override;
        std::unique_ptr<IBlockchainCache> createBlockchainCache(const Currency &currency, IBlockchainCache *parent, uint32_t startIndex = 0) override;

    private:
        std::string filename;
        std::shared_ptr<logging::ILogger> logger;
    };

} // namespace cryptonote
