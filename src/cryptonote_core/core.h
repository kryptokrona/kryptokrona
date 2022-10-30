// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once
#include <ctime>
#include <vector>
#include <unordered_map>
#include "blockchain_cache.h"
#include "blockchain_messages.h"
#include "cached_block.h"
#include "cached_transaction.h"
#include "currency.h"
#include "checkpoints.h"
#include "iblockchain_cache.h"
#include "iblockchain_cache_factory.h"
#include "icore.h"
#include "icore_information.h"
#include "imain_chain_storage.h"
#include "itransaction_pool.h"
#include "itransaction_pool_cleaner.h"
#include "iupgrade_manager.h"
#include <logging/logger_message.h>
#include "message_queue.h"
#include "transaction_validator_state.h"

#include <system/context_group.h>

#include <wallet_types.h>

namespace cryptonote
{
    class Core : public ICore, public ICoreInformation {
    public:
      Core(const Currency& currency, std::shared_ptr<logging::ILogger> logger, Checkpoints&& checkpoints, system::Dispatcher& dispatcher,
           std::unique_ptr<IBlockchainCacheFactory>&& blockchainCacheFactory, std::unique_ptr<IMainChainStorage>&& mainChainStorage);
      virtual ~Core();

      virtual bool addMessageQueue(MessageQueue<BlockchainMessage>&  messageQueue) override;
      virtual bool removeMessageQueue(MessageQueue<BlockchainMessage>& messageQueue) override;

      virtual uint32_t getTopBlockIndex() const override;
      virtual crypto::Hash getTopBlockHash() const override;
      virtual crypto::Hash getBlockHashByIndex(uint32_t blockIndex) const override;
      virtual uint64_t getBlockTimestampByIndex(uint32_t blockIndex) const override;

      virtual bool hasBlock(const crypto::Hash& blockHash) const override;
      virtual BlockTemplate getBlockByIndex(uint32_t index) const override;
      virtual BlockTemplate getBlockByHash(const crypto::Hash& blockHash) const override;

      virtual std::vector<crypto::Hash> buildSparseChain() const override;
      virtual std::vector<crypto::Hash> findBlockchainSupplement(const std::vector<crypto::Hash>& remoteBlockIds, size_t maxCount,
        uint32_t& totalBlockCount, uint32_t& startBlockIndex) const override;

      virtual std::vector<RawBlock> getBlocks(uint32_t minIndex, uint32_t count) const override;
      virtual void getBlocks(const std::vector<crypto::Hash>& blockHashes, std::vector<RawBlock>& blocks, std::vector<crypto::Hash>& missedHashes) const override;
      virtual bool queryBlocks(const std::vector<crypto::Hash>& blockHashes, uint64_t timestamp,
        uint32_t& startIndex, uint32_t& currentIndex, uint32_t& fullOffset, std::vector<BlockFullInfo>& entries) const override;
      virtual bool queryBlocksLite(const std::vector<crypto::Hash>& knownBlockHashes, uint64_t timestamp,
        uint32_t& startIndex, uint32_t& currentIndex, uint32_t& fullOffset, std::vector<BlockShortInfo>& entries) const override;
      virtual bool queryBlocksDetailed(const std::vector<crypto::Hash>& knownBlockHashes, uint64_t timestamp,
        uint64_t& startIndex, uint64_t& currentIndex, uint64_t& fullOffset, std::vector<BlockDetails>& entries, uint32_t blockCount) const override;

      virtual bool getWalletSyncData(
        const std::vector<crypto::Hash> &knownBlockHashes,
        const uint64_t startHeight,
        const uint64_t startTimestamp,
        const uint64_t blockCount,
        std::vector<WalletTypes::WalletBlockInfo> &walletBlocks) const override;

      virtual bool getTransactionsStatus(
        std::unordered_set<crypto::Hash> transactionHashes,
        std::unordered_set<crypto::Hash> &transactionsInPool,
        std::unordered_set<crypto::Hash> &transactionsInBlock,
        std::unordered_set<crypto::Hash> &transactionsUnknown) const override;

      virtual bool hasTransaction(const crypto::Hash& transactionHash) const override;
      virtual std::optional<BinaryArray> getTransaction(const crypto::Hash& transactionHash) const override;
      virtual void getTransactions(const std::vector<crypto::Hash>& transactionHashes, std::vector<BinaryArray>& transactions, std::vector<crypto::Hash>& missedHashes) const override;

      virtual uint64_t getBlockDifficulty(uint32_t blockIndex) const override;
      virtual uint64_t getDifficultyForNextBlock() const override;

      virtual std::error_code addBlock(const CachedBlock& cachedBlock, RawBlock&& rawBlock) override;
      virtual std::error_code addBlock(RawBlock&& rawBlock) override;

      virtual std::error_code submitBlock(BinaryArray&& rawBlockTemplate) override;

      virtual bool getTransactionGlobalIndexes(const crypto::Hash& transactionHash, std::vector<uint32_t>& globalIndexes) const override;
      virtual bool getRandomOutputs(uint64_t amount, uint16_t count, std::vector<uint32_t>& globalIndexes, std::vector<crypto::PublicKey>& publicKeys) const override;

      virtual bool getGlobalIndexesForRange(
        const uint64_t startHeight,
        const uint64_t endHeight,
        std::unordered_map<crypto::Hash, std::vector<uint64_t>> &indexes) const override;

      virtual bool addTransactionToPool(const BinaryArray& transactionBinaryArray) override;

      virtual std::vector<crypto::Hash> getPoolTransactionHashes() const override;
      virtual std::tuple<bool, BinaryArray> getPoolTransaction(const crypto::Hash& transactionHash) const override;
      virtual bool getPoolChanges(const crypto::Hash& lastBlockHash, const std::vector<crypto::Hash>& knownHashes, std::vector<BinaryArray>& addedTransactions,
        std::vector<crypto::Hash>& deletedTransactions) const override;
      virtual bool getPoolChangesLite(const crypto::Hash& lastBlockHash, const std::vector<crypto::Hash>& knownHashes, std::vector<TransactionPrefixInfo>& addedTransactions,
        std::vector<crypto::Hash>& deletedTransactions) const override;

      virtual bool getBlockTemplate(BlockTemplate& b, const AccountPublicAddress& adr, const BinaryArray& extraNonce, uint64_t& difficulty, uint32_t& height) const override;

      virtual CoreStatistics getCoreStatistics() const override;

      virtual std::time_t getStartTime() const;

      //ICoreInformation
      virtual size_t getPoolTransactionCount() const override;
      virtual size_t getBlockchainTransactionCount() const override;
      virtual size_t getAlternativeBlockCount() const override;
      virtual std::vector<Transaction> getPoolTransactions() const override;

      const Currency& getCurrency() const;

      virtual void save() override;
      virtual void load() override;

      virtual BlockDetails getBlockDetails(const crypto::Hash& blockHash) const override;
      BlockDetails getBlockDetails(const uint32_t blockHeight) const;
      virtual TransactionDetails getTransactionDetails(const crypto::Hash& transactionHash) const override;
      virtual std::vector<crypto::Hash> getBlockHashesByTimestamps(uint64_t timestampBegin, size_t secondsCount) const override;
      virtual std::vector<crypto::Hash> getTransactionHashesByPaymentId(const crypto::Hash& paymentId) const override;

      virtual uint64_t get_current_blockchain_height() const;

    private:
      const Currency& currency;
      system::Dispatcher& dispatcher;
      system::ContextGroup contextGroup;
      logging::LoggerRef logger;
      Checkpoints checkpoints;
      std::unique_ptr<IUpgradeManager> upgradeManager;
      std::vector<std::unique_ptr<IBlockchainCache>> chainsStorage;
      std::vector<IBlockchainCache*> chainsLeaves;
      std::unique_ptr<ITransactionPoolCleanWrapper> transactionPool;
      std::unordered_set<IBlockchainCache*> mainChainSet;

      std::string dataFolder;

      IntrusiveLinkedList<MessageQueue<BlockchainMessage>> queueList;
      std::unique_ptr<IBlockchainCacheFactory> blockchainCacheFactory;
      std::unique_ptr<IMainChainStorage> mainChainStorage;
      bool initialized;

      time_t start_time;

      size_t blockMedianSize;

      void throwIfNotInitialized() const;
      bool extractTransactions(const std::vector<BinaryArray>& rawTransactions, std::vector<CachedTransaction>& transactions, uint64_t& cumulativeSize);

      std::error_code validateSemantic(const Transaction& transaction, uint64_t& fee, uint32_t blockIndex);
      std::error_code validateTransaction(const CachedTransaction& transaction, TransactionValidatorState& state, IBlockchainCache* cache, uint64_t& fee, uint32_t blockIndex);

      uint32_t findBlockchainSupplement(const std::vector<crypto::Hash>& remoteBlockIds) const;
      std::vector<crypto::Hash> getBlockHashes(uint32_t startBlockIndex, uint32_t maxCount) const;

      std::error_code validateBlock(const CachedBlock& block, IBlockchainCache* cache, uint64_t& minerReward);

      uint64_t getAdjustedTime() const;
      void updateMainChainSet();
      IBlockchainCache* findSegmentContainingBlock(const crypto::Hash& blockHash) const;
      IBlockchainCache* findSegmentContainingBlock(uint32_t blockHeight) const;
      IBlockchainCache* findMainChainSegmentContainingBlock(const crypto::Hash& blockHash) const;
      IBlockchainCache* findAlternativeSegmentContainingBlock(const crypto::Hash& blockHash) const;

      IBlockchainCache* findMainChainSegmentContainingBlock(uint32_t blockIndex) const;
      IBlockchainCache* findAlternativeSegmentContainingBlock(uint32_t blockIndex) const;

      IBlockchainCache* findSegmentContainingTransaction(const crypto::Hash& transactionHash) const;

      BlockTemplate restoreBlockTemplate(IBlockchainCache* blockchainCache, uint32_t blockIndex) const;
      std::vector<crypto::Hash> doBuildSparseChain(const crypto::Hash& blockHash) const;

      RawBlock getRawBlock(IBlockchainCache* segment, uint32_t blockIndex) const;

      size_t pushBlockHashes(uint32_t startIndex, uint32_t fullOffset, size_t maxItemsCount, std::vector<BlockShortInfo>& entries) const;
      size_t pushBlockHashes(uint32_t startIndex, uint32_t fullOffset, size_t maxItemsCount, std::vector<BlockFullInfo>& entries) const;
      size_t pushBlockHashes(uint32_t startIndex, uint32_t fullOffset, size_t maxItemsCount, std::vector<BlockDetails>& entries) const;
      bool notifyObservers(BlockchainMessage&& msg);
      void fillQueryBlockFullInfo(uint32_t fullOffset, uint32_t currentIndex, size_t maxItemsCount, std::vector<BlockFullInfo>& entries) const;
      void fillQueryBlockShortInfo(uint32_t fullOffset, uint32_t currentIndex, size_t maxItemsCount, std::vector<BlockShortInfo>& entries) const;
      void fillQueryBlockDetails(uint32_t fullOffset, uint32_t currentIndex, size_t maxItemsCount, std::vector<BlockDetails>& entries) const;

      void getTransactionPoolDifference(const std::vector<crypto::Hash>& knownHashes, std::vector<crypto::Hash>& newTransactions, std::vector<crypto::Hash>& deletedTransactions) const;

      uint8_t getBlockMajorVersionForHeight(uint32_t height) const;
      size_t calculateCumulativeBlocksizeLimit(uint32_t height) const;

      bool validateBlockTemplateTransaction(
        const CachedTransaction &cachedTransaction,
        const uint64_t blockHeight) const;

      void fillBlockTemplate(
        BlockTemplate& block,
        const size_t medianSize,
        const size_t maxCumulativeSize,
        const uint64_t height,
        size_t& transactionsSize,
        uint64_t& fee) const;

      void deleteAlternativeChains();
      void deleteLeaf(size_t leafIndex);
      void mergeMainChainSegments();
      void mergeSegments(IBlockchainCache* acceptingSegment, IBlockchainCache* segment);
      TransactionDetails getTransactionDetails(const crypto::Hash& transactionHash, IBlockchainCache* segment, bool foundInPool) const;
      void notifyOnSuccess(error::AddBlockErrorCode opResult, uint32_t previousBlockIndex, const CachedBlock& cachedBlock,
                           const IBlockchainCache& cache);
      void copyTransactionsToPool(IBlockchainCache* alt);

      void actualizePoolTransactions();
      void actualizePoolTransactionsLite(const TransactionValidatorState& validatorState); //Checks pool txs only for double spend.

      void transactionPoolCleaningProcedure();
      void huginCleaningProcedure();
      void updateBlockMedianSize();
      bool addTransactionToPool(CachedTransaction&& cachedTransaction);
      bool isTransactionValidForPool(const CachedTransaction& cachedTransaction, TransactionValidatorState& validatorState);

      void initRootSegment();
      void importBlocksFromStorage();
      void cutSegment(IBlockchainCache& segment, uint32_t startIndex);

      void switchMainChainStorage(uint32_t splitBlockIndex, IBlockchainCache& newChain);

      static WalletTypes::RawCoinbaseTransaction getRawCoinbaseTransaction(
        const cryptonote::Transaction &t);

      static WalletTypes::RawTransaction getRawTransaction(
        const std::vector<uint8_t> &rawTX);

      static crypto::PublicKey getPubKeyFromExtra(const std::vector<uint8_t> &extra);

      static std::string getPaymentIDFromExtra(const std::vector<uint8_t> &extra);
    };
}
