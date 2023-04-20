// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <zedwallet/types.h>

void syncWallet(cryptonote::INode &node,
                std::shared_ptr<WalletInfo> walletInfo);

void checkForNewTransactions(std::shared_ptr<WalletInfo> walletInfo);
