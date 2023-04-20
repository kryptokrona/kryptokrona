// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cstdint>
#include <string>

namespace tools
{
    namespace base58
    {
        std::string encode(const std::string &data);
        bool decode(const std::string &enc, std::string &data);

        std::string encode_addr(uint64_t tag, const std::string &data);
        bool decode_addr(std::string addr, uint64_t &tag, std::string &data);
    }
}
