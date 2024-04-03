// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "blockchain_read_batch.h"

#include <boost/range/combine.hpp>

#include "dbutils.h"

using namespace cryptonote;

BlockchainReadBatch::BlockchainReadBatch()
{
}

BlockchainReadBatch::~BlockchainReadBatch()
{
}

BlockchainReadBatch &BlockchainReadBatch::requestSpentKeyImagesByBlock(uint32_t blockIndex)
{
    state.spentKeyImagesByBlock.emplace(blockIndex, std::vector<crypto::KeyImage>());
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestBlockIndexBySpentKeyImage(const crypto::KeyImage &keyImage)
{
    state.blockIndexesBySpentKeyImages.emplace(keyImage, 0);
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestCachedTransaction(const crypto::Hash &txHash)
{
    state.cachedTransactions.emplace(txHash, ExtendedTransactionInfo());
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestCachedTransactions(const std::vector<crypto::Hash> &transactions)
{
    for (const auto hash : transactions)
    {
        state.cachedTransactions.emplace(hash, ExtendedTransactionInfo());
    }

    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestTransactionHashesByBlock(uint32_t blockIndex)
{
    state.transactionHashesByBlocks.emplace(blockIndex, std::vector<crypto::Hash>());
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestCachedBlock(uint32_t blockIndex)
{
    state.cachedBlocks.emplace(blockIndex, CachedBlockInfo());
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestBlockIndexByBlockHash(const crypto::Hash &blockHash)
{
    state.blockIndexesByBlockHashes.emplace(blockHash, 0);
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestKeyOutputGlobalIndexesCountForAmount(IBlockchainCache::Amount amount)
{
    state.keyOutputGlobalIndexesCountForAmounts.emplace(amount, 0);
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestKeyOutputGlobalIndexForAmount(IBlockchainCache::Amount amount, uint32_t outputIndexWithinAmout)
{
    state.keyOutputGlobalIndexesForAmounts.emplace(std::make_pair(amount, outputIndexWithinAmout), PackedOutIndex());
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestRawBlock(uint32_t blockIndex)
{
    state.rawBlocks.emplace(blockIndex, RawBlock());
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestRawBlocks(uint64_t startHeight, uint64_t endHeight)
{
    for (uint64_t i = startHeight; i < endHeight; i++)
    {
        state.rawBlocks.emplace(i, RawBlock());
    }

    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestLastBlockIndex()
{
    state.lastBlockIndex.second = true;
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestClosestTimestampBlockIndex(uint64_t timestamp)
{
    state.closestTimestampBlockIndex[timestamp];
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestKeyOutputAmountsCount()
{
    state.keyOutputAmountsCount.second = true;
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestTransactionCountByPaymentId(const crypto::Hash &paymentId)
{
    state.transactionCountsByPaymentIds.emplace(paymentId, 0);
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestTransactionHashByPaymentId(const crypto::Hash &paymentId, uint32_t transactionIndexWithinPaymentId)
{
    state.transactionHashesByPaymentIds.emplace(std::make_pair(paymentId, transactionIndexWithinPaymentId), NULL_HASH);
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestBlockHashesByTimestamp(uint64_t timestamp)
{
    state.blockHashesByTimestamp.emplace(timestamp, std::vector<crypto::Hash>());
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestTransactionsCount()
{
    state.transactionsCount.second = true;
    return *this;
}

BlockchainReadBatch &BlockchainReadBatch::requestKeyOutputInfo(IBlockchainCache::Amount amount, IBlockchainCache::GlobalOutputIndex globalIndex)
{
    state.keyOutputKeys.emplace(std::make_pair(amount, globalIndex), KeyOutputInfo{});
    return *this;
}

BlockchainReadResult BlockchainReadBatch::extractResult()
{
    assert(resultSubmitted);
    auto st = std::move(state);
    state.lastBlockIndex = {0, false};
    state.keyOutputAmountsCount = {{}, false};

    resultSubmitted = false;
    return BlockchainReadResult(st);
}

std::vector<std::string> BlockchainReadBatch::getRawKeys() const
{
    std::vector<std::string> rawKeys;
    rawKeys.reserve(state.size());

    db::serializeKeys(rawKeys, db::BLOCK_INDEX_TO_KEY_IMAGE_PREFIX, state.spentKeyImagesByBlock);
    db::serializeKeys(rawKeys, db::KEY_IMAGE_TO_BLOCK_INDEX_PREFIX, state.blockIndexesBySpentKeyImages);
    db::serializeKeys(rawKeys, db::TRANSACTION_HASH_TO_TRANSACTION_INFO_PREFIX, state.cachedTransactions);
    db::serializeKeys(rawKeys, db::BLOCK_INDEX_TO_TX_HASHES_PREFIX, state.transactionHashesByBlocks);
    db::serializeKeys(rawKeys, db::BLOCK_INDEX_TO_BLOCK_INFO_PREFIX, state.cachedBlocks);
    db::serializeKeys(rawKeys, db::BLOCK_HASH_TO_BLOCK_INDEX_PREFIX, state.blockIndexesByBlockHashes);
    db::serializeKeys(rawKeys, db::KEY_OUTPUT_AMOUNT_PREFIX, state.keyOutputGlobalIndexesCountForAmounts);
    db::serializeKeys(rawKeys, db::KEY_OUTPUT_AMOUNT_PREFIX, state.keyOutputGlobalIndexesForAmounts);
    db::serializeKeys(rawKeys, db::BLOCK_INDEX_TO_RAW_BLOCK_PREFIX, state.rawBlocks);
    db::serializeKeys(rawKeys, db::CLOSEST_TIMESTAMP_BLOCK_INDEX_PREFIX, state.closestTimestampBlockIndex);
    db::serializeKeys(rawKeys, db::KEY_OUTPUT_AMOUNTS_COUNT_PREFIX, state.keyOutputAmounts);
    db::serializeKeys(rawKeys, db::PAYMENT_ID_TO_TX_HASH_PREFIX, state.transactionCountsByPaymentIds);
    db::serializeKeys(rawKeys, db::PAYMENT_ID_TO_TX_HASH_PREFIX, state.transactionHashesByPaymentIds);
    db::serializeKeys(rawKeys, db::TIMESTAMP_TO_BLOCKHASHES_PREFIX, state.blockHashesByTimestamp);
    db::serializeKeys(rawKeys, db::KEY_OUTPUT_KEY_PREFIX, state.keyOutputKeys);

    if (state.lastBlockIndex.second)
    {
        rawKeys.emplace_back(db::serializeKey(db::BLOCK_INDEX_TO_BLOCK_HASH_PREFIX, db::LAST_BLOCK_INDEX_KEY));
    }

    if (state.keyOutputAmountsCount.second)
    {
        rawKeys.emplace_back(db::serializeKey(db::KEY_OUTPUT_AMOUNTS_COUNT_PREFIX, db::KEY_OUTPUT_AMOUNTS_COUNT_KEY));
    }

    if (state.transactionsCount.second)
    {
        rawKeys.emplace_back(db::serializeKey(db::TRANSACTION_HASH_TO_TRANSACTION_INFO_PREFIX, db::TRANSACTIONS_COUNT_KEY));
    }

    assert(!rawKeys.empty());
    return rawKeys;
}

BlockchainReadResult::BlockchainReadResult(BlockchainReadState _state) : state(std::move(_state))
{
}

BlockchainReadResult::~BlockchainReadResult()
{
}

const std::unordered_map<uint32_t, std::vector<crypto::KeyImage>> &BlockchainReadResult::getSpentKeyImagesByBlock() const
{
    return state.spentKeyImagesByBlock;
}

const std::unordered_map<crypto::KeyImage, uint32_t> &BlockchainReadResult::getBlockIndexesBySpentKeyImages() const
{
    return state.blockIndexesBySpentKeyImages;
}

const std::unordered_map<crypto::Hash, ExtendedTransactionInfo> &BlockchainReadResult::getCachedTransactions() const
{
    return state.cachedTransactions;
}

const std::unordered_map<uint32_t, std::vector<crypto::Hash>> &BlockchainReadResult::getTransactionHashesByBlocks() const
{
    return state.transactionHashesByBlocks;
}

const std::unordered_map<uint32_t, CachedBlockInfo> &BlockchainReadResult::getCachedBlocks() const
{
    return state.cachedBlocks;
}

const std::unordered_map<crypto::Hash, uint32_t> &BlockchainReadResult::getBlockIndexesByBlockHashes() const
{
    return state.blockIndexesByBlockHashes;
}

const std::unordered_map<IBlockchainCache::Amount, uint32_t> &BlockchainReadResult::getKeyOutputGlobalIndexesCountForAmounts() const
{
    return state.keyOutputGlobalIndexesCountForAmounts;
}

const std::unordered_map<std::pair<IBlockchainCache::Amount, uint32_t>, PackedOutIndex> &BlockchainReadResult::getKeyOutputGlobalIndexesForAmounts() const
{
    return state.keyOutputGlobalIndexesForAmounts;
}

const std::unordered_map<uint32_t, RawBlock> &BlockchainReadResult::getRawBlocks() const
{
    return state.rawBlocks;
}

const std::pair<uint32_t, bool> &BlockchainReadResult::getLastBlockIndex() const
{
    return state.lastBlockIndex;
}

const std::unordered_map<uint64_t, uint32_t> &BlockchainReadResult::getClosestTimestampBlockIndex() const
{
    return state.closestTimestampBlockIndex;
}

uint32_t BlockchainReadResult::getKeyOutputAmountsCount() const
{
    return state.keyOutputAmountsCount.first;
}

const std::unordered_map<crypto::Hash, uint32_t> &BlockchainReadResult::getTransactionCountByPaymentIds() const
{
    return state.transactionCountsByPaymentIds;
}

const std::unordered_map<std::pair<crypto::Hash, uint32_t>, crypto::Hash> &BlockchainReadResult::getTransactionHashesByPaymentIds() const
{
    return state.transactionHashesByPaymentIds;
}

const std::unordered_map<uint64_t, std::vector<crypto::Hash>> &BlockchainReadResult::getBlockHashesByTimestamp() const
{
    return state.blockHashesByTimestamp;
}

const std::pair<uint64_t, bool> &BlockchainReadResult::getTransactionsCount() const
{
    return state.transactionsCount;
}

const KeyOutputKeyResult &BlockchainReadResult::getKeyOutputInfo() const
{
    return state.keyOutputKeys;
}

void BlockchainReadBatch::submitRawResult(const std::vector<std::string> &values, const std::vector<bool> &resultStates)
{
    assert(state.size() == values.size());
    assert(values.size() == resultStates.size());
    auto range = boost::combine(values, resultStates);
    auto iter = range.begin();

    db::deserializeValues(state.spentKeyImagesByBlock, iter, db::BLOCK_INDEX_TO_KEY_IMAGE_PREFIX);
    db::deserializeValues(state.blockIndexesBySpentKeyImages, iter, db::KEY_IMAGE_TO_BLOCK_INDEX_PREFIX);
    db::deserializeValues(state.cachedTransactions, iter, db::TRANSACTION_HASH_TO_TRANSACTION_INFO_PREFIX);
    db::deserializeValues(state.transactionHashesByBlocks, iter, db::BLOCK_INDEX_TO_TX_HASHES_PREFIX);
    db::deserializeValues(state.cachedBlocks, iter, db::BLOCK_INDEX_TO_BLOCK_INFO_PREFIX);
    db::deserializeValues(state.blockIndexesByBlockHashes, iter, db::BLOCK_HASH_TO_BLOCK_INDEX_PREFIX);
    db::deserializeValues(state.keyOutputGlobalIndexesCountForAmounts, iter, db::KEY_OUTPUT_AMOUNT_PREFIX);
    db::deserializeValues(state.keyOutputGlobalIndexesForAmounts, iter, db::KEY_OUTPUT_AMOUNT_PREFIX);
    db::deserializeValues(state.rawBlocks, iter, db::BLOCK_INDEX_TO_RAW_BLOCK_PREFIX);
    db::deserializeValues(state.closestTimestampBlockIndex, iter, db::CLOSEST_TIMESTAMP_BLOCK_INDEX_PREFIX);
    db::deserializeValues(state.keyOutputAmounts, iter, db::KEY_OUTPUT_AMOUNTS_COUNT_PREFIX);
    db::deserializeValues(state.transactionCountsByPaymentIds, iter, db::PAYMENT_ID_TO_TX_HASH_PREFIX);
    db::deserializeValues(state.transactionHashesByPaymentIds, iter, db::PAYMENT_ID_TO_TX_HASH_PREFIX);
    db::deserializeValues(state.blockHashesByTimestamp, iter, db::TIMESTAMP_TO_BLOCKHASHES_PREFIX);
    db::deserializeValues(state.keyOutputKeys, iter, db::KEY_OUTPUT_KEY_PREFIX);

    db::deserializeValue(state.lastBlockIndex, iter, db::BLOCK_INDEX_TO_BLOCK_HASH_PREFIX);
    db::deserializeValue(state.keyOutputAmountsCount, iter, db::KEY_OUTPUT_AMOUNTS_COUNT_PREFIX);
    db::deserializeValue(state.transactionsCount, iter, db::TRANSACTION_HASH_TO_TRANSACTION_INFO_PREFIX);

    assert(iter == range.end());

    resultSubmitted = true;
}

BlockchainReadState::BlockchainReadState(BlockchainReadState &&state) : spentKeyImagesByBlock(std::move(state.spentKeyImagesByBlock)),
                                                                        blockIndexesBySpentKeyImages(std::move(state.blockIndexesBySpentKeyImages)),
                                                                        cachedTransactions(std::move(state.cachedTransactions)),
                                                                        transactionHashesByBlocks(std::move(state.transactionHashesByBlocks)),
                                                                        cachedBlocks(std::move(state.cachedBlocks)),
                                                                        blockIndexesByBlockHashes(std::move(state.blockIndexesByBlockHashes)),
                                                                        keyOutputGlobalIndexesCountForAmounts(std::move(state.keyOutputGlobalIndexesCountForAmounts)),
                                                                        keyOutputGlobalIndexesForAmounts(std::move(state.keyOutputGlobalIndexesForAmounts)),
                                                                        rawBlocks(std::move(state.rawBlocks)),
                                                                        blockHashesByTimestamp(std::move(state.blockHashesByTimestamp)),
                                                                        keyOutputKeys(std::move(state.keyOutputKeys)),
                                                                        closestTimestampBlockIndex(std::move(state.closestTimestampBlockIndex)),
                                                                        lastBlockIndex(std::move(state.lastBlockIndex)),
                                                                        keyOutputAmountsCount(std::move(state.keyOutputAmountsCount)),
                                                                        keyOutputAmounts(std::move(state.keyOutputAmounts)),
                                                                        transactionCountsByPaymentIds(std::move(state.transactionCountsByPaymentIds)),
                                                                        transactionHashesByPaymentIds(std::move(state.transactionHashesByPaymentIds)),
                                                                        transactionsCount(std::move(state.transactionsCount))
{
}

size_t BlockchainReadState::size() const
{
    return spentKeyImagesByBlock.size() +
           blockIndexesBySpentKeyImages.size() +
           cachedTransactions.size() +
           transactionHashesByBlocks.size() +
           cachedBlocks.size() +
           blockIndexesByBlockHashes.size() +
           keyOutputGlobalIndexesCountForAmounts.size() +
           keyOutputGlobalIndexesForAmounts.size() +
           rawBlocks.size() +
           closestTimestampBlockIndex.size() +
           keyOutputAmounts.size() +
           transactionCountsByPaymentIds.size() +
           transactionHashesByPaymentIds.size() +
           blockHashesByTimestamp.size() +
           keyOutputKeys.size() +
           (lastBlockIndex.second ? 1 : 0) +
           (keyOutputAmountsCount.second ? 1 : 0) +
           (transactionsCount.second ? 1 : 0);
}

BlockchainReadResult::BlockchainReadResult(BlockchainReadResult &&result) : state(std::move(result.state))
{
}
