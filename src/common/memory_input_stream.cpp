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

#include "memory_input_stream.h"
#include <algorithm>
#include <cassert>
#include <cstring> // memcpy

namespace common
{
    MemoryInputStream::MemoryInputStream(const void* buffer, uint64_t bufferSize) :
    buffer(static_cast<const char*>(buffer)), bufferSize(bufferSize), position(0) {}

    bool MemoryInputStream::endOfStream() const {
      return position == bufferSize;
    }

    uint64_t MemoryInputStream::readSome(void* data, uint64_t size) {
      assert(position <= bufferSize);
      uint64_t readSize = std::min(size, bufferSize - position);

      if (readSize > 0) {
        memcpy(data, buffer + position, readSize);
        position += readSize;
      }

      return readSize;
    }
}
