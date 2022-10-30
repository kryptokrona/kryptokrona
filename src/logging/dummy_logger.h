// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <logging/ilogger.h>

namespace logging
{
    class DummyLogger : public ILogger
    {
        public:
            virtual ~DummyLogger() {};

            virtual void operator()(const std::string &category, Level level, boost::posix_time::ptime time, const std::string &body) override
            {
                // do nothing
            }
    };
}
