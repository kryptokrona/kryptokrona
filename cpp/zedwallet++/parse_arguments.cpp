// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

///////////////////////////////////////
#include <zedwallet++/parse_arguments.h>
///////////////////////////////////////

#include <cxxopts.hpp>

#include <config/cli_header.h>
#include <config/cryptonote_config.h>
#include <config/wallet_config.h>

#include <zedwallet++/utilities.h>

#include "version.h"

Config parseArguments(int argc, char **argv)
{
    Config config;

    std::string defaultRemoteDaemon = "127.0.0.1:" + std::to_string(cryptonote::RPC_DEFAULT_PORT);

    cxxopts::Options options(argv[0], cryptonote::getProjectCLIHeader());

    bool help, version;

    std::string remoteDaemon;

    options.add_options("Core")("h,help", "Display this help message", cxxopts::value<bool>(help)->implicit_value("true"))("v,version", "Output software version information", cxxopts::value<bool>(version)->default_value("false")->implicit_value("true"));

    options.add_options("Daemon")("r,remote-daemon", "The daemon <host:port> combination to use for node operations.",
                                  cxxopts::value<std::string>(remoteDaemon)->default_value(defaultRemoteDaemon), "<host:port>");

    options.add_options("Wallet")("w,wallet-file", "Open the wallet <file>", cxxopts::value<std::string>(config.walletFile), "<file>")("p,password", "Use the password <pass> to open the wallet", cxxopts::value<std::string>(config.walletPass), "<pass>");

    try
    {
        const auto result = options.parse(argc, argv);

        config.walletGiven = result.count("wallet-file") != 0;

        /* We could check if the string is empty, but an empty password is valid */
        config.passGiven = result.count("password") != 0;
    }
    catch (const cxxopts::OptionException &e)
    {
        std::cout << "Error: Unable to parse command line argument options: " << e.what() << std::endl
                  << std::endl;
        std::cout << options.help({}) << std::endl;
        exit(1);
    }

    if (help) // Do we want to display the help message?
    {
        std::cout << options.help({}) << std::endl;
        exit(0);
    }
    else if (version) // Do we want to display the software version?
    {
        std::cout << cryptonote::getProjectCLIHeader() << std::endl;
        exit(0);
    }

    if (!remoteDaemon.empty())
    {
        if (!zed_utilities::parseDaemonAddressFromString(config.host, config.port, remoteDaemon))
        {
            std::cout << "There was an error parsing the --remote-daemon you specified" << std::endl;
            exit(1);
        }
    }

    return config;
}
