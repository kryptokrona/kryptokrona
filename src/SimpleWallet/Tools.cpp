/*
Copyright (C) 2018, The TurtleCoin developers

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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
        std::cout << InformationMsg(msg + " (Y/n): ");

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
            std::cout << WarningMsg("Bad input: ") << InformationMsg(answer)
                      << WarningMsg(" - please enter either Y or N.") << std::endl;
        }
    }
}
