#include "ParseArguments.h"

/* Thanks to https://stackoverflow.com/users/85381/iain for this small command
   line parsing snippet! https://stackoverflow.com/a/868894/8737306 */
char* getCmdOption(char ** begin, char ** end, const std::string & option)
{
    char ** itr = std::find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
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
    config.exit = false;
    config.host = "0.0.0.0";
    config.port = CryptoNote::RPC_DEFAULT_PORT;

    if (cmdOptionExists(argv, argv+argc, "-h")
     || cmdOptionExists(argv, argv+argc, "--help"))
    {
        helpMessage();
        config.exit = true;
    }
    else if (cmdOptionExists(argv, argv+argc, "-v")
          || cmdOptionExists(argv, argv+argc, "--version"))
    {
        versionMessage();
        config.exit = true;
    }
    else if (cmdOptionExists(argv, argv+argc, "--remote-daemon"))
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
            size_t splitter = urlString.find_first_of(":");

            /* Host is everything before ":" */
            config.host = urlString.substr(0, splitter);

            /* Port is everything after ":" */
            std::string port = urlString.substr(splitter + 1,   
                                                std::string::npos);

            try
            {
                config.port = std::stoi(port);
            }
            catch (const std::invalid_argument &e)
            {
                std::cout << "Failed to parse daemon port!" << std::endl;
                config.exit = true;
            }
        }
    }

    return config;
}

void versionMessage()
{
    std::cout << "TurtleCoin v" << PROJECT_VERSION << " Simplewallet"
              << std::endl;
}

void helpMessage()
{
    versionMessage();

    std::cout << std::endl << "simplewallet [--version] [--help] "
              << "[--remote-daemon <url>]" << std::endl << std::endl
              << "Commands:" << std::endl << "  -h, " << std::left
              << std::setw(25) << "--help"
              << "Display this help message and exit"
              << std::endl << "  -v, " << std::left << std::setw(25)
              << "--version" << "Display the version information and exit"
              << std::endl << "      " << std::left << std::setw(25)
              << "--remote-daemon <url>" << "Connect to the remote daemon at "
              << "<url> instead of the default: 0.0.0.0:11898" << std::endl;
}
