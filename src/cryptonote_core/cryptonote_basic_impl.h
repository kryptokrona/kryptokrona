// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "common/string_tools.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "cryptonote_core/cryptonote_basic.h"

namespace cryptonote
{
    /************************************************************************/
    /* CryptoNote helper functions                                          */
    /************************************************************************/
    uint64_t getPenalizedAmount(uint64_t amount, size_t medianSize, size_t currentBlockSize);
    std::string getAccountAddressAsStr(uint64_t prefix, const AccountPublicAddress &adr);
    bool parseAccountAddressString(uint64_t &prefix, AccountPublicAddress &adr, const std::string &str);
}

bool parse_hash256(const std::string &str_hash, crypto::Hash &hash);
