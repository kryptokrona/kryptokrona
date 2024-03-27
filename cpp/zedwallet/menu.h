// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <zedwallet/types.h>

template <typename T>
std::string parseCommand(const std::vector<T> &printableCommands,
                         const std::vector<T> &availableCommands,
                         std::string prompt,
                         bool backgroundRefresh,
                         std::shared_ptr<WalletInfo> walletInfo);

std::tuple<bool, std::shared_ptr<WalletInfo>>
selectionScreen(Config &config, cryptonote::WalletGreen &wallet,
                cryptonote::INode &node);

bool checkNodeStatus(cryptonote::INode &node);

std::string getAction(Config &config);

void mainLoop(std::shared_ptr<WalletInfo> walletInfo, cryptonote::INode &node);

template <typename T>
void printCommands(const std::vector<T> &commands, size_t offset = 0);
