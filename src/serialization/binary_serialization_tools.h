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

#include <cryptonote.h>
#include "binary_input_stream_serializer.h"
#include "binary_output_stream_serializer.h"
#include "common/memory_input_stream.h"
#include "common/std_input_stream.h"
#include "common/std_output_stream.h"
#include "common/vector_output_stream.h"

#include <fstream>

namespace cryptonote
{
    template <typename T>
    BinaryArray storeToBinary(const T& obj) {
      BinaryArray result;
      Common::VectorOutputStream stream(result);
      BinaryOutputStreamSerializer ba(stream);
      serialize(const_cast<T&>(obj), ba);
      return result;
    }

    template <typename T>
    void loadFromBinary(T& obj, const BinaryArray& blob) {
      Common::MemoryInputStream stream(blob.data(), blob.size());
      BinaryInputStreamSerializer ba(stream);
      serialize(obj, ba);
    }

    template <typename T>
    bool storeToBinaryFile(const T& obj, const std::string& filename) {
      try {
        std::ofstream dataFile;
        dataFile.open(filename, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
        if (dataFile.fail()) {
          return false;
        }

        Common::StdOutputStream stream(dataFile);
        BinaryOutputStreamSerializer out(stream);
        CryptoNote::serialize(const_cast<T&>(obj), out);

        if (dataFile.fail()) {
          return false;
        }

        dataFile.flush();
      } catch (std::exception&) {
        return false;
      }

      return true;
    }

    template<class T>
    bool loadFromBinaryFile(T& obj, const std::string& filename) {
      try {
        std::ifstream dataFile;
        dataFile.open(filename, std::ios_base::binary | std::ios_base::in);
        if (dataFile.fail()) {
          return false;
        }

        Common::StdInputStream stream(dataFile);
        BinaryInputStreamSerializer in(stream);
        serialize(obj, in);
        return !dataFile.fail();
      } catch (std::exception&) {
        return false;
      }
    }
}
