// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <vector>
#include <common/ioutput_stream.h>
#include "iserializer.h"
#include "memory_stream.h"

namespace cryptonote
{

    class KVBinaryOutputStreamSerializer : public ISerializer
    {
    public:
        KVBinaryOutputStreamSerializer();
        virtual ~KVBinaryOutputStreamSerializer() {}

        void dump(common::IOutputStream &target);

        virtual ISerializer::SerializerType type() const override;

        virtual bool beginObject(common::StringView name) override;
        virtual void endObject() override;

        virtual bool beginArray(uint64_t &size, common::StringView name) override;
        virtual void endArray() override;

        virtual bool operator()(uint8_t &value, common::StringView name) override;
        virtual bool operator()(int16_t &value, common::StringView name) override;
        virtual bool operator()(uint16_t &value, common::StringView name) override;
        virtual bool operator()(int32_t &value, common::StringView name) override;
        virtual bool operator()(uint32_t &value, common::StringView name) override;
        virtual bool operator()(int64_t &value, common::StringView name) override;
        virtual bool operator()(uint64_t &value, common::StringView name) override;
        virtual bool operator()(double &value, common::StringView name) override;
        virtual bool operator()(bool &value, common::StringView name) override;
        virtual bool operator()(std::string &value, common::StringView name) override;
        virtual bool binary(void *value, uint64_t size, common::StringView name) override;
        virtual bool binary(std::string &value, common::StringView name) override;

        template <typename T>
        bool operator()(T &value, common::StringView name)
        {
            return ISerializer::operator()(value, name);
        }

    private:
        void writeElementPrefix(uint8_t type, common::StringView name);
        void checkArrayPreamble(uint8_t type);
        void updateState(uint8_t type);
        MemoryStream &stream();

        enum class State
        {
            Root,
            Object,
            ArrayPrefix,
            Array
        };

        struct Level
        {
            State state;
            std::string name;
            uint64_t count;

            Level(common::StringView nm) : name(nm), state(State::Object), count(0) {}

            Level(common::StringView nm, uint64_t arraySize) : name(nm), state(State::ArrayPrefix), count(arraySize) {}

            Level(Level &&rv)
            {
                state = rv.state;
                name = std::move(rv.name);
                count = rv.count;
            }
        };

        std::vector<MemoryStream> m_objectsStack;
        std::vector<Level> m_stack;
    };

}
