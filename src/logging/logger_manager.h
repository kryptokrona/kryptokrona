// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <list>
#include <memory>
#include <mutex>
#include "common/json_value.h"
#include "logger_group.h"

namespace logging
{

    class LoggerManager : public LoggerGroup
    {
    public:
        LoggerManager();
        void configure(const common::JsonValue &val);
        virtual void operator()(const std::string &category, Level level, boost::posix_time::ptime time, const std::string &body) override;

    private:
        std::vector<std::unique_ptr<CommonLogger>> loggers;
        std::mutex reconfigureLock;
    };

}
