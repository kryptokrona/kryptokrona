// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "console_logger.h"
#include <iostream>
#include <unordered_map>
#include <common/console_tools.h>

namespace logging
{

    using common::console::Color;

    ConsoleLogger::ConsoleLogger(Level level) : CommonLogger(level)
    {
    }

    void ConsoleLogger::doLogString(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mutex);
        bool readingText = true;
        bool changedColor = false;
        std::string color = "";

        static std::unordered_map<std::string, Color> colorMapping = {
            {BLUE, Color::Blue},
            {GREEN, Color::Green},
            {RED, Color::Red},
            {YELLOW, Color::Yellow},
            {WHITE, Color::White},
            {CYAN, Color::Cyan},
            {MAGENTA, Color::Magenta},

            {BRIGHT_BLUE, Color::BrightBlue},
            {BRIGHT_GREEN, Color::BrightGreen},
            {BRIGHT_RED, Color::BrightRed},
            {BRIGHT_YELLOW, Color::BrightYellow},
            {BRIGHT_WHITE, Color::BrightWhite},
            {BRIGHT_CYAN, Color::BrightCyan},
            {BRIGHT_MAGENTA, Color::BrightMagenta},

            {DEFAULT, Color::Default}};

        for (size_t charPos = 0; charPos < message.size(); ++charPos)
        {
            if (message[charPos] == ILogger::COLOR_DELIMETER)
            {
                readingText = !readingText;
                color += message[charPos];
                if (readingText)
                {
                    auto it = colorMapping.find(color);
                    common::console::setTextColor(it == colorMapping.end() ? Color::Default : it->second);
                    changedColor = true;
                    color.clear();
                }
            }
            else if (readingText)
            {
                std::cout << message[charPos];
            }
            else
            {
                color += message[charPos];
            }
        }

        if (changedColor)
        {
            common::console::setTextColor(Color::Default);
        }
    }

}
