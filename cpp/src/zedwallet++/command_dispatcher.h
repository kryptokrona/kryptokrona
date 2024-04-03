// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <wallet_backend/wallet_backend.h>

#include <zedwallet++/parse_arguments.h>

bool handleCommand(
    const std::string command,
    const std::shared_ptr<WalletBackend> walletBackend,
    const std::shared_ptr<std::mutex> mutex);

std::shared_ptr<WalletBackend> handleLaunchCommand(
    const std::string launchCommand,
    const Config &config);
