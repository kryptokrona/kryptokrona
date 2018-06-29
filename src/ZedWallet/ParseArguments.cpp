// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

/////////////////////////////////////
#include <ZedWallet/ParseArguments.h>
/////////////////////////////////////

#include "CryptoNoteConfig.h"

#include "version.h"

#include <ZedWallet/WalletConfig.h>

/* Thanks to https://stackoverflow.com/users/85381/iain for this small command
   line parsing snippet! https://stackoverflow.com/a/868894/8737306 */
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    auto it = std::find(begin, end, option);

    if (it != end && ++it != end)
    {
        return *it;
    }

    return 0;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option)
{
    return std::find(begin, end, option) != end;
}

Config parseArguments(int argc, char **argv)
{
    Config config;

    if (cmdOptionExists(argv, argv+argc, "-h")
     || cmdOptionExists(argv, argv+argc, "--help"))
    {
        helpMessage();
        config.exit = true;
        return config;
    }

    if (cmdOptionExists(argv, argv+argc, "-v")
     || cmdOptionExists(argv, argv+argc, "--version"))
    {
        versionMessage();
        config.exit = true;
        return config;
    }

    if (cmdOptionExists(argv, argv+argc, "--debug"))
    {
        config.debug = true;
    }

    if (cmdOptionExists(argv, argv+argc, "--wallet-file"))
    {
        char *wallet = getCmdOption(argv, argv+argc, "--wallet-file");

        if (!wallet)
        {
            std::cout << "--wallet-file was specified, but no wallet file "
                      << "was given!" << std::endl;

            helpMessage();
            config.exit = true;
            return config;
        }

        config.walletFile = std::string(wallet);
        config.walletGiven = true;
    }

    if (cmdOptionExists(argv, argv+argc, "--password"))
    {
        char *password = getCmdOption(argv, argv+argc, "--password");

        if (!password)
        {
            std::cout << "--password was specified, but no password was "
                      << "given!" << std::endl;

            helpMessage();
            config.exit = true;
            return config;
        }

        config.walletPass = std::string(password);
        config.passGiven = true;
    }

    if (cmdOptionExists(argv, argv+argc, "--remote-daemon"))
    {
        char *url = getCmdOption(argv, argv + argc, "--remote-daemon");

        /* No url following --remote-daemon */
        if (!url)
        {
            std::cout << "--remote-daemon was specified, but no daemon was "
                      << "given!" << std::endl;

            helpMessage();

            config.exit = true;
        }
        else
        {
            std::string urlString(url);

            /* Get the index of the ":" */
            size_t splitter = urlString.find_last_of(":");

            /* Host is everything before ":" */
            config.host = urlString.substr(0, splitter);

            /* No ":" present, or user specifies http:// without port at end */
            if (splitter == std::string::npos || config.host == "http"
             || config.host == "https")
            {
                config.host = urlString;
            }
            else
            {
                /* Host is everything before ":" */
                config.host = urlString.substr(0, splitter);

                /* Port is everything after ":" */
                std::string port = urlString.substr(splitter + 1,   
                                                    std::string::npos);

                try
                {
                    config.port = std::stoi(port);
                }
                catch (const std::invalid_argument &)
                {
                    std::cout << "Failed to parse daemon port!" << std::endl;
                    config.exit = true;
                }
            }
        }
    }

    return config;
}

void versionMessage()
{
    std::cout << WalletConfig::coinName << " v" << PROJECT_VERSION << " "
              << WalletConfig::walletName << std::endl;

}

/* TODO: Make this into a commands struct like help for better printing */
void helpMessage()
{
    versionMessage();

    std::cout << std::endl
              << "zedwallet "
              << "[--remote-daemon <url>] "
              << "[--wallet-file <file>] "
              << "[--password <pass>]"
              << std::endl << std::endl
              << "Commands:" << std::endl << "  -h, " << std::left
              << std::setw(25) << "--help"
              << "Display this help message and exit"
              << std::endl << "  -v, " << std::left << std::setw(25)
              << "--version" << "Display the version information and exit"
              << std::endl << "      " << std::left << std::setw(25)
              << "--debug" << "Enable walletd debugging to zedwallet.log"
              << std::endl << "      " << std::left << std::setw(25)
              << "--remote-daemon <url>" << "Connect to the remote daemon at "
              << "<url>"
              << std::endl << "      " << std::left << std::setw(25)
              << "--wallet-file <file>" << "Open the wallet <file>"
              << std::endl << "      " << std::left << std::setw(25)
              << "--password <pass>" << "Use the password <pass> to open the "
              << "wallet"
              << std::endl;
}
