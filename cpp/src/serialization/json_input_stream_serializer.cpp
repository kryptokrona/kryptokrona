// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "serialization/json_input_stream_serializer.h"

#include <ctype.h>
#include <exception>

namespace cryptonote
{

    namespace
    {

        common::JsonValue getJsonValueFromStreamHelper(std::istream &stream)
        {
            common::JsonValue value;
            stream >> value;
            return value;
        }

    }

    JsonInputStreamSerializer::JsonInputStreamSerializer(std::istream &stream) : JsonInputValueSerializer(getJsonValueFromStreamHelper(stream))
    {
    }

    JsonInputStreamSerializer::~JsonInputStreamSerializer()
    {
    }

} // namespace cryptonote
