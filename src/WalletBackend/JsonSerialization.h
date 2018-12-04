// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <Common/StringTools.h>

#include "CryptoTypes.h"

#include "IWallet.h"

#include "json.hpp"

#include <WalletBackend/SubWallet.h>
#include <WalletBackend/WalletBackend.h>

using nlohmann::json;

/* Tmp struct just used in serialization (See cpp for justification) */
struct Transfer
{
    Crypto::PublicKey publicKey;
    int64_t amount;
};

/* As above */
struct TxPrivateKey
{
    Crypto::Hash txHash;
    Crypto::SecretKey txPrivateKey;
};

/* SubWallet */
void to_json(json &j, const SubWallet &s);
void from_json(const json &j, SubWallet &s);

/* WalletBackend */
void to_json(json &j, const WalletBackend &w);
void from_json(const json &j, WalletBackend &w);

/* CryptoNote::WalletTransaction */
void to_json(json &j, const CryptoNote::WalletTransaction &t);
void from_json(const json &j, CryptoNote::WalletTransaction &t);

/* WalletSynchronizer */
void to_json(json &j, const WalletSynchronizer &w);
void from_json(const json &j, WalletSynchronizer &w);

/* SynchronizationStatus */
void to_json(json &j, const SynchronizationStatus &s);
void from_json(const json &j, SynchronizationStatus &s);

/* Transfer */
void to_json(json &j, const Transfer &t);
void from_json(const json &j, Transfer &t);

void to_json(json &j, const TxPrivateKey &t);
void from_json(const json &j, TxPrivateKey &t);

namespace WalletTypes
{
    /* TransactionInput */
    void to_json(json &j, const WalletTypes::TransactionInput &t);
    void from_json(const json &j, WalletTypes::TransactionInput &t);

    /* WalletTypes::Transaction */
    void to_json(json &j, const WalletTypes::Transaction &t);
    void from_json(const json &j, WalletTypes::Transaction &t);
}

std::vector<Transfer> transfersToVector(
    const std::unordered_map<Crypto::PublicKey, int64_t> transfers);

std::unordered_map<Crypto::PublicKey, int64_t> vectorToTransfers(
    const std::vector<Transfer> vector);

std::vector<SubWallet> subWalletsToVector(
    const std::unordered_map<Crypto::PublicKey, SubWallet> subWallets);

std::unordered_map<Crypto::PublicKey, SubWallet> vectorToSubWallets(
    const std::vector<SubWallet> vector);

std::vector<TxPrivateKey> txPrivateKeysToVector(
    const std::unordered_map<Crypto::Hash, Crypto::SecretKey> txPrivateKeys);

std::unordered_map<Crypto::Hash, Crypto::SecretKey> vectorToTxPrivateKeys(
    const std::vector<TxPrivateKey> vector);
