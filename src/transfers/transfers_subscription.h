// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "itransfers_synchronizer.h"
#include "transfers_container.h"
#include "iobservable_impl.h"

#include "logging/logger_ref.h"

namespace cryptonote
{

    class TransfersSubscription : public IObservableImpl<ITransfersObserver, ITransfersSubscription>
    {
    public:
        TransfersSubscription(const cryptonote::Currency &currency, std::shared_ptr<logging::ILogger> logger, const AccountSubscription &sub);

        SynchronizationStart getSyncStart();
        void onBlockchainDetach(uint32_t height);
        void onError(const std::error_code &ec, uint32_t height);
        bool advanceHeight(uint32_t height);
        const AccountKeys &getKeys() const;
        bool addTransaction(const TransactionBlockInfo &blockInfo, const ITransactionReader &tx,
                            const std::vector<TransactionOutputInformationIn> &transfers);

        void deleteUnconfirmedTransaction(const crypto::Hash &transactionHash);
        void markTransactionConfirmed(const TransactionBlockInfo &block, const crypto::Hash &transactionHash, const std::vector<uint32_t> &globalIndices);

        // ITransfersSubscription
        virtual AccountPublicAddress getAddress() override;
        virtual ITransfersContainer &getContainer() override;

    private:
        logging::LoggerRef logger;
        TransfersContainer transfers;
        AccountSubscription subscription;
        std::string m_address;
    };

}
