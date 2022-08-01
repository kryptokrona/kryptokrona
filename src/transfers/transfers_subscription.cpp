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

#include "iwallet.h"
#include "transfers_subscription.h"
#include "cryptonote_core/cryptonote_basic_impl.h"

using namespace crypto;
using namespace logging;

namespace cryptonote
{
    TransfersSubscription::TransfersSubscription(const CryptoNote::Currency& currency, std::shared_ptr<Logging::ILogger> logger, const AccountSubscription& sub)
      : subscription(sub), logger(logger, "TransfersSubscription"), transfers(currency, logger, sub.transactionSpendableAge),
        m_address(currency.accountAddressAsString(sub.keys.address)) {
    }

    SynchronizationStart TransfersSubscription::getSyncStart() {
      return subscription.syncStart;
    }

    void TransfersSubscription::onBlockchainDetach(uint32_t height) {
      std::vector<Hash> deletedTransactions = transfers.detach(height);
      for (auto& hash : deletedTransactions) {
        logger(TRACE) << "Transaction deleted from wallet " << m_address << ", hash " << hash;
        m_observerManager.notify(&ITransfersObserver::onTransactionDeleted, this, hash);
      }
    }

    void TransfersSubscription::onError(const std::error_code& ec, uint32_t height) {
      if (height != WALLET_UNCONFIRMED_TRANSACTION_HEIGHT) {
      transfers.detach(height);
      }
      m_observerManager.notify(&ITransfersObserver::onError, this, height, ec);
    }

    bool TransfersSubscription::advanceHeight(uint32_t height) {
      return transfers.advanceHeight(height);
    }

    const AccountKeys& TransfersSubscription::getKeys() const {
      return subscription.keys;
    }

    bool TransfersSubscription::addTransaction(const TransactionBlockInfo& blockInfo, const ITransactionReader& tx,
                                               const std::vector<TransactionOutputInformationIn>& transfersList) {
      bool added = transfers.addTransaction(blockInfo, tx, transfersList);
      if (added) {
        logger(TRACE) << "Transaction updates balance of wallet " << m_address << ", hash " << tx.getTransactionHash();
        m_observerManager.notify(&ITransfersObserver::onTransactionUpdated, this, tx.getTransactionHash());
      }

      return added;
    }

    AccountPublicAddress TransfersSubscription::getAddress() {
      return subscription.keys.address;
    }

    ITransfersContainer& TransfersSubscription::getContainer() {
      return transfers;
    }

    void TransfersSubscription::deleteUnconfirmedTransaction(const Hash& transactionHash) {
      if (transfers.deleteUnconfirmedTransaction(transactionHash)) {
        logger(TRACE) << "Transaction deleted from wallet " << m_address << ", hash " << transactionHash;
        m_observerManager.notify(&ITransfersObserver::onTransactionDeleted, this, transactionHash);
      }
    }

    void TransfersSubscription::markTransactionConfirmed(const TransactionBlockInfo& block, const Hash& transactionHash,
                                                         const std::vector<uint32_t>& globalIndices) {
      transfers.markTransactionConfirmed(block, transactionHash, globalIndices);
      m_observerManager.notify(&ITransfersObserver::onTransactionUpdated, this, transactionHash);
    }
}
