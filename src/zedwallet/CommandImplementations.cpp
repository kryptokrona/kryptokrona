// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

/////////////////////////////////////////////
#include <zedwallet/CommandImplementations.h>
/////////////////////////////////////////////

#include <atomic>

#include <Common/StringTools.h>

#include <CryptoNoteCore/Account.h>
#include <CryptoNoteCore/TransactionExtra.h>

#ifndef MSVC
#include <fstream>
#endif

#include <Mnemonics/electrum-words.h>

#include <zedwallet/ColouredMsg.h>
#include <zedwallet/Open.h>
#include <zedwallet/Fusion.h>
#include <zedwallet/Sync.h>
#include <zedwallet/Tools.h>
#include <zedwallet/Transfer.h>
#include <zedwallet/Types.h>
#include <zedwallet/WalletConfig.h>

void changePassword(std::shared_ptr<WalletInfo> &walletInfo)
{
    /* Check the user knows the current password */
    confirmPassword(walletInfo->walletPass, "Confirm your current password: ");

    /* Get a new password for the wallet */
    const std::string newPassword
        = getWalletPassword(true, "Enter your new password: ");

    /* Change the wallet password */
    walletInfo->wallet.changePassword(walletInfo->walletPass, newPassword);

    /* Change the stored wallet metadata */
    walletInfo->walletPass = newPassword;

    /* Make sure we save with the new password */
    walletInfo->wallet.save();

    std::cout << SuccessMsg("Your password has been changed!") << std::endl;
}

void exportKeys(std::shared_ptr<WalletInfo> &walletInfo)
{
    confirmPassword(walletInfo->walletPass);
    printPrivateKeys(walletInfo->wallet, walletInfo->viewWallet);
}

void printPrivateKeys(CryptoNote::WalletGreen &wallet, bool viewWallet)
{
    const Crypto::SecretKey privateViewKey = wallet.getViewKey().secretKey;

    if (viewWallet)
    {
        std::cout << SuccessMsg("Private view key:")
                  << std::endl
                  << SuccessMsg(Common::podToHex(privateViewKey))
                  << std::endl;
        return;
    }

    Crypto::SecretKey privateSpendKey = wallet.getAddressSpendKey(0).secretKey;

    Crypto::SecretKey derivedPrivateViewKey;

    CryptoNote::AccountBase::generateViewFromSpend(privateSpendKey,
                                                   derivedPrivateViewKey);

    const bool deterministicPrivateKeys
             = derivedPrivateViewKey == privateViewKey;

    std::cout << SuccessMsg("Private spend key:")
              << std::endl
              << SuccessMsg(Common::podToHex(privateSpendKey))
              << std::endl
              << std::endl
              << SuccessMsg("Private view key:")
              << std::endl
              << SuccessMsg(Common::podToHex(privateViewKey))
              << std::endl;

    if (deterministicPrivateKeys)
    {
        std::string mnemonicSeed;

        crypto::ElectrumWords::bytes_to_words(privateSpendKey, 
                                              mnemonicSeed,
                                              "English");

        std::cout << std::endl
                  << SuccessMsg("Mnemonic seed:")
                  << std::endl
                  << SuccessMsg(mnemonicSeed)
                  << std::endl;
    }
}

void status(CryptoNote::INode &node)
{
    std::atomic_bool completed(false);

    std::string status;

    std::thread getStatus([&node, &completed, &status]
    {
        status = node.getInfo();
        completed.store(true);
    });

    const auto startTime = std::chrono::system_clock::now();

    while (!completed.load())
    {
        const auto currentTime = std::chrono::system_clock::now();

        if ((currentTime - startTime) > std::chrono::seconds(5))
        {
            std::cout << WarningMsg("Unable to get daemon status - has it ")
                      << WarningMsg("crashed/frozen?")
                      << std::endl;

            /* Detach the thread so it doesn't call terminate() when we exit
               scope */
            getStatus.detach();

            /* Wait a moment for detach to complete */
            std::this_thread::sleep_for(std::chrono::seconds(1));

            return;
        }
    }

    /* Wait for the thread to clean up so we don't call terminate() on exit */
    getStatus.join();

    if (status == "Problem retrieving information from RPC server.")
    {
        std::cout << WarningMsg("Unable to get daemon status - has it "
                                "crashed/frozen?")
                  << std::endl;
    }
    else
    {
        std::cout << InformationMsg(status) << std::endl;
    }
}

void balance(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet,
             bool viewWallet)
{
    const uint64_t unconfirmedBalance = wallet.getPendingBalance();
    const uint64_t confirmedBalance = wallet.getActualBalance();
    const uint64_t totalBalance = unconfirmedBalance + confirmedBalance;

    const uint32_t localHeight = node.getLastLocalBlockHeight();
    const uint32_t remoteHeight = node.getLastKnownBlockHeight();
    const uint32_t walletHeight = wallet.getBlockCount();

    std::cout << "Available balance: "
              << SuccessMsg(formatAmount(confirmedBalance)) << std::endl
              << "Locked (unconfirmed) balance: "
              << WarningMsg(formatAmount(unconfirmedBalance))
              << std::endl << "Total balance: "
              << InformationMsg(formatAmount(totalBalance)) << std::endl;

    if (viewWallet)
    {
        std::cout << std::endl 
                  << InformationMsg("Please note that view only wallets "
                                    "can only track incoming transactions,")
                  << std::endl
                  << InformationMsg("and so your wallet balance may appear "
                                    "inflated.") << std::endl;
    }

    if (localHeight < remoteHeight)
    {
        std::cout << std::endl
                  << InformationMsg("Your daemon is not fully synced with "
                                    "the network!")
                  << std::endl
                  << "Your balance may be incorrect until you are fully "
                  << "synced!" << std::endl;
    }
    /* Small buffer because wallet height doesn't update instantly like node
       height does */
    else if (walletHeight + 1000 < remoteHeight)
    {
        std::cout << std::endl
                  << InformationMsg("The blockchain is still being scanned for "
                                    "your transactions.")
                  << std::endl
                  << "Balances might be incorrect whilst this is ongoing."
                  << std::endl;
    }
}

void blockchainHeight(CryptoNote::INode &node, CryptoNote::WalletGreen &wallet)
{
    const uint32_t localHeight = node.getLastLocalBlockHeight();
    const uint32_t remoteHeight = node.getLastKnownBlockHeight();
    const uint32_t walletHeight = wallet.getBlockCount();

    /* This is the height that the wallet has been scanned to. The blockchain
       can be fully updated, but we have to walk the chain to find our
       transactions, and this number indicates that progress. */
    std::cout << "Wallet blockchain height: ";

    /* Small buffer because wallet height doesn't update instantly like node
       height does */
    if (walletHeight + 1000 > remoteHeight)
    {
        std::cout << SuccessMsg(std::to_string(walletHeight));
    }
    else
    {
        std::cout << WarningMsg(std::to_string(walletHeight));
    }

    std::cout << std::endl << "Local blockchain height: ";

    if (localHeight == remoteHeight)
    {
        std::cout << SuccessMsg(std::to_string(localHeight));
    }
    else
    {
        std::cout << WarningMsg(std::to_string(localHeight));
    }

    std::cout << std::endl << "Network blockchain height: "
              << SuccessMsg(std::to_string(remoteHeight)) << std::endl;

    if (localHeight == 0 && remoteHeight == 0)
    {
        std::cout << WarningMsg("Uh oh, it looks like you don't have ")
                  << WarningMsg(WalletConfig::daemonName)
                  << WarningMsg(" open!")
                  << std::endl;
    }
    else if (walletHeight + 1000 < remoteHeight && localHeight == remoteHeight)
    {
        std::cout << InformationMsg("You are synced with the network, but the "
                                    "blockchain is still being scanned for "
                                    "your transactions.")
                  << std::endl
                  << "Balances might be incorrect whilst this is ongoing."
                  << std::endl;
    }
    else if (localHeight == remoteHeight)
    {
        std::cout << SuccessMsg("Yay! You are synced!") << std::endl;
    }
    else
    {
        std::cout << WarningMsg("Be patient, you are still syncing with the "
                                "network!") << std::endl;
    }
}

void reset(CryptoNote::INode &node, std::shared_ptr<WalletInfo> &walletInfo)
{
    std::cout << InformationMsg("Resetting wallet...") << std::endl;

    walletInfo->knownTransactionCount = 0;

    /* Wallet is now unitialized. You must reinit with load, initWithKeys,
       or whatever. This function wipes the cache, then saves the wallet. */
    walletInfo->wallet.clearCacheAndShutdown();

    /* Now, we reopen the wallet. It now has no cached tx's, and balance */
    walletInfo->wallet.load(walletInfo->walletFileName,
                            walletInfo->walletPass);

    /* Now we rescan the chain to re-discover our balance and transactions */
    syncWallet(node, walletInfo);
}

void saveCSV(CryptoNote::WalletGreen &wallet, CryptoNote::INode &node)
{
    const size_t numTransactions = wallet.getTransactionCount();

    std::ofstream csv;
    csv.open(WalletConfig::csvFilename);

    if (!csv)
    {
        std::cout << WarningMsg("Couldn't open transactions.csv file for "
                                "saving!")
                  << std::endl
                  << WarningMsg("Ensure it is not open in any other "
                                "application.")
                  << std::endl;
        return;
    }

    std::cout << InformationMsg("Saving CSV file...") << std::endl;

    /* Create CSV header */
    csv << "Timestamp,Block Height,Hash,Amount,In/Out"
        << std::endl;

    /* Loop through transactions */
    for (size_t i = 0; i < numTransactions; i++)
    {
        const CryptoNote::WalletTransaction t = wallet.getTransaction(i);

        /* Ignore fusion transactions */
        if (t.totalAmount == 0)
        {
            continue;
        }

        const std::string amount = formatAmountBasic(std::abs(t.totalAmount));

        const std::string direction = t.totalAmount > 0 ? "IN" : "OUT";

        csv << unixTimeToDate(t.timestamp) << ","       /* Timestamp */
            << t.blockHeight << ","                     /* Block Height */
            << Common::podToHex(t.hash) << ",";         /* Hash */
            
            /* Make outgoing transfers negative for easier spreadsheet math */
            if (t.totalAmount < 0)
            {
                csv << "-" << amount << ",";
            }
            else
            {
                csv << amount << ",";
            }
            
        csv << direction                                /* In/Out */
            << std::endl;
    }

    csv.close();

    std::cout << SuccessMsg("CSV successfully written to ")
              << SuccessMsg(WalletConfig::csvFilename)
              << SuccessMsg("!")
              << std::endl;
}

void printOutgoingTransfer(CryptoNote::WalletTransaction t,
                           CryptoNote::INode &node)
{
    std::cout << WarningMsg("Outgoing transfer:")
              << std::endl
              << WarningMsg("Hash: " + Common::podToHex(t.hash))
              << std::endl;

    if (t.timestamp != 0)
    {
        std::cout << WarningMsg("Block height: ")
                  << WarningMsg(std::to_string(t.blockHeight))
                  << std::endl
                  << WarningMsg("Timestamp: ")
                  << WarningMsg(unixTimeToDate(t.timestamp))
                  << std::endl;
    }

    std::cout << WarningMsg("Spent: " + formatAmount(-t.totalAmount - t.fee))
              << std::endl
              << WarningMsg("Fee: " + formatAmount(t.fee))
              << std::endl
              << WarningMsg("Total Spent: " + formatAmount(-t.totalAmount))
              << std::endl;

    const std::string paymentID = getPaymentIDFromExtra(t.extra);

    if (paymentID != "")
    {
        std::cout << WarningMsg("Payment ID: " + paymentID) << std::endl;
    }

    std::cout << std::endl;
}

void printIncomingTransfer(CryptoNote::WalletTransaction t,
                           CryptoNote::INode &node)
{
    std::cout << SuccessMsg("Incoming transfer:")
              << std::endl
              << SuccessMsg("Hash: " + Common::podToHex(t.hash))
              << std::endl;

    if (t.timestamp != 0)
    {
        std::cout << SuccessMsg("Block height: ")
                  << SuccessMsg(std::to_string(t.blockHeight))
                  << std::endl
                  << SuccessMsg("Timestamp: ")
                  << SuccessMsg(unixTimeToDate(t.timestamp))
                  << std::endl;
    }


    std::cout << SuccessMsg("Amount: " + formatAmount(t.totalAmount))
              << std::endl;

    const std::string paymentID = getPaymentIDFromExtra(t.extra);

    if (paymentID != "")
    {
        std::cout << SuccessMsg("Payment ID: " + paymentID) << std::endl;
    }

    std::cout << std::endl;
}

void listTransfers(bool incoming, bool outgoing, 
                   CryptoNote::WalletGreen &wallet, CryptoNote::INode &node)
{
    const size_t numTransactions = wallet.getTransactionCount();

    int64_t totalSpent = 0;
    int64_t totalReceived = 0;

    for (size_t i = 0; i < numTransactions; i++)
    {
        const CryptoNote::WalletTransaction t = wallet.getTransaction(i);

        if (t.totalAmount < 0 && outgoing)
        {
            printOutgoingTransfer(t, node);
            totalSpent += -t.totalAmount;
        }
        else if (t.totalAmount > 0 && incoming)
        {
            printIncomingTransfer(t, node);
            totalReceived += t.totalAmount;
        }
    }

    if (incoming)
    {
        std::cout << SuccessMsg("Total received: " 
                              + formatAmount(totalReceived))
                  << std::endl;
    }

    if (outgoing)
    {
        std::cout << WarningMsg("Total spent: " + formatAmount(totalSpent))
                  << std::endl;
    }
}

void save(CryptoNote::WalletGreen &wallet)
{
    std::cout << InformationMsg("Saving.") << std::endl;
    wallet.save();
    std::cout << InformationMsg("Saved.") << std::endl;
}

void createIntegratedAddress()
{
    std::cout << InformationMsg("Creating an integrated address from an ")
              << InformationMsg("address and payment ID pair...")
              << std::endl << std::endl;

    std::string address;
    std::string paymentID;

    while (true)
    {
        std::cout << InformationMsg("Address: ");

        std::getline(std::cin, address);
        boost::algorithm::trim(address);

        std::cout << std::endl;

        if (parseStandardAddress(address, true))
        {
            break;
        }
    }

    while (true)
    {
        std::cout << InformationMsg("Payment ID: ");

        std::getline(std::cin, paymentID);
        boost::algorithm::trim(paymentID);

        std::vector<uint8_t> extra;

        std::cout << std::endl;

        if (!CryptoNote::createTxExtraWithPaymentId(paymentID, extra))
        {
            std::cout << WarningMsg("Failed to parse! Payment ID's are 64 "
                                    "character hexadecimal strings.")
                      << std::endl << std::endl;

            continue;
        }

        break;
    }

    std::cout << InformationMsg(createIntegratedAddress(address, paymentID))
              << std::endl;
}
