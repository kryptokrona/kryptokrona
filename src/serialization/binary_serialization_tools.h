// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

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
    BinaryArray storeToBinary(const T &obj)
    {
        BinaryArray result;
        common::VectorOutputStream stream(result);
        BinaryOutputStreamSerializer ba(stream);
        serialize(const_cast<T &>(obj), ba);
        return result;
    }

    template <typename T>
    void loadFromBinary(T &obj, const BinaryArray &blob)
    {
        common::MemoryInputStream stream(blob.data(), blob.size());
        BinaryInputStreamSerializer ba(stream);
        serialize(obj, ba);
    }

    template <typename T>
    bool storeToBinaryFile(const T &obj, const std::string &filename)
    {
        try
        {
            std::ofstream dataFile;
            dataFile.open(filename, std::ios_base::binary | std::ios_base::out | std::ios::trunc);
            if (dataFile.fail())
            {
                return false;
            }

            common::StdOutputStream stream(dataFile);
            BinaryOutputStreamSerializer out(stream);
            cryptonote::serialize(const_cast<T &>(obj), out);

            if (dataFile.fail())
            {
                return false;
            }

            dataFile.flush();
        }
        catch (std::exception &)
        {
            return false;
        }

        return true;
    }

    template <class T>
    bool loadFromBinaryFile(T &obj, const std::string &filename)
    {
        try
        {
            std::ifstream dataFile;
            dataFile.open(filename, std::ios_base::binary | std::ios_base::in);
            if (dataFile.fail())
            {
                return false;
            }

            common::StdInputStream stream(dataFile);
            BinaryInputStreamSerializer in(stream);
            serialize(obj, in);
            return !dataFile.fail();
        }
        catch (std::exception &)
        {
            return false;
        }
    }

}
