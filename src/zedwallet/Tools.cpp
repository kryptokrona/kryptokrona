// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

////////////////////////////
#include <zedwallet/Tools.h>
////////////////////////////

#include <cmath>
#include <thread>

#include <Common/Base58.h>
#include <Common/StringTools.h>

#include <CryptoNoteCore/CryptoNoteBasicImpl.h>
#include <CryptoNoteCore/CryptoNoteTools.h>
#include <CryptoNoteCore/TransactionExtra.h>

#include <fstream>

#include <iostream>

#include <Utilities/ColouredMsg.h>
#include <zedwallet/PasswordContainer.h>
#include <config/wallet_config.h>

void confirmPassword(const std::string &walletPass, const std::string &msg)
{
    /* Password container requires an rvalue, we don't want to wipe our current
       pass so copy it into a tmp string and std::move that instead */
    std::string tmpString = walletPass;
    Tools::PasswordContainer pwdContainer(std::move(tmpString));

    while (!pwdContainer.read_and_validate(msg))
    {
        std::cout << WarningMsg("Incorrect password! Try again.") << std::endl;
    }
}

/* Get the amount we need to divide to convert from atomic to pretty print,
   e.g. 100 for 2 decimal places */
uint64_t getDivisor()
{
    return static_cast<uint64_t>(pow(10, WalletConfig::numDecimalPlaces));
}

std::string formatAmount(const uint64_t amount)
{
    const uint64_t divisor = getDivisor();
    const uint64_t dollars = amount / divisor;
    const uint64_t cents = amount % divisor;

    return formatDollars(dollars) + "." + formatCents(cents) + " " + WalletConfig::ticker;
}

std::string formatAmountBasic(const uint64_t amount)
{
    const uint64_t divisor = getDivisor();
    const uint64_t dollars = amount / divisor;
    const uint64_t cents = amount % divisor;

    return std::to_string(dollars) + "." + formatCents(cents);
}

std::string formatDollars(const uint64_t amount)
{
    /* We want to format our number with comma separators so it's easier to
       use. Now, we could use the nice print_money() function to do this.
       However, whilst this initially looks pretty handy, if we have a locale
       such as ja_JP.utf8, 1 TRTL will actually be formatted as 100 TRTL, which
       is terrible, and could really screw over users.

       So, easy solution right? Just use en_US.utf8! Sure, it's not very
       international, but it'll work! Unfortunately, no. The user has to have
       the locale installed, and if they don't, we get a nasty error at
       runtime.

       Annoyingly, there's no easy way to comma separate numbers outside of
       using the locale method, without writing a pretty long boiler plate
       function. So, instead, we define our own locale, which just returns
       the values we want.

       It's less internationally friendly than we would potentially like
       but that would require a ton of scrutinization which if not done could
       land us with quite a few issues and rightfully angry users.
       Furthermore, we'd still have to hack around cases like JP locale
       formatting things incorrectly, and it makes reading in inputs harder
       too. */

    /* Thanks to https://stackoverflow.com/a/7277333/8737306 for this neat
       workaround */
    class comma_numpunct : public std::numpunct<char>
    {
    protected:
        virtual char do_thousands_sep() const
        {
            return ',';
        }

        virtual std::string do_grouping() const
        {
            return "\03";
        }
    };

    std::locale comma_locale(std::locale(), new comma_numpunct());
    std::stringstream stream;
    stream.imbue(comma_locale);
    stream << amount;
    return stream.str();
}

/* Pad to the amount of decimal spaces, e.g. with 2 decimal spaces 5 becomes
   05, 50 remains 50 */
std::string formatCents(const uint64_t amount)
{
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(WalletConfig::numDecimalPlaces)
           << amount;
    return stream.str();
}

bool confirm(const std::string &msg)
{
    return confirm(msg, true);
}

/* defaultReturn = what value we return on hitting enter, i.e. the "expected"
   workflow */
bool confirm(const std::string &msg, const bool defaultReturn)
{
    /* In unix programs, the upper case letter indicates the default, for
       example when you hit enter */
    std::string prompt = " (Y/n): ";

    /* Yes, I know I can do !defaultReturn. It doesn't make as much sense
       though. If someone deletes this comment to make me look stupid I'll be
       mad >:( */
    if (defaultReturn == false)
    {
        prompt = " (y/N): ";
    }

    while (true)
    {
        std::cout << InformationMsg(msg + prompt);

        std::string answer;
        std::getline(std::cin, answer);

        const char c = ::tolower(answer[0]);

        switch (c)
        {
        /* Lets people spam enter / choose default value */
        case '\0':
            return defaultReturn;
        case 'y':
            return true;
        case 'n':
            return false;
        }

        std::cout << WarningMsg("Bad input: ") << InformationMsg(answer)
                  << WarningMsg(" - please enter either Y or N.")
                  << std::endl;
    }
}

std::string getPaymentIDFromExtra(const std::string &extra)
{
    std::string paymentID;

    if (extra.length() > 0)
    {
        std::vector<uint8_t> vecExtra;

        for (const auto it : extra)
        {
            vecExtra.push_back(static_cast<uint8_t>(it));
        }

        Crypto::Hash paymentIdHash;

        if (CryptoNote::getPaymentIdFromTxExtra(vecExtra, paymentIdHash))
        {
            return Common::podToHex(paymentIdHash);
        }
    }

    return paymentID;
}

std::string unixTimeToDate(const uint64_t timestamp)
{
    const std::time_t time = timestamp;
    char buffer[100];
    std::strftime(buffer, sizeof(buffer), "%F %R", std::localtime(&time));
    return std::string(buffer);
}

std::string createIntegratedAddress(const std::string &address,
                                    const std::string &paymentID)
{
    uint64_t prefix;

    CryptoNote::AccountPublicAddress addr;

    /* Get the private + public key from the address */
    CryptoNote::parseAccountAddressString(prefix, addr, address);

    /* Pack as a binary array */
    CryptoNote::BinaryArray ba;
    CryptoNote::toBinaryArray(addr, ba);
    std::string keys = Common::asString(ba);

    /* Encode prefix + paymentID + keys as an address */
    return Tools::Base58::encode_addr(
        CryptoNote::parameters::CRYPTONOTE_PUBLIC_ADDRESS_BASE58_PREFIX,
        paymentID + keys);
}

uint64_t getScanHeight()
{
    while (true)
    {
        std::cout << InformationMsg("What height would you like to begin ")
                  << InformationMsg("scanning your wallet from?")
                  << std::endl
                  << std::endl
                  << "This can greatly speed up the initial wallet "
                  << "scanning process."
                  << std::endl
                  << std::endl
                  << "If you do not know the exact height, "
                  << "err on the side of caution so transactions do not "
                  << "get missed."
                  << std::endl
                  << std::endl
                  << InformationMsg("Hit enter for the sub-optimal default ")
                  << InformationMsg("of zero: ");

        std::string stringHeight;

        std::getline(std::cin, stringHeight);

        /* Remove commas so user can enter height as e.g. 200,000 */
        removeCharFromString(stringHeight, ',');

        if (stringHeight == "")
        {
            return 0;
        }

        try
        {
            return std::stoull(stringHeight);
        }
        catch (const std::out_of_range &)
        {
            std::cout << WarningMsg("Input is too large or too small!");
        }
        catch (const std::invalid_argument &)
        {
            std::cout << WarningMsg("Failed to parse height - input is not ")
                      << WarningMsg("a number!") << std::endl
                      << std::endl;
        }
    }
}

/* Erases all instances of c from the string. E.g. 2,000,000 becomes 2000000 */
void removeCharFromString(std::string &str, const char c)
{
    str.erase(std::remove(str.begin(), str.end(), c), str.end());
}

/* Trims any whitespace from left and right */
void trim(std::string &str)
{
    rightTrim(str);
    leftTrim(str);
}

void leftTrim(std::string &str)
{
    std::string whitespace = " \t\n\r\f\v";

    str.erase(0, str.find_first_not_of(whitespace));
}

void rightTrim(std::string &str)
{
    std::string whitespace = " \t\n\r\f\v";

    str.erase(str.find_last_not_of(whitespace) + 1);
}

/* Checks if str begins with substring */
bool startsWith(const std::string &str, const std::string &substring)
{
    return str.rfind(substring, 0) == 0;
}

/* Does the given filename exist on disk? */
bool fileExists(const std::string &filename)
{
    /* Bool conversion needs an explicit cast */
    return static_cast<bool>(std::ifstream(filename));
}

bool shutdown(std::shared_ptr<WalletInfo> walletInfo, CryptoNote::INode &node,
              bool &alreadyShuttingDown)
{
    if (alreadyShuttingDown)
    {
        std::cout << "Patience little turtle, we're already shutting down!"
                  << std::endl;

        return false;
    }

    std::cout << InformationMsg("Shutting down...") << std::endl;

    alreadyShuttingDown = true;

    bool finishedShutdown = false;

    std::thread timelyShutdown([&finishedShutdown]
                               {
        const auto startTime = std::chrono::system_clock::now();

        /* Has shutdown finished? */
        while (!finishedShutdown)
        {
            const auto currentTime = std::chrono::system_clock::now();

            /* If not, wait for a max of 20 seconds then force exit. */
            if ((currentTime - startTime) > std::chrono::seconds(20))
            {
                std::cout << WarningMsg("Wallet took too long to save! "
                                        "Force closing.") << std::endl
                          << "Bye." << std::endl;
                exit(0);
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        } });

    if (walletInfo != nullptr)
    {
        std::cout << InformationMsg("Saving wallet file...") << std::endl;

        walletInfo->wallet.save();

        std::cout << InformationMsg("Shutting down wallet interface...")
                  << std::endl;

        walletInfo->wallet.shutdown();
    }

    std::cout << InformationMsg("Shutting down node connection...")
              << std::endl;

    node.shutdown();

    finishedShutdown = true;

    /* Wait for shutdown watcher to finish */
    timelyShutdown.join();

    std::cout << "Bye." << std::endl;

    return true;
}

std::vector<std::string> split(const std::string &str, char delim = ' ')
{
    std::vector<std::string> cont;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim))
    {
        cont.push_back(token);
    }
    return cont;
}

bool parseDaemonAddressFromString(std::string &host, int &port, const std::string &address)
{
    std::vector<std::string> parts = split(address, ':');

    if (parts.empty())
    {
        return false;
    }
    else if (parts.size() >= 2)
    {
        try
        {
            host = parts.at(0);
            port = std::stoi(parts.at(1));
            return true;
        }
        catch (const std::out_of_range &)
        {
            return false;
        }
        catch (const std::invalid_argument &)
        {
            return false;
        }
    }

    host = parts.at(0);
    port = CryptoNote::RPC_DEFAULT_PORT;
    return true;
}
