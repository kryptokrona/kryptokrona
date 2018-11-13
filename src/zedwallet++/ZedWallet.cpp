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

void shutdown(std::atomic<bool> &ctrl_c, std::shared_ptr<WalletBackend> walletBackend)
{
    while (!ctrl_c)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    if (walletBackend != nullptr)
    {
        std::cout << InformationMsg("\nSaving and shutting down...\n");

        /* Delete the walletbackend - this will call the deconstructor,
           which will set the appropriate m_shouldStop flag. Since this
           function gets triggered from a signal handler, we can't just call
           save() - The data may be in an invalid state. 
           
           Obviously, calling delete on a shared pointer is undefined
           behaviour if we continue using it in another thread - fortunately,
           we're exiting right now. */
        walletBackend->~WalletBackend();
    }

    std::cout << "Thanks for stopping by..." << std::endl;

    exit(0);
}

int main(int argc, char **argv)
{
    Config config = parseArguments(argc, argv);

    std::cout << InformationMsg(CryptoNote::getProjectCLIHeader()) << std::endl;

    try
    {
        const auto [quit, sync, walletBackend] = selectionScreen(config);

        if (quit)
        {
            std::cout << "Thanks for stopping by..." << std::endl;
            return 0;
        }

        /* Atomic bool to signal if ctrl_c is used */
        std::atomic<bool> ctrl_c(false);

        /* Launch the thread which watches for the shutdown signal */
        std::thread(shutdown, std::ref(ctrl_c), std::ref(walletBackend)).detach();

        /* Trigger the shutdown signal if ctrl+c is used
           We do the actual handling in a separate thread to handle stuff not
           being re-entrant. */
        Tools::SignalHandler::install([&ctrl_c] { ctrl_c = true; });

        /* Don't explicitly sync in foreground if it's a new wallet */
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
