// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <string>
#include "rpc/core_rpc_server_commands_definitions.h"

namespace utilities
{
    std::string get_mining_speed(const uint64_t hashrate);

    std::string get_sync_percentage(
        uint64_t height,
        const uint64_t target_height);

    std::string get_upgrade_time(
        const uint64_t height,
        const uint64_t upgrade_height);

    std::string get_status_string(cryptonote::COMMAND_RPC_GET_INFO::response iresp);

    std::string formatAmount(const uint64_t amount);

    std::string formatAmountBasic(const uint64_t amount);

    std::string prettyPrintBytes(const uint64_t numBytes);
}
