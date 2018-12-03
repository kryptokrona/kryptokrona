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

/* SubWallet */
void to_json(json &j, const SubWallet &s);
void from_json(const json &j, SubWallet &s);

/* WalletBackend */
void to_json(json &j, const WalletBackend &w);
void from_json(const json &j, WalletBackend &w);

/* Crypto::SecretKey */
void to_json(json &j, const Crypto::SecretKey &s);

/* Crypto::PublicKey */
void to_json(json &j, const Crypto::PublicKey &s);

/* Crypto::Hash */
void to_json(json &j, const Crypto::Hash &s);

/* Crypto::KeyImage */
void to_json(json &j, const Crypto::KeyImage &s);

/* CryptoNote::WalletTransaction */
void to_json(json &j, const CryptoNote::WalletTransaction &t);
void from_json(const json &j, CryptoNote::WalletTransaction &t);

/* Crypto::Hash */
void to_json(json &j, const Crypto::Hash &h);

/* WalletSynchronizer */
void to_json(json &j, const WalletSynchronizer &w);
void from_json(const json &j, WalletSynchronizer &w);

/* SynchronizationStatus */
void to_json(json &j, const SynchronizationStatus &s);
void from_json(const json &j, SynchronizationStatus &s);

/* Transfer */
void to_json(json &j, const Transfer &t);
void from_json(const json &j, Transfer &t);

/* TransactionInput */
void to_json(json &j, const WalletTypes::TransactionInput &t);
void from_json(const json &j, WalletTypes::TransactionInput &t);

/* WalletTypes::Transaction */
void to_json(json &j, const WalletTypes::Transaction &t);
void from_json(const json &j, WalletTypes::Transaction &t);

std::vector<Transfer> transfersToVector(std::unordered_map<Crypto::PublicKey, int64_t> transfers);

std::unordered_map<Crypto::PublicKey, int64_t> vectorToTransfers(std::vector<Transfer> vector);

std::vector<SubWallet> subWalletsToVector(std::unordered_map<Crypto::PublicKey, SubWallet> subWallets);

std::unordered_map<Crypto::PublicKey, SubWallet> vectorToSubWallets(std::vector<SubWallet> vector);
