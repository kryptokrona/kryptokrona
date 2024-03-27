// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <zedwallet/types.h>

std::shared_ptr<WalletInfo> importFromKeys(cryptonote::WalletGreen &wallet,
                                           crypto::SecretKey privateSpendKey,
                                           crypto::SecretKey privateViewKey);

std::shared_ptr<WalletInfo> openWallet(cryptonote::WalletGreen &wallet,
                                       Config &config);

std::shared_ptr<WalletInfo> createViewWallet(cryptonote::WalletGreen &wallet);

std::shared_ptr<WalletInfo> importWallet(cryptonote::WalletGreen &wallet);

std::shared_ptr<WalletInfo> createViewWallet(cryptonote::WalletGreen &wallet);

std::shared_ptr<WalletInfo> mnemonicImportWallet(cryptonote::WalletGreen
                                                     &wallet);

std::shared_ptr<WalletInfo> generateWallet(cryptonote::WalletGreen &wallet);

crypto::SecretKey getPrivateKey(std::string outputMsg);

std::string getNewWalletFileName();

std::string getExistingWalletFileName(Config &config);

std::string getWalletPassword(bool verifyPwd, std::string msg);

bool isValidMnemonic(std::string &mnemonic_phrase,
                     crypto::SecretKey &private_spend_key);

void logIncorrectMnemonicWords(std::vector<std::string> words);

void viewWalletMsg();

void connectingMsg();

void promptSaveKeys(cryptonote::WalletGreen &wallet);
