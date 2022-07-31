// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <system_error>
#include <vector>

#include "crypto/crypto.h"
#include "cryptonote_core/cryptonote_basic.h"
#include "cryptonote_protocol/cryptonote_protocol_definitions.h"
#include "rpc/core_rpc_server_commands_definitions.h"

#include "blockchain_explorer_data.h"
#include "itransaction.h"

#include <wallet_types.h>

namespace cryptonote
{
    class INodeObserver {
    public:
      virtual ~INodeObserver() {}
      virtual void peerCountUpdated(size_t count) {}
      virtual void localBlockchainUpdated(uint32_t height) {}
      virtual void lastKnownBlockHeightUpdated(uint32_t height) {}
      virtual void poolChanged() {}
      virtual void blockchainSynchronized(uint32_t topHeight) {}
    };

    struct OutEntry {
      uint32_t outGlobalIndex;
      Crypto::PublicKey outKey;
    };

    struct OutsForAmount {
      uint64_t amount;
      std::vector<OutEntry> outs;
    };

    struct TransactionShortInfo {
      Crypto::Hash txId;
      TransactionPrefix txPrefix;
    };

    struct BlockShortEntry {
      Crypto::Hash blockHash;
      bool hasBlock;
      CryptoNote::BlockTemplate block;
      std::vector<TransactionShortInfo> txsShortInfo;
    };

    struct BlockHeaderInfo {
      uint32_t index;
      uint8_t majorVersion;
      uint8_t minorVersion;
      uint64_t timestamp;
      Crypto::Hash hash;
      Crypto::Hash prevHash;
      uint32_t nonce;
      bool isAlternative;
      uint32_t depth; // last block index = current block index + depth
      uint64_t difficulty;
      uint64_t reward;
    };

    class INode {
    public:
      typedef std::function<void(std::error_code)> Callback;

      virtual ~INode() {}
      virtual bool addObserver(INodeObserver* observer) = 0;
      virtual bool removeObserver(INodeObserver* observer) = 0;

      //precondition: must be called in dispatcher's thread
      virtual void init(const Callback& callback) = 0;
      //precondition: must be called in dispatcher's thread
      virtual bool shutdown() = 0;

      //precondition: all of following methods must not be invoked in dispatcher's thread
      virtual size_t getPeerCount() const = 0;
      virtual uint32_t getLastLocalBlockHeight() const = 0;
      virtual uint32_t getLastKnownBlockHeight() const = 0;
      virtual uint32_t getLocalBlockCount() const = 0;
      virtual uint32_t getKnownBlockCount() const = 0;
      virtual uint64_t getNodeHeight() const = 0;

      virtual void getFeeInfo() = 0;

      virtual void getBlockHashesByTimestamps(uint64_t timestampBegin, size_t secondsCount, std::vector<Crypto::Hash>& blockHashes, const Callback& callback) = 0;
      virtual void getTransactionHashesByPaymentId(const Crypto::Hash& paymentId, std::vector<Crypto::Hash>& transactionHashes, const Callback& callback) = 0;

      virtual BlockHeaderInfo getLastLocalBlockHeaderInfo() const = 0;

      virtual void relayTransaction(const Transaction& transaction, const Callback& callback) = 0;
      virtual void getRandomOutsByAmounts(std::vector<uint64_t>&& amounts, uint16_t outsCount, std::vector<RandomOuts>& result, const Callback& callback) = 0;
      virtual void getTransactionOutsGlobalIndices(const Crypto::Hash& transactionHash, std::vector<uint32_t>& outsGlobalIndices, const Callback& callback) = 0;

      virtual void getGlobalIndexesForRange(
        const uint64_t startHeight,
        const uint64_t endHeight,
        std::unordered_map<Crypto::Hash, std::vector<uint64_t>> &indexes,
        const Callback &callback) = 0;

      virtual void getTransactionsStatus(
        const std::unordered_set<Crypto::Hash> transactionHashes,
        std::unordered_set<Crypto::Hash> &transactionsInPool,
        std::unordered_set<Crypto::Hash> &transactionsInBlock,
        std::unordered_set<Crypto::Hash> &transactionsUnknown,
        const Callback &callback) = 0;

      virtual void queryBlocks(std::vector<Crypto::Hash>&& knownBlockIds, uint64_t timestamp, std::vector<BlockShortEntry>& newBlocks, uint32_t& startHeight, const Callback& callback) = 0;
      virtual void getWalletSyncData(std::vector<Crypto::Hash>&& knownBlockIds, uint64_t startHeight, uint64_t startTimestamp, std::vector<WalletTypes::WalletBlockInfo>& newBlocks, const Callback& callback) = 0;
      virtual void getPoolSymmetricDifference(std::vector<Crypto::Hash>&& knownPoolTxIds, Crypto::Hash knownBlockId, bool& isBcActual, std::vector<std::unique_ptr<ITransactionReader>>& newTxs, std::vector<Crypto::Hash>& deletedTxIds, const Callback& callback) = 0;

      virtual void getBlocks(const std::vector<uint32_t>& blockHeights, std::vector<std::vector<BlockDetails>>& blocks, const Callback& callback) = 0;
      virtual void getBlocks(const std::vector<Crypto::Hash>& blockHashes, std::vector<BlockDetails>& blocks, const Callback& callback) = 0;
      virtual void getBlock(const uint32_t blockHeight, BlockDetails &block, const Callback& callback) = 0;
      virtual void getTransactions(const std::vector<Crypto::Hash>& transactionHashes, std::vector<TransactionDetails>& transactions, const Callback& callback) = 0;
      virtual void isSynchronized(bool& syncStatus, const Callback& callback) = 0;
      virtual std::string feeAddress() = 0;
      virtual uint32_t feeAmount() = 0;
    };
}
