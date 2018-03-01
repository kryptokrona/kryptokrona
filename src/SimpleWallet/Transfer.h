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

void fusionTX(CryptoNote::WalletGreen &wallet, 
              CryptoNote::TransactionParameters p);

void sendMultipleTransactions(CryptoNote::WalletGreen &wallet,
                              std::vector<CryptoNote::TransactionParameters>
                              transfers);

void splitTx(CryptoNote::WalletGreen &wallet,
             CryptoNote::TransactionParameters p);

bool optimize(CryptoNote::WalletGreen &wallet, uint64_t threshold);

void quickOptimize(CryptoNote::WalletGreen &wallet);

void fullOptimize(CryptoNote::WalletGreen &wallet);

bool confirmTransaction(CryptoNote::TransactionParameters t,
                        std::shared_ptr<WalletInfo> walletInfo);

std::string getPaymentID();

std::string getDestinationAddress();

uint64_t getFee();

uint64_t getTransferAmount();

uint16_t getMixin();

size_t makeFusionTransaction(CryptoNote::WalletGreen &wallet, 
                             uint64_t threshold);
