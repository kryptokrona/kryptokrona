// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <config/CryptoNoteConfig.h>

struct Config
{
    /* Was the wallet file specified on CLI */
    bool walletGiven = false;

    /* Was the wallet pass specified on CLI */
    bool passGiven = false;

    /* The daemon host */
    std::string host;
    
    /* The daemon port */
    uint16_t port = CryptoNote::RPC_DEFAULT_PORT;

    /* The wallet file path */
    std::string walletFile;

    /* The wallet password */
    std::string walletPass;
};

Config parseArguments(int argc, char **argv);
