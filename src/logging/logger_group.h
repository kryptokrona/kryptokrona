// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <vector>
#include "common_logger.h"

namespace logging
{

    class LoggerGroup : public CommonLogger
    {
    public:
        LoggerGroup(Level level = DEBUGGING);

        void addLogger(ILogger &logger);
        void removeLogger(ILogger &logger);
        virtual void operator()(const std::string &category, Level level, boost::posix_time::ptime time, const std::string &body) override;

    protected:
        std::vector<ILogger *> loggers;
    };

}
