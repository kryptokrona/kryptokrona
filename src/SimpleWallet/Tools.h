#include <cctype>
#include <fstream>
#include <iostream>
#include <iomanip>

#include <Common/ConsoleTools.h>
#include <SimpleWallet/PasswordContainer.h>

void confirmPassword(std::string walletPass);

bool confirm(std::string msg);

std::string formatAmount(uint64_t amount);

class ColouredMsg
{
    public:
        ColouredMsg(std::string msg, Common::Console::Color colour) 
                  : msg(msg), colour(colour) {}

        ColouredMsg(std::string msg, int padding, 
                    Common::Console::Color colour)
                  : msg(msg), colour(colour), padding(padding), pad(true) {}


        /* Set the text colour, write the message, then reset. We use a class
           as it seems the only way to have a valid << operator. We need this
           so we can nicely do something like:

           std::cout << "Hello " << GreenMsg("user") << std::endl;

           Without having to write:

           std::cout << "Hello ";
           GreenMsg("user");
           std::cout << std::endl; */

        friend std::ostream& operator<<(std::ostream& os, const ColouredMsg &m)
        {
            Common::Console::setTextColor(m.colour);

            if (m.pad)
            {
                os << std::left << std::setw(m.padding) << m.msg;
            }
            else
            {
                os << m.msg;
            }

            Common::Console::setTextColor(Common::Console::Color::Default);
            return os;
        }

    protected:
        std::string msg;
        Common::Console::Color colour;
        int padding = 0;
        bool pad = false;
};

/* These are pretty general rules and are sometimes broken. */

/* Something good happened */
class GreenMsg : public ColouredMsg
{
    public:
        explicit GreenMsg(std::string msg) 
               : ColouredMsg(msg, Common::Console::Color::Green) {}

        explicit GreenMsg(std::string msg, int padding)
               : ColouredMsg(msg, padding, Common::Console::Color::Green) {}
};

/* Notification / Warning / Alert */
class PurpleMsg : public ColouredMsg
{
    public:
        explicit PurpleMsg(std::string msg) 
               : ColouredMsg(msg, Common::Console::Color::BrightMagenta) {}

        explicit PurpleMsg(std::string msg, int padding)
               : ColouredMsg(msg, padding, 
                             Common::Console::Color::BrightMagenta) {}
};

/* Something bad happened */
class RedMsg : public ColouredMsg
{
    public:
        explicit RedMsg(std::string msg) 
               : ColouredMsg(msg, Common::Console::Color::BrightRed) {}

        explicit RedMsg(std::string msg, int padding)
               : ColouredMsg(msg, padding, 
                             Common::Console::Color::BrightRed) {}
};
