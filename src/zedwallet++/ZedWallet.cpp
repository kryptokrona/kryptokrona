// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#include <iostream>

#include <Common/SignalHandler.h>

#include <config/CliHeader.h>

#include <zedwallet++/ColouredMsg.h>
#include <zedwallet++/Menu.h>
#include <zedwallet++/ParseArguments.h>
#include <zedwallet++/Sync.h>
#include <zedwallet++/TransactionMonitor.h>

int main(int argc, char **argv)
{
    Config config = parseArguments(argc, argv);

    std::cout << InformationMsg(CryptoNote::getProjectCLIHeader()) << std::endl;

    try
    {
        const auto [quit, sync, walletBackend] = selectionScreen(config);

        /* Save sync progress and exit if we ctrl+c */
        Tools::SignalHandler::install([&walletBackend = walletBackend]
        {
            if (walletBackend != nullptr)
            {
                std::cout << InformationMsg("\nSaving and shutting down...\n");

                walletBackend->save();
            }

            std::cout << "Thanks for stopping by..." << std::endl;

            exit(0);
        });

        if (!quit)
        {
            if (sync)
            {
                syncWallet(walletBackend);
            }

            /* Init the transaction monitor */
            TransactionMonitor txMonitor(walletBackend);

            /* Launch the transaction monitor in another thread */
            std::thread txMonitorThread(&TransactionMonitor::start, &txMonitor);

            /* Launch the wallet interface */
            mainLoop(walletBackend, txMonitor.getMutex());

            /* Stop the transaction monitor */
            txMonitor.stop();

            /* Wait for the transaction monitor to stop */
            if (txMonitorThread.joinable())
            {
                txMonitorThread.join();
            }
        }

        std::cout << "Thanks for stopping by..." << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cout << WarningMsg("Unexpected error: " + std::string(e.what()))
                  << std::endl
                  << "Please report this error, and what you were doing to "
                  << "cause it." << std::endl;

        std::cout << "Hit any key to exit: ";

        getchar();
    }
}
