// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "string_input_stream.h"
#include <string.h>

namespace common
{

    StringInputStream::StringInputStream(const std::string &in) : in(in), offset(0)
    {
    }

    uint64_t StringInputStream::readSome(void *data, uint64_t size)
    {
        if (size > in.size() - offset)
        {
            size = in.size() - offset;
        }

        memcpy(data, in.data() + offset, size);
        offset += size;
        return size;
    }

}
