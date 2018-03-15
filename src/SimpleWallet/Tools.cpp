#include <SimpleWallet/Tools.h>

void confirmPassword(std::string walletPass)
{
    /* Password container requires an rvalue, we don't want to wipe our current
       pass so copy it into a tmp string and std::move that instead */
    std::string tmpString = walletPass;
    Tools::PasswordContainer pwdContainer(std::move(tmpString));

    while (!pwdContainer.read_and_validate())
    {
        std::cout << "Incorrect password! Try again." << std::endl;
    }
}

std::string formatAmount(uint64_t amount)
{
    std::string s = std::to_string(amount);

    size_t numDecimalPlaces = 2;

    if (s.size() < numDecimalPlaces + 1)
    {
        s.insert(0, numDecimalPlaces + 1 - s.size(), '0');
    }

    s.insert(s.size() - numDecimalPlaces, ".");

    s += " TRTL";

    return s;
}

bool confirm(std::string msg)
{
    while (true)
    {
        std::cout << PurpleMsg(msg + " (Y/n): ");

        std::string answer;
        std::getline(std::cin, answer);

        char c = std::tolower(answer[0]);

        /* Lets people spam enter in the transaction screen */
        if (c == 'y' || c == '\0')
        {
            return true;
        }
        else if (c == 'n')
        {
            return false;
        }
        /* Don't loop forever on EOF */
        else if (c == std::ifstream::traits_type::eof())
        {
            return false;
        } 
        else
        {
            std::cout << RedMsg("Bad input: ") << PurpleMsg(answer)
                      << RedMsg(" - please enter either Y or N.") << std::endl;
        }
    }
}
