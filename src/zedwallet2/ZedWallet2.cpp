// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#include <tuple>
#include <WalletBackend/WalletBackend.h>
#include <Common/StringTools.h>

int main()
{
    std::string walletName = "test.wallet";
    std::string walletPass = "password";
    std::string daemonHost = "127.0.0.1";
    std::string seed = "biggest yields peeled pawnshop godfather likewise hickory queen exit trying buying island wagtail vitals lucky theatrics dewdrop licks update pivot digit foes ensign estate queen";
    std::string address = "TRTLv2Fyavy8CXG8BPEbNeCHFZ1fuDCYCZ3vW5H5LXN4K2M2MHUpTENip9bbavpHvvPwb4NDkBWrNgURAd5DB38FHXWZyoBh4wW";

    uint64_t seedScanHeight = 822500;

    uint64_t keyScanHeight = 890800;

    Crypto::SecretKey privateSpendKey;
    Crypto::SecretKey privateViewKey;

    Common::podFromHex("bf09c76d79e35ff67c73cd4c7f4c6093f369e2c3a249b6a3d77dca6ad48b790a", privateSpendKey.data);
    Common::podFromHex("8b3b27f3b2f5109e22b86c5bc99ca0a68c8f442b7eff201bc84feaec62239505", privateViewKey.data);

    uint16_t daemonPort = 11898;

    WalletBackend wallet;
    WalletError error;

    std::string selection;

    std::cout << "Selection? (open/create/seed/keys/view): ";

    std::getline(std::cin, selection);

    if (selection == "open")
    {
        std::tie(error, wallet) = WalletBackend::openWallet(walletName, walletPass, daemonHost, daemonPort);
    }
    else if (selection == "create")
    {
        std::tie(error, wallet) = WalletBackend::createWallet(walletName, walletPass, daemonHost, daemonPort);
    }
    else if (selection == "seed")
    {
        std::tie(error, wallet) = WalletBackend::importWalletFromSeed(seed, walletName, walletPass, seedScanHeight, daemonHost, daemonPort);
    }
    else if (selection == "keys")
    {
        std::tie(error, wallet) = WalletBackend::importWalletFromKeys(privateSpendKey, privateViewKey, walletName, walletPass, keyScanHeight, daemonHost, daemonPort);
    }
    else if (selection == "view")
    {
        std::tie(error, wallet) = WalletBackend::importViewWallet(privateSpendKey, address, walletName, walletPass, keyScanHeight, daemonHost, daemonPort);
    }
    else
    {
        std::cout << "Bad input." << std::endl;
        return 1;
    }

    if (error)
    {
        std::cout << "Operation failed, error: " << getErrorMessage(error) << std::endl;
        return 1;
    }

    wallet.m_eventHandler->registerOnSynced([&](int blockHeight)
    {
        std::cout << "Wallet is synced! Top block: " << blockHeight << std::endl;

        Crypto::Hash transactionHash;

        std::tie(error, transactionHash) = wallet.sendTransactionBasic(
            "TRTLv2Fyavy8CXG8BPEbNeCHFZ1fuDCYCZ3vW5H5LXN4K2M2MHUpTENip9bbavpHvvPwb4NDkBWrNgURAd5DB38FHXWZyoBh4wW",
            1000000,
            std::string()
        );

        if (error)
        {
            std::cout << "Failed to send transaction: " << getErrorMessage(error) << std::endl;
        }
        else
        {
            std::cout << "Sent transaction: " << transactionHash << std::endl;
        }
    });
    
    std::string input;

    /* Wait for input */
    std::getline(std::cin, input);

    return 0;
}
