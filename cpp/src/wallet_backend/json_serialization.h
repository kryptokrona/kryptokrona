// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <common/string_tools.h>

#include "crypto_types.h"

#include "iwallet.h"

#include "json.hpp"

#include <sub_wallets/sub_wallet.h>

#include <wallet_backend/wallet_backend.h>

using nlohmann::json;

/* Tmp struct just used in serialization (See cpp for justification) */
struct Transfer
{
    crypto::PublicKey publicKey;
    int64_t amount;
};

/* As above */
struct TxPrivateKey
{
    crypto::Hash txHash;
    crypto::SecretKey txPrivateKey;
};

/* Transfer */
void to_json(json &j, const Transfer &t);
void from_json(const json &j, Transfer &t);

void to_json(json &j, const TxPrivateKey &t);
void from_json(const json &j, TxPrivateKey &t);

namespace wallet_types
{
    /* wallet_types::Transaction */
    void to_json(json &j, const wallet_types::Transaction &t);
    void from_json(const json &j, wallet_types::Transaction &t);
}

std::vector<Transfer> transfersToVector(
    const std::unordered_map<crypto::PublicKey, int64_t> transfers);

std::unordered_map<crypto::PublicKey, int64_t> vectorToTransfers(
    const std::vector<Transfer> vector);

std::vector<TxPrivateKey> txPrivateKeysToVector(
    const std::unordered_map<crypto::Hash, crypto::SecretKey> txPrivateKeys);

std::unordered_map<crypto::Hash, crypto::SecretKey> vectorToTxPrivateKeys(
    const std::vector<TxPrivateKey> vector);
