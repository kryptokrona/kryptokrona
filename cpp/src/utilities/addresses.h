// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cryptonote.h>

#include <string>

#include <vector>

namespace utilities
{
    std::vector<crypto::PublicKey> addressesToSpendKeys(const std::vector<std::string> addresses);

    std::tuple<crypto::PublicKey, crypto::PublicKey> addressToKeys(const std::string address);

    std::tuple<std::string, std::string> extractIntegratedAddressData(const std::string address);

    std::string publicKeysToAddress(
        const crypto::PublicKey publicSpendKey,
        const crypto::PublicKey publicViewKey);

    std::string privateKeysToAddress(
        const crypto::SecretKey privateSpendKey,
        const crypto::SecretKey privateViewKey);
}
