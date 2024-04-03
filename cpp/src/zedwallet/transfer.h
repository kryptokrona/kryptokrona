// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <memory>

#include <zedwallet/types.h>
#include <config/wallet_config.h>

enum AddressType
{
    NotAnAddress,
    IntegratedAddress,
    StandardAddress
};

enum BalanceInfo
{
    NotEnoughBalance,
    EnoughBalance,
    SetMixinToZero
};

void transfer(std::shared_ptr<WalletInfo> walletInfo, uint32_t height,
              bool sendAll = false, std::string nodeAddress = std::string(),
              uint32_t nodeFee = 0);

void doTransfer(std::string address, uint64_t amount, uint64_t fee,
                std::string extra, std::shared_ptr<WalletInfo> walletInfo,
                uint32_t height, bool integratedAddress,
                uint64_t mixin, std::string nodeAddress, uint32_t nodeFee,
                std::string originalAddress);

void splitTX(cryptonote::WalletGreen &wallet,
             const cryptonote::TransactionParameters splitTXParams,
             uint32_t nodeFee);

void sendTX(std::shared_ptr<WalletInfo> walletInfo,
            cryptonote::TransactionParameters p, uint32_t height,
            bool retried = false, uint32_t nodeFee = 0);

bool confirmTransaction(cryptonote::TransactionParameters t,
                        std::shared_ptr<WalletInfo> walletInfo,
                        bool integratedAddress, uint32_t nodeFee,
                        std::string originalAddress);

bool parseAmount(std::string amountString);

bool parseStandardAddress(std::string address, bool printErrors = false);

bool parseIntegratedAddress(std::string address);

bool parseFee(std::string feeString);

bool handleTransferError(const std::system_error &e, bool retried,
                         uint32_t height);

AddressType parseAddress(std::string address);

std::string getExtraFromPaymentID(std::string paymentID);

Maybe<std::string> getPaymentID(std::string msg);

Maybe<std::string> getExtra();

Maybe<std::pair<AddressType, std::string>> getAddress(std::string msg);

Maybe<uint64_t> getFee();

Maybe<uint64_t> getTransferAmount();

Maybe<std::pair<std::string, std::string>> extractIntegratedAddress(
    std::string integratedAddress);

BalanceInfo doWeHaveEnoughBalance(uint64_t amount, uint64_t fee,
                                  std::shared_ptr<WalletInfo> walletInfo,
                                  uint64_t height, uint32_t nodeFee);
