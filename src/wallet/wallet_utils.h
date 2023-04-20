// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <string>

#include "iwallet.h"
#include "cryptonote_core/currency.h"
#include "wallet/wallet_green.h"

namespace cryptonote
{

    uint64_t getDefaultMixinByHeight(const uint64_t height);
    void throwIfKeysMismatch(const crypto::SecretKey &secretKey, const crypto::PublicKey &expectedPublicKey, const std::string &message = "");
    bool validateAddress(const std::string &address, const cryptonote::Currency &currency);

    std::ostream &operator<<(std::ostream &os, cryptonote::WalletTransactionState state);
    std::ostream &operator<<(std::ostream &os, cryptonote::WalletTransferType type);
    std::ostream &operator<<(std::ostream &os, cryptonote::WalletGreen::WalletState state);
    std::ostream &operator<<(std::ostream &os, cryptonote::WalletGreen::WalletTrackingMode mode);

    class TransferListFormatter
    {
    public:
        explicit TransferListFormatter(const cryptonote::Currency &currency, const WalletGreen::TransfersRange &range);

        void print(std::ostream &os) const;

        friend std::ostream &operator<<(std::ostream &os, const TransferListFormatter &formatter);

    private:
        const cryptonote::Currency &m_currency;
        const WalletGreen::TransfersRange &m_range;
    };

    class WalletOrderListFormatter
    {
    public:
        explicit WalletOrderListFormatter(const cryptonote::Currency &currency, const std::vector<cryptonote::WalletOrder> &walletOrderList);

        void print(std::ostream &os) const;

        friend std::ostream &operator<<(std::ostream &os, const WalletOrderListFormatter &formatter);

    private:
        const cryptonote::Currency &m_currency;
        const std::vector<cryptonote::WalletOrder> &m_walletOrderList;
    };

}
