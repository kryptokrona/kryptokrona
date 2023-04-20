// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <sstream>
#include <config/cryptonote_config.h>
#include <config/ascii.h>
#include <version.h>

namespace cryptonote
{
    inline std::string getProjectCLIHeader()
    {
        std::stringstream programHeader;
        programHeader << std::endl
                      << asciiArt << std::endl
                      << " " << PROJECT_NAME << " v" << PROJECT_VERSION << std::endl
                      << " This software is distributed under the General Public License v3.0"
                      << std::endl
                      << std::endl
                      << " " << PROJECT_COPYRIGHT
                      << std::endl
                      << std::endl
                      << " Additional Copyright(s) may apply, please see the included LICENSE file for more information." << std::endl
                      << " If you did not receive a copy of the LICENSE, please visit:" << std::endl
                      << " " << cryptonote::LICENSE_URL
                      << std::endl
                      << std::endl;

        return programHeader.str();
    }
}