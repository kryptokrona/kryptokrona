// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <boost/optional.hpp>
#include <cryptonote.h>

namespace cryptonote
{
    class CachedBlock {
    public:
      explicit CachedBlock(const BlockTemplate& block);
      const BlockTemplate& getBlock() const;
      const crypto::Hash& getTransactionTreeHash() const;
      const crypto::Hash& getBlockHash() const;
      const crypto::Hash& getBlockLongHash() const;
      const crypto::Hash& getAuxiliaryBlockHeaderHash() const;
      const BinaryArray& getBlockHashingBinaryArray() const;
      const BinaryArray& getParentBlockBinaryArray(bool headerOnly) const;
      const BinaryArray& getParentBlockHashingBinaryArray(bool headerOnly) const;
      uint32_t getBlockIndex() const;

    private:
      const BlockTemplate& block;
      mutable boost::optional<BinaryArray> blockHashingBinaryArray;
      mutable boost::optional<BinaryArray> parentBlockBinaryArray;
      mutable boost::optional<BinaryArray> parentBlockHashingBinaryArray;
      mutable boost::optional<BinaryArray> parentBlockBinaryArrayHeaderOnly;
      mutable boost::optional<BinaryArray> parentBlockHashingBinaryArrayHeaderOnly;
      mutable boost::optional<uint32_t> blockIndex;
      mutable boost::optional<crypto::Hash> transactionTreeHash;
      mutable boost::optional<crypto::Hash> blockHash;
      mutable boost::optional<crypto::Hash> blockLongHash;
      mutable boost::optional<crypto::Hash> auxiliaryBlockHeaderHash;
    };
}
