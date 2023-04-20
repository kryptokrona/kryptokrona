// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The BBSCoin Developers
// Copyright (c) 2018, The Karbo Developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "common/observer_manager.h"
#include "itransfers_synchronizer.h"
#include "iblockchain_synchronizer.h"
#include "type_helpers.h"

#include <unordered_map>
#include <memory>
#include <cstring>

#include "logging/logger_ref.h"

namespace cryptonote
{
    class Currency;
}

namespace cryptonote
{

    class TransfersConsumer;
    class INode;

    class TransfersSyncronizer : public ITransfersSynchronizer, public IBlockchainConsumerObserver
    {
    public:
        TransfersSyncronizer(const cryptonote::Currency &currency, std::shared_ptr<logging::ILogger> logger, IBlockchainSynchronizer &sync, INode &node);
        virtual ~TransfersSyncronizer();

        void initTransactionPool(const std::unordered_set<crypto::Hash> &uncommitedTransactions);

        // ITransfersSynchronizer
        virtual ITransfersSubscription &addSubscription(const AccountSubscription &acc) override;
        virtual bool removeSubscription(const AccountPublicAddress &acc) override;
        virtual void getSubscriptions(std::vector<AccountPublicAddress> &subscriptions) override;
        virtual ITransfersSubscription *getSubscription(const AccountPublicAddress &acc) override;
        virtual std::vector<crypto::Hash> getViewKeyKnownBlocks(const crypto::PublicKey &publicViewKey) override;

        void subscribeConsumerNotifications(const crypto::PublicKey &viewPublicKey, ITransfersSynchronizerObserver *observer);
        void unsubscribeConsumerNotifications(const crypto::PublicKey &viewPublicKey, ITransfersSynchronizerObserver *observer);

        void addPublicKeysSeen(const AccountPublicAddress &acc, const crypto::Hash &transactionHash, const crypto::PublicKey &outputKey);

        // IStreamSerializable
        virtual void save(std::ostream &os) override;
        virtual void load(std::istream &in) override;

    private:
        logging::LoggerRef m_logger;

        // map { view public key -> consumer }
        typedef std::unordered_map<crypto::PublicKey, std::unique_ptr<TransfersConsumer>> ConsumersContainer;
        ConsumersContainer m_consumers;

        typedef tools::ObserverManager<ITransfersSynchronizerObserver> SubscribersNotifier;
        typedef std::unordered_map<crypto::PublicKey, std::unique_ptr<SubscribersNotifier>> SubscribersContainer;
        SubscribersContainer m_subscribers;

        // std::unordered_map<AccountAddress, std::unique_ptr<TransfersConsumer>> m_subscriptions;
        IBlockchainSynchronizer &m_sync;
        INode &m_node;
        const cryptonote::Currency &m_currency;

        virtual void onBlocksAdded(IBlockchainConsumer *consumer, const std::vector<crypto::Hash> &blockHashes) override;
        virtual void onBlockchainDetach(IBlockchainConsumer *consumer, uint32_t blockIndex) override;
        virtual void onTransactionDeleteBegin(IBlockchainConsumer *consumer, crypto::Hash transactionHash) override;
        virtual void onTransactionDeleteEnd(IBlockchainConsumer *consumer, crypto::Hash transactionHash) override;
        virtual void onTransactionUpdated(IBlockchainConsumer *consumer, const crypto::Hash &transactionHash,
                                          const std::vector<ITransfersContainer *> &containers) override;

        bool findViewKeyForConsumer(IBlockchainConsumer *consumer, crypto::PublicKey &viewKey) const;
        SubscribersContainer::const_iterator findSubscriberForConsumer(IBlockchainConsumer *consumer) const;
    };

}
