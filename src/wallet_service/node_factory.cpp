// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "node_factory.h"

#include "node_rpc_proxy/node_rpc_proxy.h"
#include <memory>
#include <future>

namespace payment_service
{

    class NodeRpcStub : public cryptonote::INode
    {
    public:
        virtual ~NodeRpcStub() {}
        virtual bool addObserver(cryptonote::INodeObserver *observer) override { return true; }
        virtual bool removeObserver(cryptonote::INodeObserver *observer) override { return true; }

        virtual void init(const Callback &callback) override {}
        virtual bool shutdown() override { return true; }

        virtual size_t getPeerCount() const override { return 0; }
        virtual uint32_t getLastLocalBlockHeight() const override { return 0; }
        virtual uint32_t getLastKnownBlockHeight() const override { return 0; }
        virtual uint32_t getLocalBlockCount() const override { return 0; }
        virtual uint32_t getKnownBlockCount() const override { return 0; }
        virtual uint64_t getNodeHeight() const override { return 0; }

        virtual void getFeeInfo() override {}

        virtual void getBlockHashesByTimestamps(uint64_t timestampBegin, size_t secondsCount, std::vector<crypto::Hash> &blockHashes, const Callback &callback) override
        {
            callback(std::error_code());
        }

        virtual void getTransactionHashesByPaymentId(const crypto::Hash &paymentId, std::vector<crypto::Hash> &transactionHashes, const Callback &callback) override
        {
            callback(std::error_code());
        }

        virtual cryptonote::BlockHeaderInfo getLastLocalBlockHeaderInfo() const override { return cryptonote::BlockHeaderInfo(); }

        virtual void relayTransaction(const cryptonote::Transaction &transaction, const Callback &callback) override { callback(std::error_code()); }
        virtual void getRandomOutsByAmounts(std::vector<uint64_t> &&amounts, uint16_t outsCount,
                                            std::vector<cryptonote::RandomOuts> &result, const Callback &callback) override
        {
        }

        virtual void getTransactionOutsGlobalIndices(const crypto::Hash &transactionHash, std::vector<uint32_t> &outsGlobalIndices, const Callback &callback) override {}

        virtual void getGlobalIndexesForRange(
            const uint64_t startHeight,
            const uint64_t endHeight,
            std::unordered_map<crypto::Hash, std::vector<uint64_t>> &indexes,
            const Callback &callback) override
        {
            callback(std::error_code());
        };

        virtual void getTransactionsStatus(
            const std::unordered_set<crypto::Hash> transactionHashes,
            std::unordered_set<crypto::Hash> &transactionsInPool,
            std::unordered_set<crypto::Hash> &transactionsInBlock,
            std::unordered_set<crypto::Hash> &transactionsUnknown,
            const Callback &callback) override
        {
            callback(std::error_code());
        };

        virtual void queryBlocks(std::vector<crypto::Hash> &&knownBlockIds, uint64_t timestamp, std::vector<cryptonote::BlockShortEntry> &newBlocks,
                                 uint32_t &startHeight, const Callback &callback) override
        {
            startHeight = 0;
            callback(std::error_code());
        };

        virtual void getWalletSyncData(std::vector<crypto::Hash> &&knownblockIds, uint64_t startHeight, uint64_t startTimestamp, std::vector<wallet_types::WalletBlockInfo> &newBlocks,
                                       const Callback &callback) override
        {
            callback(std::error_code());
        };

        virtual void getPoolSymmetricDifference(std::vector<crypto::Hash> &&knownPoolTxIds, crypto::Hash knownBlockId, bool &isBcActual,
                                                std::vector<std::unique_ptr<cryptonote::ITransactionReader>> &newTxs, std::vector<crypto::Hash> &deletedTxIds, const Callback &callback) override
        {
            isBcActual = true;
            callback(std::error_code());
        }

        virtual void getBlocks(const std::vector<uint32_t> &blockHeights, std::vector<std::vector<cryptonote::BlockDetails>> &blocks,
                               const Callback &callback) override {}

        virtual void getBlocks(const std::vector<crypto::Hash> &blockHashes, std::vector<cryptonote::BlockDetails> &blocks,
                               const Callback &callback) override {}

        virtual void getBlock(const uint32_t blockHeight, cryptonote::BlockDetails &block,
                              const Callback &callback) override {}

        virtual void getTransactions(const std::vector<crypto::Hash> &transactionHashes, std::vector<cryptonote::TransactionDetails> &transactions,
                                     const Callback &callback) override {}

        virtual void isSynchronized(bool &syncStatus, const Callback &callback) override {}
        virtual std::string feeAddress() override { return std::string(); }
        virtual uint32_t feeAmount() override { return 0; }
    };

    class NodeInitObserver
    {
    public:
        NodeInitObserver()
        {
            initFuture = initPromise.get_future();
        }

        void initCompleted(std::error_code result)
        {
            initPromise.set_value(result);
        }

        void waitForInitEnd()
        {
            std::error_code ec = initFuture.get();
            if (ec)
            {
                throw std::system_error(ec);
            }
            return;
        }

    private:
        std::promise<std::error_code> initPromise;
        std::future<std::error_code> initFuture;
    };

    NodeFactory::NodeFactory()
    {
    }

    NodeFactory::~NodeFactory()
    {
    }

    cryptonote::INode *NodeFactory::createNode(const std::string &daemonAddress, uint16_t daemonPort, uint16_t initTimeout, std::shared_ptr<logging::ILogger> logger)
    {
        std::unique_ptr<cryptonote::INode> node(new cryptonote::NodeRpcProxy(daemonAddress, daemonPort, initTimeout, logger));

        NodeInitObserver initObserver;
        node->init(std::bind(&NodeInitObserver::initCompleted, &initObserver, std::placeholders::_1));
        initObserver.waitForInitEnd();

        return node.release();
    }

    cryptonote::INode *NodeFactory::createNodeStub()
    {
        return new NodeRpcStub();
    }

}
