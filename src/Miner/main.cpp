// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "MinerManager.h"

#include <System/Dispatcher.h>

int main(int argc, char **argv)
{
    while (true)
    {
        CryptoNote::MiningConfig config;
        config.parse(argc, argv);

        try
        {
            System::Dispatcher dispatcher;
            Miner::MinerManager app(dispatcher, config);

            app.start();
        }
        catch (const std::exception& e)
        {
            std::cout << "Unhandled exception caught: " << e.what()
                      << "\nAttempting to relaunch..." << std::endl;
        }
    }
}
