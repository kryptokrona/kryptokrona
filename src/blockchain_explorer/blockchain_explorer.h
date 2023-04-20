// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <mutex>
#include <atomic>
#include <unordered_set>

#include "iblockchain_explorer.h"
#include "inode.h"

#include "blockchain_explorer_errors.h"
#include "common/observer_manager.h"
#include "serialization/binary_input_stream_serializer.h"
#include "serialization/binary_output_stream_serializer.h"
#include "wallet/wallet_async_context_counter.h"

#include "logging/logger_ref.h"

namespace cryptonote
{

    enum State
    {
        NOT_INITIALIZED,
        INITIALIZED
    };

    class BlockchainExplorer : public IBlockchainExplorer, public INodeObserver
    {
    public:
        BlockchainExplorer(INode &node, std::shared_ptr<logging::ILogger> logger);

        BlockchainExplorer(const BlockchainExplorer &) = delete;
        BlockchainExplorer(BlockchainExplorer &&) = delete;

        BlockchainExplorer &operator=(const BlockchainExplorer &) = delete;
        BlockchainExplorer &operator=(BlockchainExplorer &&) = delete;

        virtual ~BlockchainExplorer();

        virtual bool addObserver(IBlockchainObserver *observer) override;
        virtual bool removeObserver(IBlockchainObserver *observer) override;

        virtual bool getBlocks(const std::vector<uint32_t> &blockHeights, std::vector<std::vector<BlockDetails>> &blocks) override;
        virtual bool getBlocks(const std::vector<crypto::Hash> &blockHashes, std::vector<BlockDetails> &blocks) override;
        virtual bool getBlocks(uint64_t timestampBegin, uint64_t timestampEnd, uint32_t blocksNumberLimit, std::vector<BlockDetails> &blocks, uint32_t &blocksNumberWithinTimestamps) override;

        virtual bool getBlockchainTop(BlockDetails &topBlock) override;

        virtual bool getTransactions(const std::vector<crypto::Hash> &transactionHashes, std::vector<TransactionDetails> &transactions) override;
        virtual bool getTransactionsByPaymentId(const crypto::Hash &paymentId, std::vector<TransactionDetails> &transactions) override;
        virtual bool getPoolState(const std::vector<crypto::Hash> &knownPoolTransactionHashes, crypto::Hash knownBlockchainTop, bool &isBlockchainActual, std::vector<TransactionDetails> &newTransactions, std::vector<crypto::Hash> &removedTransactions) override;

        virtual bool isSynchronized() override;

        virtual void init() override;
        virtual void shutdown() override;

        virtual void poolChanged() override;
        virtual void blockchainSynchronized(uint32_t topIndex) override;
        virtual void localBlockchainUpdated(uint32_t index) override;

        typedef WalletAsyncContextCounter AsyncContextCounter;

    private:
        void poolUpdateEndHandler();

        class PoolUpdateGuard
        {
        public:
            PoolUpdateGuard();

            bool beginUpdate();
            bool endUpdate();

        private:
            enum class State
            {
                NONE,
                UPDATING,
                UPDATE_REQUIRED
            };

            std::atomic<State> m_state;
        };

        bool getBlockchainTop(BlockDetails &topBlock, bool checkInitialization);
        bool getBlocks(const std::vector<uint32_t> &blockHeights, std::vector<std::vector<BlockDetails>> &blocks, bool checkInitialization);

        void rebuildIndexes();
        void handleBlockchainUpdatedNotification(const std::vector<std::vector<BlockDetails>> &blocks);

        BlockDetails knownBlockchainTop;
        std::unordered_map<crypto::Hash, TransactionDetails> knownPoolState;

        std::atomic<State> state;
        std::atomic<bool> synchronized;
        std::atomic<uint32_t> observersCounter;
        tools::ObserverManager<IBlockchainObserver> observerManager;

        std::mutex mutex;

        INode &node;
        logging::LoggerRef logger;

        AsyncContextCounter asyncContextCounter;
        PoolUpdateGuard poolUpdateGuard;
    };
}
