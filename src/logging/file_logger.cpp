// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "file_logger.h"

namespace logging
{

    FileLogger::FileLogger(Level level) : StreamLogger(level)
    {
    }

    void FileLogger::init(const std::string &fileName)
    {
        fileStream.open(fileName, std::ios::app);
        StreamLogger::attachToStream(fileStream);
    }

}
