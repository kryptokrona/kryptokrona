// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <config/cryptonote_config.h>

struct Config
{
    /* The IP to listen for requests on */
    std::string rpcBindIp;

    /* What port should we listen on */
    uint16_t port;

    /* Password the user must supply with each request */
    std::string rpcPassword;

    /* The value to use with the 'Access-Control-Allow-Origin' header */
    std::string corsHeader;
};

Config parseArguments(int argc, char **argv);
