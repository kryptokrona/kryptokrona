// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

///////////////////////////////
#include <zedwallet++/Fusion.h>
///////////////////////////////

#include <WalletBackend/WalletBackend.h>

#include <zedwallet++/ColouredMsg.h>
#include <zedwallet++/Utilities.h>

void optimize(const std::shared_ptr<WalletBackend> walletBackend)
{
    int optimizationRound = 0;

    while (true)
    {
        std::cout << InformationMsg("Running optimization round ")
                  << optimizationRound << "...\n";

        /* No optimization done on this round, we're done */
        if (!optimizeRound(walletBackend))
        {
            break;
        }

        optimizationRound++;
    }

    std::cout << SuccessMsg("Optimization complete!\n");
}

bool optimizeRound(const std::shared_ptr<WalletBackend> walletBackend)
{
    int failCount = 0;

    int sentTransactions = 0;

    const uint64_t initialBalance = walletBackend->getTotalUnlockedBalance();

    /* Since input selection is random, lets let it fail a few times before
       failing the whole round */
    while (failCount < 5)
    {
        const auto [error, hash] = walletBackend->sendFusionTransactionBasic();

        if (error == FULLY_OPTIMIZED)
        {
            failCount++;
        }
        else if (error)
        {
            failCount++;

            std::stringstream stream;

            stream << "Failed to send fusion transction: " << error << "\n";

            std::cout << WarningMsg(stream.str());
        }
        else
        {
            failCount = 0;

            sentTransactions++;

            std::cout << "Sent fusion transaction #" << sentTransactions
                      << "\nHash: " << hash << "\n";
        }
    }

    uint64_t currentBalance = walletBackend->getTotalUnlockedBalance();

    /* Wait for balance to unlock, so sending transactions can proceed */
    while(currentBalance < initialBalance)
    {
        std::cout << "Waiting for balance to return and unlock:"
                  << "\nTotal balance: " << ZedUtilities::formatAmount(initialBalance)
                  << "\nLocked balance: " << ZedUtilities::formatAmount(initialBalance - currentBalance)
                  << "\nUnlocked balance: " << ZedUtilities::formatAmount(currentBalance)
                  << "\nWill check again in 15 seconds...\n";

        std::this_thread::sleep_for(std::chrono::seconds(15));

        currentBalance = walletBackend->getTotalUnlockedBalance();
    }

    if (sentTransactions != 0)
    {
        std::cout << SuccessMsg("All fusion transactions confirmed!\n");
    }

    /* Return whether we sent any transactions or not. If we did, we will
       probably start another round */
    return sentTransactions != 0;
}
