// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <string>

#include <vector>

#include <wallet_backend/wallet_backend.h>

std::string yellowANSIMsg(std::string msg);

std::string getPrompt(std::shared_ptr<WalletBackend> walletBackend);

std::string getAddress(
    const std::string msg,
    const bool integratedAddressesAllowed,
    const bool cancelAllowed);

std::string getPaymentID(
    const std::string msg,
    const bool cancelAllowed);

std::tuple<bool, uint64_t> getAmountToAtomic(
    const std::string msg,
    const bool cancelAllowed);

template <typename T>
std::string getInput(
    const std::vector<T> &availableCommands,
    const std::string prompt);

std::tuple<std::string, uint16_t> getDaemonAddress();

std::string getHash(
    const std::string msg,
    const bool cancelAllowed);
