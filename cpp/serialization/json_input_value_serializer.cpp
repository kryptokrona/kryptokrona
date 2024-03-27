// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "json_input_value_serializer.h"

#include <cassert>
#include <stdexcept>

#include "common/string_tools.h"

using common::JsonValue;
using namespace cryptonote;

JsonInputValueSerializer::JsonInputValueSerializer(const common::JsonValue &value)
{
    if (!value.isObject())
    {
        throw std::runtime_error("Serializer doesn't support this type of serialization: Object expected.");
    }

    chain.push_back(&value);
}

JsonInputValueSerializer::JsonInputValueSerializer(common::JsonValue &&value) : value(std::move(value))
{
    if (!this->value.isObject())
    {
        throw std::runtime_error("Serializer doesn't support this type of serialization: Object expected.");
    }

    chain.push_back(&this->value);
}

JsonInputValueSerializer::~JsonInputValueSerializer()
{
}

ISerializer::SerializerType JsonInputValueSerializer::type() const
{
    return ISerializer::INPUT;
}

bool JsonInputValueSerializer::beginObject(common::StringView name)
{
    const JsonValue *parent = chain.back();

    if (parent->isArray())
    {
        const JsonValue &v = (*parent)[idxs.back()++];
        chain.push_back(&v);
        return true;
    }

    if (parent->contains(std::string(name)))
    {
        const JsonValue &v = (*parent)(std::string(name));
        chain.push_back(&v);
        return true;
    }

    return false;
}

void JsonInputValueSerializer::endObject()
{
    assert(!chain.empty());
    chain.pop_back();
}

bool JsonInputValueSerializer::beginArray(uint64_t &size, common::StringView name)
{
    const JsonValue *parent = chain.back();
    std::string strName(name);

    if (parent->contains(strName))
    {
        const JsonValue &arr = (*parent)(strName);
        size = arr.size();
        chain.push_back(&arr);
        idxs.push_back(0);
        return true;
    }

    size = 0;
    return false;
}

void JsonInputValueSerializer::endArray()
{
    assert(!chain.empty());
    assert(!idxs.empty());

    chain.pop_back();
    idxs.pop_back();
}

bool JsonInputValueSerializer::operator()(uint16_t &value, common::StringView name)
{
    return getNumber(name, value);
}

bool JsonInputValueSerializer::operator()(int16_t &value, common::StringView name)
{
    return getNumber(name, value);
}

bool JsonInputValueSerializer::operator()(uint32_t &value, common::StringView name)
{
    return getNumber(name, value);
}

bool JsonInputValueSerializer::operator()(int32_t &value, common::StringView name)
{
    return getNumber(name, value);
}

bool JsonInputValueSerializer::operator()(int64_t &value, common::StringView name)
{
    return getNumber(name, value);
}

bool JsonInputValueSerializer::operator()(uint64_t &value, common::StringView name)
{
    return getNumber(name, value);
}

bool JsonInputValueSerializer::operator()(double &value, common::StringView name)
{
    return getNumber(name, value);
}

bool JsonInputValueSerializer::operator()(uint8_t &value, common::StringView name)
{
    return getNumber(name, value);
}

bool JsonInputValueSerializer::operator()(std::string &value, common::StringView name)
{
    auto ptr = getValue(name);
    if (ptr == nullptr)
    {
        return false;
    }
    value = ptr->getString();
    return true;
}

bool JsonInputValueSerializer::operator()(bool &value, common::StringView name)
{
    auto ptr = getValue(name);
    if (ptr == nullptr)
    {
        return false;
    }
    value = ptr->getBool();
    return true;
}

bool JsonInputValueSerializer::binary(void *value, uint64_t size, common::StringView name)
{
    auto ptr = getValue(name);
    if (ptr == nullptr)
    {
        return false;
    }

    common::fromHex(ptr->getString(), value, size);
    return true;
}

bool JsonInputValueSerializer::binary(std::string &value, common::StringView name)
{
    auto ptr = getValue(name);
    if (ptr == nullptr)
    {
        return false;
    }

    std::string valueHex = ptr->getString();
    value = common::asString(common::fromHex(valueHex));

    return true;
}

const JsonValue *JsonInputValueSerializer::getValue(common::StringView name)
{
    const JsonValue &val = *chain.back();
    if (val.isArray())
    {
        return &val[idxs.back()++];
    }

    std::string strName(name);
    return val.contains(strName) ? &val(strName) : nullptr;
}
