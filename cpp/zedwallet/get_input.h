// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <string>

#include <vector>

#include <zedwallet/commands.h>
#include <zedwallet/types.h>

std::string yellowANSIMsg(std::string msg);

std::string getPrompt(std::shared_ptr<WalletInfo> walletInfo);

template <typename T>
std::string getInputAndWorkInBackground(const std::vector<T>
                                            &availableCommands,
                                        std::string prompt,
                                        bool backgroundRefresh,
                                        std::shared_ptr<WalletInfo>
                                            walletInfo);

template <typename T>
std::string getInput(const std::vector<T> &availableCommands,
                     std::string prompt);
