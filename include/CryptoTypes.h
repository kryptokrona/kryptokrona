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

#include <cstdint>

#include <iterator>

namespace Crypto {

struct Hash
{
    bool operator==(const Hash &other) const
    {
        return std::equal(std::begin(data), std::end(data), std::begin(other.data));
    }

    bool operator!=(const Hash &other) const
    {
        return !(*this == other);
    }

    uint8_t data[32];
};

struct PublicKey
{
    bool operator==(const PublicKey &other) const
    {
        return std::equal(std::begin(data), std::end(data), std::begin(other.data));
    }

    bool operator!=(const PublicKey &other) const
    {
        return !(*this == other);
    }

    uint8_t data[32];
};

struct SecretKey
{
    bool operator==(const SecretKey &other) const
    {
        return std::equal(std::begin(data), std::end(data), std::begin(other.data));
    }
    
    bool operator!=(const SecretKey &other) const
    {
        return !(*this == other);
    }

    uint8_t data[32];
};

struct KeyDerivation
{
    bool operator==(const KeyDerivation &other) const
    {
        return std::equal(std::begin(data), std::end(data), std::begin(other.data));
    }

    bool operator!=(const KeyDerivation &other) const
    {
        return !(*this == other);
    }

    uint8_t data[32];
};

struct KeyImage
{
    bool operator==(const KeyImage &other) const
    {
        return std::equal(std::begin(data), std::end(data), std::begin(other.data));
    }

    bool operator!=(const KeyImage &other) const
    {
        return !(*this == other);
    }

    uint8_t data[32];
};

struct Signature
{
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

}
