// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <config/CryptoNoteConfig.h>

struct Config
{
    /* Should we listen for requests from other PC's (0.0.0.0 vs 127.0.0.1) */
    bool acceptExternalRequests;

    /* What port should we listen on */
    uint16_t port;

    /* Password the user must supply with each request */
    std::string rpcPassword;

    /* The value to use with the 'Access-Control-Allow-Origin' header */
    std::string corsHeader;
};

Config parseArguments(int argc, char **argv);
