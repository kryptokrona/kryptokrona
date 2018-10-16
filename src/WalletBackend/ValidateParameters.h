// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <string>

#include <unordered_map>

#include <vector>

#include <WalletBackend/SubWallets.h>
#include <WalletBackend/WalletErrors.h>

WalletError validateTransaction(
    const std::vector<std::pair<std::string, uint64_t>> destinations,
    const uint64_t mixin,
    const uint64_t fee,
    const std::string paymentID,
    const std::vector<std::string> subWalletsToTakeFrom,
    const std::string changeAddress,
    const std::shared_ptr<SubWallets> subWallets,
    const uint64_t height);

WalletError validateIntegratedAddresses(
    const std::vector<std::pair<std::string, uint64_t>> destinations,
    std::string paymentID);

WalletError validatePaymentID(const std::string paymentID);

WalletError validateMixin(const uint64_t mixin, const uint64_t height);

WalletError validateAmount(
    const std::vector<std::pair<std::string, uint64_t>> destinations,
    const uint64_t fee,
    const std::vector<std::string> subWalletsToTakeFrom,
    const std::shared_ptr<SubWallets> subWallets);

WalletError validateDestinations(
    const std::vector<std::pair<std::string, uint64_t>> destinations);

WalletError validateAddresses(
    std::vector<std::string> addresses,
    const bool integratedAddressesAllowed);

WalletError validateOurAddresses(
    const std::vector<std::string> addresses,
    const std::shared_ptr<SubWallets> subWallets);
