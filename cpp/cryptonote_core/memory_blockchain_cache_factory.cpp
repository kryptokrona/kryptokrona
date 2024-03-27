// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "memory_blockchain_cache_factory.h"

namespace cryptonote
{

    MemoryBlockchainCacheFactory::MemoryBlockchainCacheFactory(const std::string &filename, std::shared_ptr<logging::ILogger> logger) : filename(filename), logger(logger)
    {
    }

    MemoryBlockchainCacheFactory::~MemoryBlockchainCacheFactory()
    {
    }

    std::unique_ptr<IBlockchainCache> MemoryBlockchainCacheFactory::createRootBlockchainCache(const Currency &currency)
    {
        return createBlockchainCache(currency, nullptr, 0);
    }

    std::unique_ptr<IBlockchainCache> MemoryBlockchainCacheFactory::createBlockchainCache(
        const Currency &currency,
        IBlockchainCache *parent,
        uint32_t startIndex)
    {

        return std::unique_ptr<IBlockchainCache>(new BlockchainCache(filename, currency, logger, parent, startIndex));
    }

} // namespace cryptonote
