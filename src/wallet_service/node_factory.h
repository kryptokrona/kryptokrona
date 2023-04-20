// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "inode.h"

#include <string>

#include "logging/logger_ref.h"

namespace payment_service
{

    class NodeFactory
    {
    public:
        static cryptonote::INode *createNode(const std::string &daemonAddress, uint16_t daemonPort, uint16_t initTimeout, std::shared_ptr<logging::ILogger> logger);
        static cryptonote::INode *createNodeStub();

    private:
        NodeFactory();
        ~NodeFactory();

        cryptonote::INode *getNode(const std::string &daemonAddress, uint16_t daemonPort);

        static NodeFactory factory;
    };

} // namespace payment_service
