// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "common/json_value.h"
#include "iserializer.h"

namespace cryptonote
{
    //deserialization
    class JsonInputValueSerializer : public ISerializer {
    public:
      JsonInputValueSerializer(const common::JsonValue& value);
      JsonInputValueSerializer(common::JsonValue&& value);
      virtual ~JsonInputValueSerializer();

      SerializerType type() const override;

      virtual bool beginObject(common::StringView name) override;
      virtual void endObject() override;

      virtual bool beginArray(uint64_t& size, common::StringView name) override;
      virtual void endArray() override;

      virtual bool operator()(uint8_t& value, common::StringView name) override;
      virtual bool operator()(int16_t& value, common::StringView name) override;
      virtual bool operator()(uint16_t& value, common::StringView name) override;
      virtual bool operator()(int32_t& value, common::StringView name) override;
      virtual bool operator()(uint32_t& value, common::StringView name) override;
      virtual bool operator()(int64_t& value, common::StringView name) override;
      virtual bool operator()(uint64_t& value, common::StringView name) override;
      virtual bool operator()(double& value, common::StringView name) override;
      virtual bool operator()(bool& value, common::StringView name) override;
      virtual bool operator()(std::string& value, common::StringView name) override;
      virtual bool binary(void* value, uint64_t size, common::StringView name) override;
      virtual bool binary(std::string& value, common::StringView name) override;

      template<typename T>
      bool operator()(T& value, common::StringView name) {
        return ISerializer::operator()(value, name);
      }

    private:
      common::JsonValue value;
      std::vector<const common::JsonValue*> chain;
      std::vector<uint64_t> idxs;

      const common::JsonValue* getValue(common::StringView name);

      template <typename T>
      bool getNumber(common::StringView name, T& v) {
        auto ptr = getValue(name);

        if (!ptr) {
          return false;
        }

        v = static_cast<T>(ptr->getInteger());
        return true;
      }
    };
}
