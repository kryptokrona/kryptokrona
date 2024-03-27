// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "miner_manager.h"

#include <syst/dispatcher.h>

int main(int argc, char **argv)
{
    while (true)
    {
        cryptonote::MiningConfig config;
        config.parse(argc, argv);

        try
        {
            syst::Dispatcher dispatcher;

            auto httpClient = std::make_shared<httplib::Client>(
                config.daemonHost.c_str(), config.daemonPort, 10 /* 10 second timeout */
            );

            miner::MinerManager app(dispatcher, config, httpClient);

            app.start();
        }
        catch (const std::exception &e)
        {
            std::cout << "Unhandled exception caught: " << e.what()
                      << "\nAttempting to relaunch..." << std::endl;
        }
    }
}
