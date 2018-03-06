#include <iostream>

#include "version.h"
#include "CryptoNoteCore/Currency.h"
#include "CryptoNoteCore/Account.h"
#include "Common/StringTools.h"
#include "NodeRpcProxy/NodeRpcProxy.h"
#include "Wallet/WalletGreen.h"
#include "PasswordContainer.h"
#include "Mnemonics/electrum-words.cpp"

#include <System/Dispatcher.h>
#include <Logging/LoggerManager.h>

#include <boost/algorithm/string.hpp>

enum Action {Open, Generate, Import, SeedImport};

Action getAction();

void generateWallet(CryptoNote::WalletGreen &wallet);
void importWallet(CryptoNote::WalletGreen &wallet);
void mnemonicImportWallet(CryptoNote::WalletGreen &wallet);
void importFromKeys(CryptoNote::WalletGreen &wallet, 
                    Crypto::SecretKey privateSpendKey,
                    Crypto::SecretKey privateViewKey);
void logIncorrectMnemonicWords(std::vector<std::string> words);

bool openWallet(CryptoNote::WalletGreen &wallet);
bool isValidMnemonic(std::string &mnemonic_phrase, Crypto::SecretKey &private_spend_key);

std::string getNewWalletFileName();
std::string getExistingWalletFileName();
std::string getWalletPassword(bool);

Crypto::SecretKey getPrivateKey(std::string outputMsg);
