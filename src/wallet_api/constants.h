// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <config/wallet_config.h>

namespace api_constants
{
    /* The number of iterations of PBKDF2 to perform on the wallet
       password. */
    const uint64_t PBKDF2_ITERATIONS = 10000;

    /* The length of the address after removing the prefix. The SEKR and Xkr
       prefixes differ in length by one char, so the body (95 chars) is the same
       for both -- only the prefix differs. */
    const uint16_t addressBodyLength = wallet_config::standardAddressLength - wallet_config::addressPrefix.length();

    /* Matches an address under either prefix, e.g. (?:SEKR|Xkr)[a-zA-Z0-9]{95},
       so wallet-api routes with an {address} path segment accept both forms. */
    const std::string addressRegex = "(?:" + std::string(wallet_config::addressPrefix) + "|" + std::string(wallet_config::addressPrefixAlt) + ")[a-zA-Z0-9]{" + std::to_string(addressBodyLength) + "}";

    /* 64 char, hex */
    const std::string hashRegex = "[a-fA-F0-9]{64}";
}
