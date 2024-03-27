// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <crypto_types.h>
#include "wallet_service_configuration.h"
#include "p2p/net_node_config.h"

namespace payment_service
{

    class ConfigurationManager
    {
    public:
        ConfigurationManager();
        bool init(int argc, char **argv);

        WalletServiceConfiguration serviceConfig;

        crypto::Hash rpcSecret;
    };

} // namespace payment_service