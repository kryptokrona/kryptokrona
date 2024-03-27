// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once
#include <map>
#include "cryptonote_basic_impl.h"
#include <logging/logger_ref.h>

namespace cryptonote
{
    class Checkpoints
    {
    public:
        Checkpoints(std::shared_ptr<logging::ILogger> log);

        bool addCheckpoint(uint32_t index, const std::string &hash_str);
        bool loadCheckpointsFromFile(const std::string &fileName);
        bool isInCheckpointZone(uint32_t index) const;
        bool checkBlock(uint32_t index, const crypto::Hash &h) const;
        bool checkBlock(uint32_t index, const crypto::Hash &h, bool &isCheckpoint) const;

    private:
        std::map<uint32_t, crypto::Hash> points;
        logging::LoggerRef logger;
    };
}
