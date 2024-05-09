// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "error_message.h"
#include <cerrno>
#include <cstring>

namespace syst
{

    std::string lastErrorMessage()
    {
        return errorMessage(errno);
    }

    std::string errorMessage(int err)
    {
        return "result=" + std::to_string(err) + ", " + std::strerror(err);
    }

}