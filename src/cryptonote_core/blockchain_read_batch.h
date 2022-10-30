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

#include <boost/functional/hash.hpp>

#include "iread_batch.h"
#include "cryptonote.h"
#include "blockchain_cache.h"
#include "database_cache_data.h"

namespace std
{
    template <> struct hash<std::pair<cryptonote::IBlockchainCache::Amount, uint32_t>> {
      using argment_type = std::pair<cryptonote::IBlockchainCache::Amount, uint32_t>;
      using result_type = size_t;

      result_type operator() (const argment_type& arg) const {
        size_t hashValue = boost::hash_value(arg.first);
        boost::hash_combine(hashValue, arg.second);
        return hashValue;
      }
    };

    template <> struct hash<std::pair<crypto::Hash, uint32_t>> {
      using argment_type = std::pair<crypto::Hash, uint32_t>;
      using result_type = size_t;

      result_type operator() (const argment_type& arg) const {
        size_t hashValue = std::hash<crypto::Hash>{}(arg.first);
        boost::hash_combine(hashValue, arg.second);
        return hashValue;
      }
    };
}

namespace cryptonote
{
    using KeyOutputKeyResult = std::unordered_map<std::pair<IBlockchainCache::Amount, IBlockchainCache::GlobalOutputIndex>, KeyOutputInfo>;

    struct BlockchainReadState {
      std::unordered_map<uint32_t, std::vector<crypto::KeyImage>> spentKeyImagesByBlock;
      std::unordered_map<crypto::KeyImage, uint32_t> blockIndexesBySpentKeyImages;
      std::unordered_map<crypto::Hash, ExtendedTransactionInfo> cachedTransactions;
      std::unordered_map<uint32_t, std::vector<crypto::Hash>> transactionHashesByBlocks;
      std::unordered_map<uint32_t, CachedBlockInfo> cachedBlocks;
      std::unordered_map<crypto::Hash, uint32_t> blockIndexesByBlockHashes;
      std::unordered_map<IBlockchainCache::Amount, uint32_t> keyOutputGlobalIndexesCountForAmounts;
      std::unordered_map<std::pair<IBlockchainCache::Amount, uint32_t>, PackedOutIndex> keyOutputGlobalIndexesForAmounts;
      std::unordered_map<uint32_t, RawBlock> rawBlocks;
      std::unordered_map<uint64_t, uint32_t> closestTimestampBlockIndex;
      std::unordered_map<uint32_t, IBlockchainCache::Amount> keyOutputAmounts;
      std::unordered_map<crypto::Hash, uint32_t> transactionCountsByPaymentIds;
      std::unordered_map<std::pair<crypto::Hash, uint32_t>, crypto::Hash> transactionHashesByPaymentIds;
      std::unordered_map<uint64_t, std::vector<crypto::Hash>> blockHashesByTimestamp;
      KeyOutputKeyResult keyOutputKeys;

      std::pair<uint32_t, bool> lastBlockIndex = { 0, false };
      std::pair<uint32_t, bool> keyOutputAmountsCount = { {}, false };
      std::pair<uint64_t, bool> transactionsCount = { 0, false };

      BlockchainReadState() = default;
      BlockchainReadState(const BlockchainReadState&) = default;
      BlockchainReadState(BlockchainReadState&& state);

      size_t size() const;
    };

    class BlockchainReadResult {
    public:
      BlockchainReadResult(BlockchainReadState state);
      ~BlockchainReadResult();

      BlockchainReadResult(BlockchainReadResult&& result);

      const std::unordered_map<uint32_t, std::vector<crypto::KeyImage>>& getSpentKeyImagesByBlock() const;
      const std::unordered_map<crypto::KeyImage, uint32_t>& getBlockIndexesBySpentKeyImages() const;
      const std::unordered_map<crypto::Hash, ExtendedTransactionInfo>& getCachedTransactions() const;
      const std::unordered_map<uint32_t, std::vector<crypto::Hash>>& getTransactionHashesByBlocks() const;
      const std::unordered_map<uint32_t, CachedBlockInfo>& getCachedBlocks() const;
      const std::unordered_map<crypto::Hash, uint32_t>& getBlockIndexesByBlockHashes() const;
      const std::unordered_map<IBlockchainCache::Amount, uint32_t>& getKeyOutputGlobalIndexesCountForAmounts() const;
      const std::unordered_map<std::pair<IBlockchainCache::Amount, uint32_t>, PackedOutIndex>& getKeyOutputGlobalIndexesForAmounts() const;
      const std::unordered_map<uint32_t, RawBlock>& getRawBlocks() const;
      const std::pair<uint32_t, bool>& getLastBlockIndex() const;
      const std::unordered_map<uint64_t, uint32_t>& getClosestTimestampBlockIndex() const;
      uint32_t getKeyOutputAmountsCount() const;
      const std::unordered_map<crypto::Hash, uint32_t>& getTransactionCountByPaymentIds() const;
      const std::unordered_map<std::pair<crypto::Hash, uint32_t>, crypto::Hash>& getTransactionHashesByPaymentIds() const;
      const std::unordered_map<uint64_t, std::vector<crypto::Hash> >& getBlockHashesByTimestamp() const;
      const std::pair<uint64_t, bool>& getTransactionsCount() const;
      const KeyOutputKeyResult& getKeyOutputInfo() const;

    private:
      BlockchainReadState state;
    };

    class BlockchainReadBatch : public IReadBatch {
    public:
      BlockchainReadBatch();
      ~BlockchainReadBatch();

      BlockchainReadBatch& requestSpentKeyImagesByBlock(uint32_t blockIndex);
      BlockchainReadBatch& requestBlockIndexBySpentKeyImage(const crypto::KeyImage& keyImage);
      BlockchainReadBatch& requestCachedTransaction(const crypto::Hash& txHash);
      BlockchainReadBatch& requestCachedTransactions(const std::vector<crypto::Hash> &transactions);
      BlockchainReadBatch& requestTransactionHashesByBlock(uint32_t blockIndex);
      BlockchainReadBatch& requestCachedBlock(uint32_t blockIndex);
      BlockchainReadBatch& requestBlockIndexByBlockHash(const crypto::Hash& blockHash);
      BlockchainReadBatch& requestKeyOutputGlobalIndexesCountForAmount(IBlockchainCache::Amount amount);
      BlockchainReadBatch& requestKeyOutputGlobalIndexForAmount(IBlockchainCache::Amount amount, uint32_t outputIndexWithinAmout);
      BlockchainReadBatch& requestRawBlock(uint32_t blockIndex);
      BlockchainReadBatch& requestRawBlocks(uint64_t startHeight, uint64_t endHeight);
      BlockchainReadBatch& requestLastBlockIndex();
      BlockchainReadBatch& requestClosestTimestampBlockIndex(uint64_t timestamp);
      BlockchainReadBatch& requestKeyOutputAmountsCount();
      BlockchainReadBatch& requestTransactionCountByPaymentId(const crypto::Hash& paymentId);
      BlockchainReadBatch& requestTransactionHashByPaymentId(const crypto::Hash& paymentId, uint32_t transactionIndexWithinPaymentId);
      BlockchainReadBatch& requestBlockHashesByTimestamp(uint64_t timestamp);
      BlockchainReadBatch& requestTransactionsCount();
      BlockchainReadBatch& requestKeyOutputInfo(IBlockchainCache::Amount amount, IBlockchainCache::GlobalOutputIndex globalIndex);

      std::vector<std::string> getRawKeys() const override;
      void submitRawResult(const std::vector<std::string>& values, const std::vector<bool>& resultStates) override;

      BlockchainReadResult extractResult();

    private:
      bool resultSubmitted = false;
      BlockchainReadState state;
    };
}
