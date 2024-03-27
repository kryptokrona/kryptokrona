// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <zedwallet/types.h>

#include <wallet/wallet_green.h>

bool handleCommand(const std::string command,
                   std::shared_ptr<WalletInfo> walletInfo,
                   cryptonote::INode &node);

void changePassword(std::shared_ptr<WalletInfo> walletInfo);

void printPrivateKeys(cryptonote::WalletGreen &wallet, bool viewWallet);

void reset(cryptonote::INode &node, std::shared_ptr<WalletInfo> walletInfo);

void status(cryptonote::INode &node, cryptonote::WalletGreen &wallet);

void printHeights(uint32_t localHeight, uint32_t remoteHeight,
                  uint32_t walletHeight);

void printSyncStatus(uint32_t localHeight, uint32_t remoteHeight,
                     uint32_t walletHeight);

void printSyncSummary(uint32_t localHeight, uint32_t remoteHeight,
                      uint32_t walletHeight);

void printPeerCount(size_t peerCount);

void printHashrate(uint64_t difficulty);

void balance(cryptonote::INode &node, cryptonote::WalletGreen &wallet,
             bool viewWallet);

void exportKeys(std::shared_ptr<WalletInfo> walletInfo);

void saveCSV(cryptonote::WalletGreen &wallet, cryptonote::INode &node);

void save(cryptonote::WalletGreen &wallet);

void listTransfers(bool incoming, bool outgoing,
                   cryptonote::WalletGreen &wallet, cryptonote::INode &node);

void printOutgoingTransfer(cryptonote::WalletTransaction t,
                           cryptonote::INode &node);

void printIncomingTransfer(cryptonote::WalletTransaction t,
                           cryptonote::INode &node);

void createIntegratedAddress();

void help(std::shared_ptr<WalletInfo> wallet);

void advanced(std::shared_ptr<WalletInfo> wallet);
