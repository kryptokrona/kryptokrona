// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <SimpleWallet/ColouredMsg.h>
#include <SimpleWallet/Types.h>

#include <Wallet/WalletGreen.h>

void printPrivateKeys(CryptoNote::WalletGreen &wallet, bool viewWallet);

void help(bool viewWallet);

void status(CryptoNote::INode &node);

void reset(CryptoNote::INode &node, std::shared_ptr<WalletInfo> &walletInfo);

void blockchainHeight(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet);

void balance(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet,
             bool viewWallet);

void exportKeys(std::shared_ptr<WalletInfo> &walletInfo);
