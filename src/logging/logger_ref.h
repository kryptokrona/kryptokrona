// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "ilogger.h"
#include "logger_message.h"

namespace logging
{

    class LoggerRef
    {
    public:
        LoggerRef(std::shared_ptr<ILogger> logger, const std::string &category);
        LoggerMessage operator()(Level level = INFO, const std::string &color = DEFAULT) const;
        std::shared_ptr<ILogger> getLogger() const;

    private:
        std::shared_ptr<ILogger> logger;
        std::string category;
    };

}
