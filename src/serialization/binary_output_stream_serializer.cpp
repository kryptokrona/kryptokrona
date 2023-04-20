// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "binary_output_stream_serializer.h"

#include <cassert>
#include <stdexcept>
#include "common/stream_tools.h"

using namespace common;

namespace cryptonote
{

    ISerializer::SerializerType BinaryOutputStreamSerializer::type() const
    {
        return ISerializer::OUTPUT;
    }

    bool BinaryOutputStreamSerializer::beginObject(common::StringView name)
    {
        return true;
    }

    void BinaryOutputStreamSerializer::endObject()
    {
    }

    bool BinaryOutputStreamSerializer::beginArray(uint64_t &size, common::StringView name)
    {
        writeVarint(stream, size);
        return true;
    }

    void BinaryOutputStreamSerializer::endArray()
    {
    }

    bool BinaryOutputStreamSerializer::operator()(uint8_t &value, common::StringView name)
    {
        writeVarint(stream, value);
        return true;
    }

    bool BinaryOutputStreamSerializer::operator()(uint16_t &value, common::StringView name)
    {
        writeVarint(stream, value);
        return true;
    }

    bool BinaryOutputStreamSerializer::operator()(int16_t &value, common::StringView name)
    {
        writeVarint(stream, static_cast<uint16_t>(value));
        return true;
    }

    bool BinaryOutputStreamSerializer::operator()(uint32_t &value, common::StringView name)
    {
        writeVarint(stream, value);
        return true;
    }

    bool BinaryOutputStreamSerializer::operator()(int32_t &value, common::StringView name)
    {
        writeVarint(stream, static_cast<uint32_t>(value));
        return true;
    }

    bool BinaryOutputStreamSerializer::operator()(int64_t &value, common::StringView name)
    {
        writeVarint(stream, static_cast<uint64_t>(value));
        return true;
    }

    bool BinaryOutputStreamSerializer::operator()(uint64_t &value, common::StringView name)
    {
        writeVarint(stream, value);
        return true;
    }

    bool BinaryOutputStreamSerializer::operator()(bool &value, common::StringView name)
    {
        char boolVal = value;
        checkedWrite(&boolVal, 1);
        return true;
    }

    bool BinaryOutputStreamSerializer::operator()(std::string &value, common::StringView name)
    {
        writeVarint(stream, value.size());
        checkedWrite(value.data(), value.size());
        return true;
    }

    bool BinaryOutputStreamSerializer::binary(void *value, uint64_t size, common::StringView name)
    {
        checkedWrite(static_cast<const char *>(value), size);
        return true;
    }

    bool BinaryOutputStreamSerializer::binary(std::string &value, common::StringView name)
    {
        // write as string (with size prefix)
        return (*this)(value, name);
    }

    bool BinaryOutputStreamSerializer::operator()(double &value, common::StringView name)
    {
        assert(false); // the method is not supported for this type of serialization
        throw std::runtime_error("double serialization is not supported in BinaryOutputStreamSerializer");
        return false;
    }

    void BinaryOutputStreamSerializer::checkedWrite(const char *buf, uint64_t size)
    {
        write(stream, buf, size);
    }

}
