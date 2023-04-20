// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <atomic>
#include <memory>
#include <string>

#include "rocksdb/db.h"

#include "idatabase.h"
#include "database_config.h"

#include <logging/logger_ref.h>

namespace cryptonote
{

    class RocksDBWrapper : public IDataBase
    {
    public:
        RocksDBWrapper(std::shared_ptr<logging::ILogger> logger);
        virtual ~RocksDBWrapper();

        RocksDBWrapper(const RocksDBWrapper &) = delete;
        RocksDBWrapper(RocksDBWrapper &&) = delete;

        RocksDBWrapper &operator=(const RocksDBWrapper &) = delete;
        RocksDBWrapper &operator=(RocksDBWrapper &&) = delete;

        void init(const DataBaseConfig &config);
        void shutdown();
        void destroy(const DataBaseConfig &config); // Be careful with this method!

        std::error_code write(IWriteBatch &batch) override;
        std::error_code read(IReadBatch &batch) override;

    private:
        std::error_code write(IWriteBatch &batch, bool sync);

        rocksdb::Options getDBOptions(const DataBaseConfig &config);
        std::string getDataDir(const DataBaseConfig &config);

        enum State
        {
            NOT_INITIALIZED,
            INITIALIZED
        };

        logging::LoggerRef logger;
        std::unique_ptr<rocksdb::DB> db;
        std::atomic<State> state;
    };
}
