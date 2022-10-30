// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <Wallet/WalletGreen.h>

bool fusionTX(CryptoNote::WalletGreen &wallet, 
              CryptoNote::TransactionParameters p,
              uint64_t height);

bool optimize(CryptoNote::WalletGreen &wallet, uint64_t threshold,
              uint64_t height);

void fullOptimize(CryptoNote::WalletGreen &wallet, uint64_t height);

size_t makeFusionTransaction(CryptoNote::WalletGreen &wallet, 
                             uint64_t threshold, uint64_t height);
