// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

///////////////////////////
#include <zedwallet/sync.h>
///////////////////////////

#include <thread>

#include <common/string_tools.h>

#include <utilities/coloured_msg.h>
#include <zedwallet/command_implementations.h>
#include <zedwallet/get_input.h>
#include <zedwallet/tools.h>
#include <zedwallet/types.h>
#include <config/wallet_config.h>

void checkForNewTransactions(std::shared_ptr<WalletInfo> walletInfo)
{
    walletInfo->wallet.updateInternalCache();

    const size_t newTransactionCount = walletInfo->wallet.getTransactionCount();

    if (newTransactionCount != walletInfo->knownTransactionCount)
    {
        for (size_t i = walletInfo->knownTransactionCount;
             i < newTransactionCount; i++)
        {
            const cryptonote::WalletTransaction t = walletInfo->wallet.getTransaction(i);

            /* Don't print outgoing or fusion transfers */
            if (t.totalAmount > 0 && t.fee != 0)
            {
                std::cout << std::endl
                          << InformationMsg("New transaction found!")
                          << std::endl
                          << SuccessMsg("Incoming transfer:")
                          << std::endl
                          << SuccessMsg("Hash: " + common::podToHex(t.hash))
                          << std::endl
                          << SuccessMsg("Amount: " + formatAmount(t.totalAmount))
                          << std::endl
                          << InformationMsg(getPrompt(walletInfo))
                          << std::flush;
            }
        }

        walletInfo->knownTransactionCount = newTransactionCount;
    }
}

void syncWallet(cryptonote::INode &node,
                std::shared_ptr<WalletInfo> walletInfo)
{
    uint32_t localHeight = node.getLastLocalBlockHeight();
    uint32_t walletHeight = walletInfo->wallet.getBlockCount();
    uint32_t remoteHeight = node.getLastKnownBlockHeight();

    size_t transactionCount = walletInfo->wallet.getTransactionCount();

    int stuckCounter = 0;

    if (localHeight != remoteHeight)
    {
        std::cout << "Your " << wallet_config::daemonName << " isn't fully "
                  << "synced yet!" << std::endl
                  << "Until you are fully synced, you won't be able to send "
                  << "transactions,"
                  << std::endl
                  << "and your balance may be missing or "
                  << "incorrect!" << std::endl
                  << std::endl;
    }

    /* If we open a legacy wallet then it will load the transactions but not
       have the walletHeight == transaction height. Lets just throw away the
       transactions and rescan. */
    if (walletHeight == 1 && transactionCount != 0)
    {
        std::cout << "Upgrading your wallet from an older version of the "
                  << "software..." << std::endl
                  << "Unfortunately, we have "
                  << "to rescan the chain to find your transactions."
                  << std::endl;

        transactionCount = 0;

        walletInfo->wallet.clearCaches(true, false);
    }

    if (walletHeight == 1)
    {
        std::cout << "Scanning through the blockchain to find transactions "
                  << "that belong to you." << std::endl
                  << "Please wait, this will take some time."
                  << std::endl
                  << std::endl;
    }
    else
    {
        std::cout << "Scanning through the blockchain to find any new "
                  << "transactions you received"
                  << std::endl
                  << "whilst your wallet wasn't open."
                  << std::endl
                  << "Please wait, this may take some time."
                  << std::endl
                  << std::endl;
    }

    int counter = 1;

    while (walletHeight < localHeight)
    {
        /* This MUST be called on the main thread! */
        walletInfo->wallet.updateInternalCache();

        localHeight = node.getLastLocalBlockHeight();
        remoteHeight = node.getLastKnownBlockHeight();
        std::cout << SuccessMsg(std::to_string(walletHeight))
                  << " of " << InformationMsg(std::to_string(localHeight))
                  << std::endl;

        const uint32_t tmpWalletHeight = walletInfo->wallet.getBlockCount();

        int waitSeconds = 1;

        /* Save periodically so if someone closes before completion they don't
           lose all their progress. Saving is actually quite slow with big
           wallets so lets do it every 10 minutes */
        if (counter % 600 == 0)
        {
            std::cout << std::endl
                      << InformationMsg("Saving current progress...")
                      << std::endl
                      << std::endl;

            walletInfo->wallet.save();
        }

        if (tmpWalletHeight == walletHeight)
        {
            stuckCounter++;
            waitSeconds = 3;

            if (stuckCounter > 20)
            {
                std::cout << WarningMsg("Syncing may be stuck. Try restarting ")
                          << WarningMsg(wallet_config::daemonName)
                          << WarningMsg(".")
                          << std::endl
                          << WarningMsg("If this persists, visit ")
                          << WarningMsg(wallet_config::contactLink)
                          << WarningMsg(" for support.")
                          << std::endl;
            }
            else if (stuckCounter > 19)
            {
                /*
                   Calling save has the side-effect of starting
                   and stopping blockchainSynchronizer, which seems
                   to sometimes force the sync to resume properly.
                   So we'll try this before warning the user.
                */
                walletInfo->wallet.save();
                waitSeconds = 5;
            }
        }
        else
        {
            stuckCounter = 0;
            walletHeight = tmpWalletHeight;

            const size_t tmpTransactionCount = walletInfo
                                                   ->wallet.getTransactionCount();

            if (tmpTransactionCount != transactionCount)
            {
                for (size_t i = transactionCount; i < tmpTransactionCount; i++)
                {
                    cryptonote::WalletTransaction t = walletInfo->wallet.getTransaction(i);

                    /* Don't print out fusion transactions */
                    if (t.totalAmount != 0)
                    {
                        std::cout << std::endl
                                  << InformationMsg("New transaction found!")
                                  << std::endl
                                  << std::endl;

                        if (t.totalAmount < 0)
                        {
                            printOutgoingTransfer(t, node);
                        }
                        else
                        {
                            printIncomingTransfer(t, node);
                        }
                    }
                }

                transactionCount = tmpTransactionCount;
            }
        }

        counter++;

        std::this_thread::sleep_for(std::chrono::seconds(waitSeconds));
    }

    std::cout << std::endl
              << SuccessMsg("Finished scanning blockchain!") << std::endl;

    /* In case the user force closes, we don't want them to have to rescan
       the whole chain. */
    walletInfo->wallet.save();

    walletInfo->knownTransactionCount = transactionCount;
}
