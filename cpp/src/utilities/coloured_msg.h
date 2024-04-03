// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <common/console_tools.h>

#include <iomanip>

#include <ostream>

#include <string>

template <typename T>
class ColouredMsg
{
public:
    ColouredMsg(
        const T msg,
        const common::console::Color colour) : msg(msg),
                                               colour(colour) {}

    ColouredMsg(
        const T msg,
        const int padding,
        const common::console::Color colour) : msg(msg),
                                               colour(colour),
                                               padding(padding),
                                               pad(true) {}

    /* Set the text colour, write the message, then reset. We use a class
       as it seems the only way to have a valid << operator. We need this
       so we can nicely do something like:

       std::cout << "Hello " << GreenMsg("user") << std::endl;

       Without having to write:

       std::cout << "Hello ";
       GreenMsg("user");
       std::cout << std::endl; */

    friend std::ostream &operator<<(std::ostream &os, const ColouredMsg &m)
    {
        common::console::setTextColor(m.colour);

        if (m.pad)
        {
            os << std::left << std::setw(m.padding) << m.msg;
        }
        else
        {
            os << m.msg;
        }

        common::console::setTextColor(common::console::Color::Default);
        return os;
    }

protected:
    /* Can be any class that supports the << operator */
    T msg;

    /* The colour to use */
    const common::console::Color colour;

    /* The amount to pad the message to */
    const int padding = 0;

    /* If we should pad the message */
    const bool pad = false;
};

template <typename T>
class SuccessMsg : public ColouredMsg<T>
{
public:
    explicit SuccessMsg(T msg)
        : ColouredMsg<T>(msg, common::console::Color::Green) {}

    explicit SuccessMsg(T msg, int padding)
        : ColouredMsg<T>(msg, padding, common::console::Color::Green) {}
};

template <typename T>
class InformationMsg : public ColouredMsg<T>
{
public:
    explicit InformationMsg(T msg)
        : ColouredMsg<T>(msg, common::console::Color::BrightYellow) {}

    explicit InformationMsg(T msg, int padding)
        : ColouredMsg<T>(msg, padding, common::console::Color::BrightYellow) {}
};

template <typename T>
class WarningMsg : public ColouredMsg<T>
{
public:
    explicit WarningMsg(T msg)
        : ColouredMsg<T>(msg, common::console::Color::BrightRed) {}

    explicit WarningMsg(T msg, int padding)
        : ColouredMsg<T>(msg, padding, common::console::Color::BrightRed) {}
};
