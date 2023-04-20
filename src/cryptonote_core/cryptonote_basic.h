// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <boost/utility/value_init.hpp>
#include <cryptonote.h>

namespace cryptonote
{
    const crypto::Hash NULL_HASH = boost::value_initialized<crypto::Hash>();
    const crypto::PublicKey NULL_PUBLIC_KEY = boost::value_initialized<crypto::PublicKey>();
    const crypto::SecretKey NULL_SECRET_KEY = boost::value_initialized<crypto::SecretKey>();

    KeyPair generateKeyPair();

    struct ParentBlockSerializer
    {
        ParentBlockSerializer(ParentBlock &parentBlock, uint64_t &timestamp, uint32_t &nonce, bool hashingSerialization, bool headerOnly) : m_parentBlock(parentBlock), m_timestamp(timestamp), m_nonce(nonce), m_hashingSerialization(hashingSerialization), m_headerOnly(headerOnly)
        {
        }

        ParentBlock &m_parentBlock;
        uint64_t &m_timestamp;
        uint32_t &m_nonce;
        bool m_hashingSerialization;
        bool m_headerOnly;
    };

    inline ParentBlockSerializer makeParentBlockSerializer(const BlockTemplate &b, bool hashingSerialization, bool headerOnly)
    {
        BlockTemplate &blockRef = const_cast<BlockTemplate &>(b);
        return ParentBlockSerializer(blockRef.parentBlock, blockRef.timestamp, blockRef.nonce, hashingSerialization, headerOnly);
    }

}
