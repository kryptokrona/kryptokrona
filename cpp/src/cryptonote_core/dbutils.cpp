// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "dbutils.h"

namespace
{
    const std::string RAW_BLOCK_NAME = "raw_block";
    const std::string RAW_TXS_NAME = "raw_txs";
}

namespace cryptonote
{
    namespace db
    {
        std::string serialize(const RawBlock &value, const std::string &name)
        {
            std::stringstream ss;
            common::StdOutputStream stream(ss);
            cryptonote::BinaryOutputStreamSerializer serializer(stream);

            serializer(const_cast<RawBlock &>(value).block, RAW_BLOCK_NAME);
            serializer(const_cast<RawBlock &>(value).transactions, RAW_TXS_NAME);

            return ss.str();
        }

        void deserialize(const std::string &serialized, RawBlock &value, const std::string &name)
        {
            std::stringstream ss(serialized);
            common::StdInputStream stream(ss);
            cryptonote::BinaryInputStreamSerializer serializer(stream);
            serializer(value.block, RAW_BLOCK_NAME);
            serializer(value.transactions, RAW_TXS_NAME);
        }
    }
}
