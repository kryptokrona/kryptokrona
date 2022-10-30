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

#include <cstdint>
#include <memory>

<<<<<<<< HEAD:src/serialization/istream.h
namespace cryptonote
{
    class IInputStream {
    public:
      virtual uint64_t read(char* data, uint64_t size) = 0;
    };
========
namespace cryptonote {

class IUpgradeManager {
public:
  virtual ~IUpgradeManager() {}

  virtual void addMajorBlockVersion(uint8_t targetVersion, uint32_t upgradeHeight) = 0;
  virtual uint8_t getBlockMajorVersion(uint32_t blockIndex) const = 0;
};
>>>>>>>> 20dc9b25 (Refactoring cryptonote package):src/cryptonote_core/iupgrade_manager.h

    class IOutputStream {
    public:
      virtual void write(const char* data, uint64_t size) = 0;
    };
}
