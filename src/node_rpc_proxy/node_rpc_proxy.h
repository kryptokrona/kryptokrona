// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>

#include "common/observer_manager.h"
#include "logging/logger_ref.h"
#include "inode.h"
#include "rpc/core_rpc_server_commands_definitions.h"

namespace syst
{
    class ContextGroup;
    class Dispatcher;
    class Event;
}

namespace cryptonote
{

    class HttpClient;

    class INodeRpcProxyObserver
    {
    public:
        virtual ~INodeRpcProxyObserver() {}
        virtual void connectionStatusUpdated(bool connected) {}
    };

    class NodeRpcProxy : public cryptonote::INode
    {
    public:
        NodeRpcProxy(const std::string &nodeHost, unsigned short nodePort, unsigned int initTimeout, std::shared_ptr<logging::ILogger> logger);
        NodeRpcProxy(const std::string &nodeHost, unsigned short nodePort, unsigned int initTimeout);
        virtual ~NodeRpcProxy();

        virtual bool addObserver(cryptonote::INodeObserver *observer) override;
        virtual bool removeObserver(cryptonote::INodeObserver *observer) override;

        virtual bool addObserver(cryptonote::INodeRpcProxyObserver *observer);
        virtual bool removeObserver(cryptonote::INodeRpcProxyObserver *observer);

        virtual void init(const Callback &callback) override;
        virtual bool shutdown() override;

        virtual size_t getPeerCount() const override;
        virtual uint32_t getLastLocalBlockHeight() const override;
        virtual uint32_t getLastKnownBlockHeight() const override;
        virtual uint32_t getLocalBlockCount() const override;
        virtual uint32_t getKnownBlockCount() const override;
        virtual uint64_t getNodeHeight() const override;

        virtual void getFeeInfo() override;

        virtual void getBlockHashesByTimestamps(uint64_t timestampBegin, size_t secondsCount, std::vector<crypto::Hash> &blockHashes, const Callback &callback) override;
        virtual void getTransactionHashesByPaymentId(const crypto::Hash &paymentId, std::vector<crypto::Hash> &transactionHashes, const Callback &callback) override;

        virtual BlockHeaderInfo getLastLocalBlockHeaderInfo() const override;

        virtual void relayTransaction(const cryptonote::Transaction &transaction, const Callback &callback) override;
        virtual void getRandomOutsByAmounts(std::vector<uint64_t> &&amounts, uint16_t outsCount, std::vector<RandomOuts> &result, const Callback &callback) override;
        virtual void getTransactionOutsGlobalIndices(const crypto::Hash &transactionHash, std::vector<uint32_t> &outsGlobalIndices, const Callback &callback) override;

        virtual void getGlobalIndexesForRange(
            const uint64_t startHeight,
            const uint64_t endHeight,
            std::unordered_map<crypto::Hash, std::vector<uint64_t>> &indexes,
            const Callback &callback) override;

        virtual void getTransactionsStatus(
            const std::unordered_set<crypto::Hash> transactionHashes,
            std::unordered_set<crypto::Hash> &transactionsInPool,
            std::unordered_set<crypto::Hash> &transactionsInBlock,
            std::unordered_set<crypto::Hash> &transactionsUnknown,
            const Callback &callback) override;

        virtual void queryBlocks(std::vector<crypto::Hash> &&knownBlockIds, uint64_t timestamp, std::vector<BlockShortEntry> &newBlocks, uint32_t &startHeight, const Callback &callback) override;
        virtual void getWalletSyncData(std::vector<crypto::Hash> &&knownBlockIds, uint64_t startHeight, uint64_t startTimestamp, std::vector<wallet_types::WalletBlockInfo> &newBlocks, const Callback &callback) override;
        virtual void getPoolSymmetricDifference(std::vector<crypto::Hash> &&knownPoolTxIds, crypto::Hash knownBlockId, bool &isBcActual,
                                                std::vector<std::unique_ptr<ITransactionReader>> &newTxs, std::vector<crypto::Hash> &deletedTxIds, const Callback &callback) override;
        virtual void getBlocks(const std::vector<uint32_t> &blockHeights, std::vector<std::vector<BlockDetails>> &blocks, const Callback &callback) override;
        virtual void getBlocks(const std::vector<crypto::Hash> &blockHashes, std::vector<BlockDetails> &blocks, const Callback &callback) override;
        virtual void getBlock(const uint32_t blockHeight, BlockDetails &block, const Callback &callback) override;
        virtual void getTransactions(const std::vector<crypto::Hash> &transactionHashes, std::vector<TransactionDetails> &transactions, const Callback &callback) override;
        virtual void isSynchronized(bool &syncStatus, const Callback &callback) override;
        virtual std::string feeAddress() override;
        virtual uint32_t feeAmount() override;

        unsigned int rpcTimeout() const { return m_rpcTimeout; }
        void rpcTimeout(unsigned int val) { m_rpcTimeout = val; }

    private:
        void resetInternalState();
        void workerThread(const Callback &initialized_callback);

        std::vector<crypto::Hash> getKnownTxsVector() const;
        void pullNodeStatusAndScheduleTheNext();
        void updateNodeStatus();
        void updateBlockchainStatus();
        bool updatePoolStatus();
        void updatePeerCount(size_t peerCount);
        void updatePoolState(const std::vector<std::unique_ptr<ITransactionReader>> &addedTxs, const std::vector<crypto::Hash> &deletedTxsIds);

        std::error_code doGetBlockHashesByTimestamps(uint64_t timestampBegin, size_t secondsCount, std::vector<crypto::Hash> &blockHashes);
        std::error_code doRelayTransaction(const cryptonote::Transaction &transaction);
        std::error_code doGetRandomOutsByAmounts(std::vector<uint64_t> &amounts, uint16_t outsCount,
                                                 std::vector<RandomOuts> &result);
        std::error_code doGetTransactionOutsGlobalIndices(const crypto::Hash &transactionHash,
                                                          std::vector<uint32_t> &outsGlobalIndices);

        std::error_code doGetGlobalIndexesForRange(
            const uint64_t startHeight,
            const uint64_t endHeight,
            std::unordered_map<crypto::Hash, std::vector<uint64_t>> &indexes);

        std::error_code doGetTransactionsStatus(
            const std::unordered_set<crypto::Hash> transactionHashes,
            std::unordered_set<crypto::Hash> &transactionsInPool,
            std::unordered_set<crypto::Hash> &transactionsInBlock,
            std::unordered_set<crypto::Hash> &transactionsUnknown);

        std::error_code doQueryBlocksLite(const std::vector<crypto::Hash> &knownBlockIds, uint64_t timestamp,
                                          std::vector<cryptonote::BlockShortEntry> &newBlocks, uint32_t &startHeight);

        std::error_code doGetWalletSyncData(const std::vector<crypto::Hash> &knownBlockIds, uint64_t startHeight, uint64_t startTimestamp, std::vector<wallet_types::WalletBlockInfo> &newBlocks);

        std::error_code doGetPoolSymmetricDifference(std::vector<crypto::Hash> &&knownPoolTxIds, crypto::Hash knownBlockId, bool &isBcActual,
                                                     std::vector<std::unique_ptr<ITransactionReader>> &newTxs, std::vector<crypto::Hash> &deletedTxIds);
        std::error_code doGetBlocksByHeight(const std::vector<uint32_t> &blockHeights, std::vector<std::vector<BlockDetails>> &blocks);
        std::error_code doGetBlocksByHash(const std::vector<crypto::Hash> &blockHashes, std::vector<BlockDetails> &blocks);
        std::error_code doGetBlock(const uint32_t blockHeight, BlockDetails &block);
        std::error_code doGetTransactionHashesByPaymentId(const crypto::Hash &paymentId, std::vector<crypto::Hash> &transactionHashes);
        std::error_code doGetTransactions(const std::vector<crypto::Hash> &transactionHashes, std::vector<TransactionDetails> &transactions);

        void scheduleRequest(std::function<std::error_code()> &&procedure, const Callback &callback);
        template <typename Request, typename Response>
        std::error_code binaryCommand(const std::string &url, const Request &req, Response &res);
        template <typename Request, typename Response>
        std::error_code jsonCommand(const std::string &url, const Request &req, Response &res);
        template <typename Request, typename Response>
        std::error_code jsonRpcCommand(const std::string &method, const Request &req, Response &res);

        enum State
        {
            STATE_NOT_INITIALIZED,
            STATE_INITIALIZING,
            STATE_INITIALIZED
        };

    private:
        logging::LoggerRef m_logger;
        State m_state = STATE_NOT_INITIALIZED;
        mutable std::mutex m_mutex;
        std::condition_variable m_cv_initialized;
        std::thread m_workerThread;
        syst::Dispatcher *m_dispatcher = nullptr;
        syst::ContextGroup *m_context_group = nullptr;
        tools::ObserverManager<cryptonote::INodeObserver> m_observerManager;
        tools::ObserverManager<cryptonote::INodeRpcProxyObserver> m_rpcProxyObserverManager;

        const std::string m_nodeHost;
        const unsigned short m_nodePort;
        unsigned int m_rpcTimeout;
        unsigned int m_initTimeout;
        HttpClient *m_httpClient = nullptr;
        syst::Event *m_httpEvent = nullptr;

        uint64_t m_pullInterval;

        // Internal state
        bool m_stop = false;
        std::atomic<size_t> m_peerCount;
        std::atomic<uint32_t> m_networkHeight;
        std::atomic<uint64_t> m_nodeHeight;

        BlockHeaderInfo lastLocalBlockHeaderInfo;
        // protect it with mutex if decided to add worker threads
        std::unordered_set<crypto::Hash> m_knownTxs;

        bool m_connected;
        std::string m_fee_address;
        uint32_t m_fee_amount = 0;
    };
}
