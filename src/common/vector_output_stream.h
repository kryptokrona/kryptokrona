// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cstdint>
#include <vector>
#include "ioutput_stream.h"

namespace common
{

    class VectorOutputStream : public IOutputStream
    {
    public:
        VectorOutputStream(std::vector<uint8_t> &out);
        VectorOutputStream &operator=(const VectorOutputStream &) = delete;
        uint64_t writeSome(const void *data, uint64_t size) override;

    private:
        std::vector<uint8_t> &out;
    };

}
