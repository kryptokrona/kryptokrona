// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <queue>

#include "ip2p_node_internal.h"
#include "levin_protocol.h"
#include "p2p_context_owner.h"
#include "p2p_interfaces.h"

namespace cryptonote
{

    class P2pContext;
    class P2pNode;

    class P2pConnectionProxy : public IP2pConnection
    {
    public:
        P2pConnectionProxy(P2pContextOwner &&ctx, IP2pNodeInternal &node);
        ~P2pConnectionProxy();

        bool processIncomingHandshake();

        // IP2pConnection
        virtual void read(P2pMessage &message) override;
        virtual void write(const P2pMessage &message) override;
        virtual void stop() override;

    private:
        void writeHandshake(const P2pMessage &message);
        void handleHandshakeRequest(const LevinProtocol::Command &cmd);
        void handleHandshakeResponse(const LevinProtocol::Command &cmd, P2pMessage &message);
        void handleTimedSync(const LevinProtocol::Command &cmd);

        std::queue<P2pMessage> m_readQueue;
        P2pContextOwner m_contextOwner;
        P2pContext &m_context;
        IP2pNodeInternal &m_node;
    };

}
