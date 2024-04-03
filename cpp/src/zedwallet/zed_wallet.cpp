// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

////////////////////////////////
#include <zedwallet/zed_wallet.h>
////////////////////////////////

#include <config/cli_header.h>
#include <common/signal_handler.h>
#include <cryptonote_core/currency.h>
#include <logging/file_logger.h>
#include <logging/logger_manager.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <utilities/coloured_msg.h>
#include <zedwallet/menu.h>
#include <zedwallet/parse_arguments.h>
#include <zedwallet/tools.h>
#include <config/wallet_config.h>

int main(int argc, char **argv)
{
/* On ctrl+c the program seems to throw "zedwallet.exe has stopped
   working" when calling exit(0)... I'm not sure why, this is a bit of
   a hack, it disables that - possibly some deconstructers calling
   terminate() */
#ifdef _WIN32
    SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX);
#endif

    Config config = parseArguments(argc, argv);

    std::cout << InformationMsg(cryptonote::getProjectCLIHeader()) << std::endl;

    const auto logManager = std::make_shared<logging::LoggerManager>();

    if (config.debug)
    {
        logManager->setMaxLevel(logging::DEBUGGING);

        logging::FileLogger fileLogger;

        fileLogger.init(wallet_config::walletName + ".log");
        logManager->addLogger(fileLogger);
    }

    /* Currency contains our coin parameters, such as decimal places, supply */
    const cryptonote::Currency currency = cryptonote::CurrencyBuilder(logManager).currency();

    syst::Dispatcher localDispatcher;
    syst::Dispatcher *dispatcher = &localDispatcher;

    /* Our connection to kryptokronad */
    std::unique_ptr<cryptonote::INode> node(
        new cryptonote::NodeRpcProxy(config.host, config.port, 10, logManager));

    std::promise<std::error_code> errorPromise;

    /* Once the function is complete, set the error value from the promise */
    auto callback = [&errorPromise](std::error_code e)
    {
        errorPromise.set_value(e);
    };

    /* Get the future of the result */
    auto initNode = errorPromise.get_future();

    node->init(callback);

    /* Connection took too long to remote node, let program continue regardless
       as they could perform functions like export_keys without being
       connected */
    if (initNode.wait_for(std::chrono::seconds(20)) != std::future_status::ready)
    {
        if (config.host != "127.0.0.1")
        {
            std::cout << WarningMsg("Unable to connect to remote node, "
                                    "connection timed out.")
                      << std::endl
                      << WarningMsg("Confirm the remote node is functioning, "
                                    "or try a different remote node.")
                      << std::endl
                      << std::endl;
        }
        else
        {
            std::cout << WarningMsg("Unable to connect to node, "
                                    "connection timed out.")
                      << std::endl
                      << std::endl;
        }
    }

    /*
      This will check to see if the node responded to /feeinfo and actually
      returned something that it expects us to use for convenience charges
      for using that node to send transactions.
    */
    if (node->feeAmount() != 0 && !node->feeAddress().empty())
    {
        std::stringstream feemsg;

        feemsg << std::endl
               << "You have connected to a node that charges "
               << "a fee to send transactions." << std::endl
               << std::endl
               << "The fee for sending transactions is: " << formatAmount(node->feeAmount()) << " per transaction." << std::endl
               << std::endl
               << "If you don't want to pay the node fee, please "
               << "relaunch " << wallet_config::walletName << " and specify a different node or run your own." << std::endl;

        std::cout << WarningMsg(feemsg.str()) << std::endl;
    }

    /* Create the wallet instance */
    cryptonote::WalletGreen wallet(*dispatcher, currency, *node, logManager);

    /* Run the interactive wallet interface */
    run(wallet, *node, config);
}

void run(cryptonote::WalletGreen &wallet, cryptonote::INode &node,
         Config &config)
{
    auto [quit, walletInfo] = selectionScreen(config, wallet, node);

    bool alreadyShuttingDown = false;

    if (!quit)
    {
        /* Call shutdown on ctrl+c */
        /* walletInfo = walletInfo - workaround for
           https://stackoverflow.com/a/46115028/8737306 - standard &
           capture works in newer compilers. */
        tools::SignalHandler::install([&walletInfo = walletInfo, &node,
                                       &alreadyShuttingDown]
                                      {
            /* If we're already shutting down let control flow continue
               as normal */
            if (shutdown(walletInfo, node, alreadyShuttingDown))
            {
                exit(0);
            } });

        mainLoop(walletInfo, node);
    }

    shutdown(walletInfo, node, alreadyShuttingDown);
}
