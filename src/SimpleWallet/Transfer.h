#include <string>

#include "CryptoNoteConfig.h"
#include "IWallet.h"

#include <Common/StringTools.h>

#include <CryptoNoteCore/TransactionExtra.h>

#include <SimpleWallet/Tools.h>

#include <Wallet/WalletGreen.h>

#include <boost/algorithm/string.hpp>

struct WalletInfo {
    WalletInfo(std::string walletFileName, 
               std::string walletPass, 
               std::string walletAddress,
               CryptoNote::WalletGreen &wallet) : 
               walletFileName(walletFileName), 
               walletPass(walletPass), 
               walletAddress(walletAddress),
               wallet(wallet) {}

    std::string walletFileName;
    std::string walletPass;
    std::string walletAddress;
    CryptoNote::WalletGreen &wallet;
};

void transfer(std::shared_ptr<WalletInfo> walletInfo);

void transfer(std::shared_ptr<WalletInfo> walletInfo,
              std::vector<std::string> args);

void doTransfer(uint16_t mixin, std::string address, uint64_t amount,
                uint64_t fee, std::string extra,
                std::shared_ptr<WalletInfo> walletInfo);

void fusionTX(CryptoNote::WalletGreen &wallet, 
              CryptoNote::TransactionParameters p);

void sendMultipleTransactions(CryptoNote::WalletGreen &wallet,
                              std::vector<CryptoNote::TransactionParameters>
                              transfers);

void splitTx(CryptoNote::WalletGreen &wallet,
             CryptoNote::TransactionParameters p);

void quickOptimize(CryptoNote::WalletGreen &wallet);

void fullOptimize(CryptoNote::WalletGreen &wallet);

bool confirmTransaction(CryptoNote::TransactionParameters t,
                        std::shared_ptr<WalletInfo> walletInfo);

bool optimize(CryptoNote::WalletGreen &wallet, uint64_t threshold);

bool parseAmount(std::string amountString);

bool parseMixin(std::string mixinString);

bool parseAddress(std::string address);

bool parseFee(std::string feeString);

std::string getPaymentID();

std::string getDestinationAddress();

uint64_t getFee();

uint64_t getTransferAmount();

uint16_t getMixin();

size_t makeFusionTransaction(CryptoNote::WalletGreen &wallet, 
                             uint64_t threshold);
