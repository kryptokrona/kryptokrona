// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <zedwallet/types.h>

bool handleCommand(const std::string command,
                   std::shared_ptr<WalletInfo> walletInfo,
                   cryptonote::INode &node);

std::shared_ptr<WalletInfo> handleLaunchCommand(cryptonote::WalletGreen &wallet,
                                                std::string launchCommand,
                                                Config &config);
