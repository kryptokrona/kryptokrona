// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <zedwallet/types.h>

std::shared_ptr<WalletInfo> importFromKeys(cryptonote::WalletGreen &wallet,
                                           Crypto::SecretKey privateSpendKey,
                                           Crypto::SecretKey privateViewKey);

std::shared_ptr<WalletInfo> openWallet(cryptonote::WalletGreen &wallet,
                                       Config &config);

std::shared_ptr<WalletInfo> createViewWallet(cryptonote::WalletGreen &wallet);

std::shared_ptr<WalletInfo> importWallet(cryptonote::WalletGreen &wallet);

std::shared_ptr<WalletInfo> createViewWallet(cryptonote::WalletGreen &wallet);

std::shared_ptr<WalletInfo> mnemonicImportWallet(cryptonote::WalletGreen
                                                     &wallet);

std::shared_ptr<WalletInfo> generateWallet(cryptonote::WalletGreen &wallet);

Crypto::SecretKey getPrivateKey(std::string outputMsg);

std::string getNewWalletFileName();

std::string getExistingWalletFileName(Config &config);

std::string getWalletPassword(bool verifyPwd, std::string msg);

bool isValidMnemonic(std::string &mnemonic_phrase,
                     Crypto::SecretKey &private_spend_key);

void logIncorrectMnemonicWords(std::vector<std::string> words);

void viewWalletMsg();

void connectingMsg();

void promptSaveKeys(cryptonote::WalletGreen &wallet);
