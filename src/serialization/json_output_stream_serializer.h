// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <iostream>
#include "common/json_value.h"
#include "iserializer.h"

namespace cryptonote
{

    class JsonOutputStreamSerializer : public ISerializer
    {
    public:
        JsonOutputStreamSerializer();
        virtual ~JsonOutputStreamSerializer();

        SerializerType type() const override;

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

        const common::JsonValue &getValue() const
        {
            return root;
        }

        friend std::ostream &operator<<(std::ostream &out, const JsonOutputStreamSerializer &enumerator);

    private:
        common::JsonValue root;
        std::vector<common::JsonValue *> chain;
    };

}
