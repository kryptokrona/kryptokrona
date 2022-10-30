// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information

#pragma once

const std::string windowsAsciiArt =
      "\n _                    _        _      \n"
        "| |__ _ _  _ _  ___ _| |_ ___ | |__ _ _  ___ ._ _  ___  \n"
        "| / /| '_>| | || . \\ | | / . \\| / /| '_>/ . \\| ' |<_> | \n"
        "|_\\_\\|_|  `_. ||  _/ |_| \\___/|_\\_\\|_|  \\___/|_|_|<___| \n"
        "          <___'|_|                                      \n";

const std::string nonWindowsAsciiArt =
      "\n                                                                            \n"
        "oooo                                            .             oooo     \n"
        "`888                                          .o8             `888                                                 \n"
        " 888  oooo  oooo d8b oooo    ooo oo.ooooo.  .o888oo  .ooooo.   888  oooo  oooo d8b  .ooooo.  ooo. .oo.    .oooo.   \n"
        " 888 .8P'   `888\"\"8P  `88.  .8'   888' `88b   888   d88' `88b  888 .8P'   `888\"\"8P d88' `88b `888P\"Y88b  `P  )88b  \n"
        " 888888.     888       `88..8'    888   888   888   888   888  888888.     888     888   888  888   888   .oP\"888  \n"
        " 888 `88b.   888        `888'     888   888   888 . 888   888  888 `88b.   888     888   888  888   888  d8(  888  \n"
        "o888o o888o d888b        .8'      888bod8P'   \"888\" `Y8bod8P' o888o o888o d888b    `Y8bod8P' o888o o888o `Y888\"\"8o \n"
        "                     .o..P'       888                                                                              \n"
        "                     `Y8P'       o888o                                                                             \n";

/* Windows has some characters it won't display in a terminal. If your ascii
   art works fine on Windows and Linux terminals, just replace 'asciiArt' with
   the art itself, and remove these two #ifdefs and above ascii arts */
#ifdef _WIN32
const std::string asciiArt = windowsAsciiArt;
#else
const std::string asciiArt = nonWindowsAsciiArt;
#endif
