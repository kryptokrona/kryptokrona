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

#include <algorithm>

#include <common/string_tools.h>

#include <cstdint>

#include <iterator>

#include "json.hpp"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace crypto
{
    struct Hash
    {
        /* Can't have constructors here, because it violates std::is_pod<>
           which is used somewhere */
        bool operator==(const Hash &other) const
        {
            return std::equal(std::begin(data), std::end(data), std::begin(other.data));
        }

        bool operator!=(const Hash &other) const
        {
            return !(*this == other);
        }

        uint8_t data[32];

        /* Converts the class to a json object */
        void toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.String(common::podToHex(data));
        }

        /* Initializes the class from a json string */
        void fromString(const std::string &s)
        {
            if (!common::podFromHex(s, data))
            {
                throw std::invalid_argument("Error parsing JSON Hash, wrong length or not hex");
            }
        }
    };

    struct PublicKey
    {
        PublicKey() {}

        PublicKey(std::initializer_list<uint8_t> input)
        {
            std::copy(input.begin(), input.end(), std::begin(data));
        }

        PublicKey(const uint8_t input[32])
        {
            std::copy(input, input + 32, std::begin(data));
        }

        bool operator==(const PublicKey &other) const
        {
            return std::equal(std::begin(data), std::end(data), std::begin(other.data));
        }

        bool operator!=(const PublicKey &other) const
        {
            return !(*this == other);
        }

        /* Converts the class to a json object */
        void toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.String(common::podToHex(data));
        }

        /* Initializes the class from a json string */
        void fromString(const std::string &s)
        {
            if (!common::podFromHex(s, data))
            {
                throw std::invalid_argument("Error parsing JSON PublicKey, wrong length or not hex");
            }
        }

        uint8_t data[32];
    };

    struct SecretKey
    {
        SecretKey() {}

        SecretKey(std::initializer_list<uint8_t> input)
        {
            std::copy(input.begin(), input.end(), std::begin(data));
        }

        SecretKey(const uint8_t input[32])
        {
            std::copy(input, input + 32, std::begin(data));
        }

        bool operator==(const SecretKey &other) const
        {
            return std::equal(std::begin(data), std::end(data), std::begin(other.data));
        }

        bool operator!=(const SecretKey &other) const
        {
            return !(*this == other);
        }

        /* Converts the class to a json object */
        void toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.String(common::podToHex(data));
        }

        /* Initializes the class from a json string */
        void fromString(const std::string &s)
        {
            if (!common::podFromHex(s, data))
            {
                throw std::invalid_argument("Error parsing JSON SecretKey, wrong length or not hex");
            }
        }

        uint8_t data[32];
    };

    struct KeyDerivation
    {
        KeyDerivation() {}

        KeyDerivation(std::initializer_list<uint8_t> input)
        {
            std::copy(input.begin(), input.end(), std::begin(data));
        }

        KeyDerivation(const uint8_t input[32])
        {
            std::copy(input, input + 32, std::begin(data));
        }

        bool operator==(const KeyDerivation &other) const
        {
            return std::equal(std::begin(data), std::end(data), std::begin(other.data));
        }

        bool operator!=(const KeyDerivation &other) const
        {
            return !(*this == other);
        }

        /* Converts the class to a json object */
        void toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.String(common::podToHex(data));
        }

        /* Initializes the class from a json string */
        void fromString(const std::string &s)
        {
            if (!common::podFromHex(s, data))
            {
                throw std::invalid_argument("Error parsing JSON KeyDerivation, wrong length or not hex");
            }
        }

        uint8_t data[32];
    };

    struct KeyImage
    {
        KeyImage() {}

        KeyImage(std::initializer_list<uint8_t> input)
        {
            std::copy(input.begin(), input.end(), std::begin(data));
        }

        KeyImage(const uint8_t input[32])
        {
            std::copy(input, input + 32, std::begin(data));
        }

        bool operator==(const KeyImage &other) const
        {
            return std::equal(std::begin(data), std::end(data), std::begin(other.data));
        }

        bool operator!=(const KeyImage &other) const
        {
            return !(*this == other);
        }

        /* Converts the class to a json object */
        void toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const
        {
            writer.String(common::podToHex(data));
        }

        /* Initializes the class from a json string */
        void fromString(const std::string &s)
        {
            if (!common::podFromHex(s, data))
            {
                throw std::invalid_argument("Error parsing JSON keyimage, wrong length or not hex");
            }
        }

        uint8_t data[32];
    };

    struct Signature
    {
        Signature() {}

        Signature(std::initializer_list<uint8_t> input)
        {
            std::copy(input.begin(), input.end(), std::begin(data));
        }

        Signature(const uint8_t input[64])
        {
            std::copy(input, input + 64, std::begin(data));
        }

        bool operator==(const Signature &other) const
        {
            return std::equal(std::begin(data), std::end(data), std::begin(other.data));
        }

        bool operator!=(const Signature &other) const
        {
            return !(*this == other);
        }

        uint8_t data[64];
    };

    /* For boost hash_value */
    inline size_t hash_value(const Hash &hash)
    {
        return reinterpret_cast<const size_t &>(hash);
    }

    inline size_t hash_value(const PublicKey &publicKey)
    {
        return reinterpret_cast<const size_t &>(publicKey);
    }

    inline size_t hash_value(const SecretKey &secretKey)
    {
        return reinterpret_cast<const size_t &>(secretKey);
    }

    inline size_t hash_value(const KeyDerivation &keyDerivation)
    {
        return reinterpret_cast<const size_t &>(keyDerivation);
    }

    inline size_t hash_value(const KeyImage &keyImage)
    {
        return reinterpret_cast<const size_t &>(keyImage);
    }

    inline void to_json(nlohmann::json &j, const Hash &h)
    {
        j = common::podToHex(h);
    }

    inline void from_json(const nlohmann::json &j, Hash &h)
    {
        if (!common::podFromHex(j.get<std::string>(), h.data))
        {
            const auto err = nlohmann::detail::parse_error::create(
                100, 0, "Wrong length or not hex!");

            throw nlohmann::json::parse_error(err);
        }
    }

    inline void to_json(nlohmann::json &j, const PublicKey &p)
    {
        j = common::podToHex(p);
    }

    inline void from_json(const nlohmann::json &j, PublicKey &p)
    {
        if (!common::podFromHex(j.get<std::string>(), p.data))
        {
            const auto err = nlohmann::detail::parse_error::create(
                100, 0, "Wrong length or not hex!");

            throw nlohmann::json::parse_error(err);
        }
    }

    inline void to_json(nlohmann::json &j, const SecretKey &s)
    {
        j = common::podToHex(s);
    }

    inline void from_json(const nlohmann::json &j, SecretKey &s)
    {
        if (!common::podFromHex(j.get<std::string>(), s.data))
        {
            const auto err = nlohmann::detail::parse_error::create(
                100, 0, "Wrong length or not hex!");

            throw nlohmann::json::parse_error(err);
        }
    }

    inline void to_json(nlohmann::json &j, const KeyDerivation &k)
    {
        j = common::podToHex(k);
    }

    inline void from_json(const nlohmann::json &j, KeyDerivation &k)
    {
        if (!common::podFromHex(j.get<std::string>(), k.data))
        {
            const auto err = nlohmann::detail::parse_error::create(
                100, 0, "Wrong length or not hex!");

            throw nlohmann::json::parse_error(err);
        }
    }

    inline void to_json(nlohmann::json &j, const KeyImage &k)
    {
        j = common::podToHex(k);
    }

    inline void from_json(const nlohmann::json &j, KeyImage &k)
    {
        if (!common::podFromHex(j.get<std::string>(), k.data))
        {
            const auto err = nlohmann::detail::parse_error::create(
                100, 0, "Wrong length or not hex!");

            throw nlohmann::json::parse_error(err);
        }
    }
}

namespace std
{
    /* For using in std::unordered_* containers */
    template <>
    struct hash<crypto::Hash>
    {
        size_t operator()(const crypto::Hash &hash) const
        {
            return reinterpret_cast<const size_t &>(hash);
        }
    };

    template <>
    struct hash<crypto::PublicKey>
    {
        size_t operator()(const crypto::PublicKey &publicKey) const
        {
            return reinterpret_cast<const size_t &>(publicKey);
        }
    };

    template <>
    struct hash<crypto::SecretKey>
    {
        size_t operator()(const crypto::SecretKey &secretKey) const
        {
            return reinterpret_cast<const size_t &>(secretKey);
        }
    };

    template <>
    struct hash<crypto::KeyDerivation>
    {
        size_t operator()(const crypto::KeyDerivation &keyDerivation) const
        {
            return reinterpret_cast<const size_t &>(keyDerivation);
        }
    };

    template <>
    struct hash<crypto::KeyImage>
    {
        size_t operator()(const crypto::KeyImage &keyImage) const
        {
            return reinterpret_cast<const size_t &>(keyImage);
        }
    };

    template <>
    struct hash<crypto::Signature>
    {
        size_t operator()(const crypto::Signature &signature) const
        {
            return reinterpret_cast<const size_t &>(signature);
        }
    };

    /* Overloading the << operator */
    inline ostream &operator<<(ostream &os, const crypto::Hash &hash)
    {
        os << common::podToHex(hash);
        return os;
    }

    inline ostream &operator<<(ostream &os, const crypto::PublicKey &publicKey)
    {
        os << common::podToHex(publicKey);
        return os;
    }

    inline ostream &operator<<(ostream &os, const crypto::SecretKey &secretKey)
    {
        os << common::podToHex(secretKey);
        return os;
    }

    inline ostream &operator<<(ostream &os, const crypto::KeyDerivation &keyDerivation)
    {
        os << common::podToHex(keyDerivation);
        return os;
    }

    inline ostream &operator<<(ostream &os, const crypto::KeyImage &keyImage)
    {
        os << common::podToHex(keyImage);
        return os;
    }

    inline ostream &operator<<(ostream &os, const crypto::Signature &signature)
    {
        os << common::podToHex(signature);
        return os;
    }
}
