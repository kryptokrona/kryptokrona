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

#include <list>
#include <vector>
#include <common/memory_input_stream.h>
#include <common/string_output_stream.h>
#include "json_input_stream_serializer.h"
#include "json_output_stream_serializer.h"
#include "kv_binary_input_stream_serializer.h"
#include "kv_binary_output_stream_serializer.h"
#include <zedwallet/types.h>

namespace common
{
    template <typename T>
    T getValueAs(const JsonValue& js) {
      return js;
      //cdstatic_assert(false, "undefined conversion");
    }

    template <>
    inline std::string getValueAs<std::string>(const JsonValue& js) { return js.getString(); }

    template <>
    inline uint64_t getValueAs<uint64_t>(const JsonValue& js) { return static_cast<uint64_t>(js.getInteger()); }
}

namespace cryptonote
{
    template <typename T>
    Common::JsonValue storeToJsonValue(const T& v) {
      JsonOutputStreamSerializer s;
      serialize(const_cast<T&>(v), s);
      return s.getValue();
    }

    template <typename T>
    Common::JsonValue storeContainerToJsonValue(const T& cont) {
      Common::JsonValue js(Common::JsonValue::ARRAY);
      for (const auto& item : cont) {
        js.pushBack(item);
      }
      return js;
    }

    template <>
    inline Common::JsonValue storeContainerToJsonValue(const std::vector<AddressBookEntry> &cont) {
      Common::JsonValue js(Common::JsonValue::ARRAY);
      for (const auto& item : cont) {
        js.pushBack(storeToJsonValue(item));
      }
      return js;
    }

    template <typename T>
    Common::JsonValue storeToJsonValue(const std::vector<T>& v) { return storeContainerToJsonValue(v); }

    template <typename T>
    Common::JsonValue storeToJsonValue(const std::list<T>& v) { return storeContainerToJsonValue(v); }

    template <>
    inline Common::JsonValue storeToJsonValue(const std::string& v) { return Common::JsonValue(v); }

    template <typename T>
    void loadFromJsonValue(T& v, const Common::JsonValue& js) {
      JsonInputValueSerializer s(js);
      serialize(v, s);
    }

    template <typename T>
    void loadFromJsonValue(std::vector<T>& v, const Common::JsonValue& js) {
      for (uint64_t i = 0; i < js.size(); ++i) {
        v.push_back(Common::getValueAs<T>(js[i]));
      }
    }

    template <>
    inline void loadFromJsonValue(AddressBook &v, const Common::JsonValue &js) {
      for (uint64_t i = 0; i < js.size(); ++i) {
        AddressBookEntry type;
        loadFromJsonValue(type, js[i]);
        v.push_back(type);
      }
    }

    template <typename T>
    void loadFromJsonValue(std::list<T>& v, const Common::JsonValue& js) {
      for (uint64_t i = 0; i < js.size(); ++i) {
        v.push_back(Common::getValueAs<T>(js[i]));
      }
    }

    template <typename T>
    std::string storeToJson(const T& v) {
      return storeToJsonValue(v).toString();
    }

    template <typename T>
    bool loadFromJson(T& v, const std::string& buf) {
      try {
        if (buf.empty()) {
          return true;
        }
        auto js = Common::JsonValue::fromString(buf);
        loadFromJsonValue(v, js);
      } catch (std::exception&) {
        return false;
      }
      return true;
    }

    template <typename T>
    std::string storeToBinaryKeyValue(const T& v) {
      KVBinaryOutputStreamSerializer s;
      serialize(const_cast<T&>(v), s);

      std::string result;
      Common::StringOutputStream stream(result);
      s.dump(stream);
      return result;
    }

    template <typename T>
    bool loadFromBinaryKeyValue(T& v, const std::string& buf) {
      try {
        Common::MemoryInputStream stream(buf.data(), buf.size());
        KVBinaryInputStreamSerializer s(stream);
        serialize(v, s);
        return true;
      } catch (std::exception&) {
        return false;
      }
    }
}
