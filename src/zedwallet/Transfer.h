// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <memory>

#include <zedwallet/Types.h>

enum AddressType {NotAnAddress, IntegratedAddress, StandardAddress};

void transfer(std::shared_ptr<WalletInfo> walletInfo, uint32_t height);

void doTransfer(std::string address, uint64_t amount, uint64_t fee,
                std::string extra, std::shared_ptr<WalletInfo> walletInfo,
                uint32_t height, bool integratedAddress);

void sendMultipleTransactions(CryptoNote::WalletGreen &wallet,
                              std::vector<CryptoNote::TransactionParameters>
                              transfers);

void splitTx(CryptoNote::WalletGreen &wallet,
             CryptoNote::TransactionParameters p);

bool confirmTransaction(CryptoNote::TransactionParameters t,
                        std::shared_ptr<WalletInfo> walletInfo,
                        bool integratedAddress);

bool parseAmount(std::string amountString);

bool parseStandardAddress(std::string address, bool printErrors = false);

bool parseIntegratedAddress(std::string address);

bool parseFee(std::string feeString);

AddressType parseAddress(std::string address);

std::string getExtraFromPaymentID(std::string paymentID);

Maybe<std::string> getPaymentID(std::string msg);

Maybe<std::string> getExtra();

Maybe<std::pair<AddressType, std::string>> getAddress(std::string msg);

Maybe<uint64_t> getFee();

Maybe<uint64_t> getTransferAmount();

Maybe<std::pair<std::string, std::string>> extractIntegratedAddress(
    std::string integratedAddress);
