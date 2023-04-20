// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "iblockchain_cache_factory.h"
#include <logging/logger_message.h>

namespace cryptonote
{

    class IDataBase;

    class DatabaseBlockchainCacheFactory : public IBlockchainCacheFactory
    {
    public:
        explicit DatabaseBlockchainCacheFactory(IDataBase &database, std::shared_ptr<logging::ILogger> logger);
        virtual ~DatabaseBlockchainCacheFactory();

        virtual std::unique_ptr<IBlockchainCache> createRootBlockchainCache(const Currency &currency) override;
        virtual std::unique_ptr<IBlockchainCache> createBlockchainCache(const Currency &currency, IBlockchainCache *parent, uint32_t startIndex = 0) override;

    private:
        IDataBase &database;
        std::shared_ptr<logging::ILogger> logger;
    };

} // namespace cryptonote
