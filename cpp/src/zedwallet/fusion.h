// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <wallet/wallet_green.h>

bool fusionTX(cryptonote::WalletGreen &wallet,
              cryptonote::TransactionParameters p,
              uint64_t height);

bool optimize(cryptonote::WalletGreen &wallet, uint64_t threshold,
              uint64_t height);

void fullOptimize(cryptonote::WalletGreen &wallet, uint64_t height);

size_t makeFusionTransaction(cryptonote::WalletGreen &wallet,
                             uint64_t threshold, uint64_t height);
