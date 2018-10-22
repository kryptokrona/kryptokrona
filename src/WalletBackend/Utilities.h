// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <CryptoNote.h>

#include <string>

#include <vector>

#include <WalletBackend/WalletErrors.h>

namespace Utilities
{
    std::vector<Crypto::PublicKey> addressesToViewKeys(const std::vector<std::string> addresses);

    std::vector<Crypto::PublicKey> addressesToSpendKeys(const std::vector<std::string> addresses);

    uint64_t getTransactionSum(const std::vector<std::pair<std::string, uint64_t>> destinations);

    std::tuple<std::string, std::string> extractIntegratedAddressData(const std::string address);

    std::tuple<Crypto::PublicKey, Crypto::PublicKey> addressToKeys(const std::string address);

    std::string privateKeysToAddress(
        const Crypto::SecretKey privateSpendKey,
        const Crypto::SecretKey privateViewKey);

    std::string publicKeysToAddress(
        const Crypto::PublicKey publicSpendKey,
        const Crypto::PublicKey publicViewKey);

    uint64_t getUpperBound(const uint64_t val, const uint64_t nearestMultiple);
    uint64_t getLowerBound(const uint64_t val, const uint64_t nearestMultiple);
}
