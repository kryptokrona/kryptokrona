// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <common/iinput_stream.h>
#include "iserializer.h"
#include "json_input_value_serializer.h"

namespace cryptonote
{

    class KVBinaryInputStreamSerializer : public JsonInputValueSerializer
    {
    public:
        KVBinaryInputStreamSerializer(common::IInputStream &strm);

        virtual bool binary(void *value, uint64_t size, common::StringView name) override;
        virtual bool binary(std::string &value, common::StringView name) override;
    };

}
