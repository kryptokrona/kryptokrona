// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "serialization/serialization_overloads.h"

#include <stdexcept>
#include <limits>

namespace cryptonote
{

    void serializeBlockHeight(ISerializer &s, uint32_t &blockHeight, common::StringView name)
    {
        if (s.type() == ISerializer::INPUT)
        {
            uint64_t height;
            s(height, name);

            if (height == std::numeric_limits<uint64_t>::max())
            {
                blockHeight = std::numeric_limits<uint32_t>::max();
            }
            else if (height > std::numeric_limits<uint32_t>::max() && height < std::numeric_limits<uint64_t>::max())
            {
                throw std::runtime_error("Deserialization error: wrong value");
            }
            else
            {
                blockHeight = static_cast<uint32_t>(height);
            }
        }
        else
        {
            s(blockHeight, name);
        }
    }

    void serializeGlobalOutputIndex(ISerializer &s, uint32_t &globalOutputIndex, common::StringView name)
    {
        serializeBlockHeight(s, globalOutputIndex, name);
    }

} // namespace cryptonote
