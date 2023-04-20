// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "itransaction.h"
#include <functional>
#include <cstring>

namespace cryptonote
{

    inline bool operator==(const AccountPublicAddress &_v1, const AccountPublicAddress &_v2)
    {
        return memcmp(&_v1, &_v2, sizeof(AccountPublicAddress)) == 0;
    }

}

namespace std
{

    template <>
    struct hash<cryptonote::AccountPublicAddress>
    {
        size_t operator()(const cryptonote::AccountPublicAddress &val) const
        {
            size_t spend = *(reinterpret_cast<const size_t *>(&val.spendPublicKey));
            size_t view = *(reinterpret_cast<const size_t *>(&val.viewPublicKey));
            return spend ^ view;
        }
    };

}
