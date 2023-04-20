// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <chrono>
#include <vector>

#include <syst/context_group.h>
#include <syst/dispatcher.h>
#include <syst/event.h>
#include <syst/tcp_connection.h>
#include <syst/timer.h>

#include <config/cryptonote_config.h>
#include "levin_protocol.h"
#include "p2p_interfaces.h"
#include "p2p_protocol_definitions.h"
#include "p2p_protocol_types.h"

namespace cryptonote
{

    class P2pContext
    {
    public:
        using Clock = std::chrono::steady_clock;
        using TimePoint = Clock::time_point;

        struct Message : P2pMessage
        {
            enum Type
            {
                NOTIFY,
                REQUEST,
                REPLY
            };

            Type messageType;
            uint32_t returnCode;

            Message(P2pMessage &&msg, Type messageType, uint32_t returnCode = 0);
            size_t size() const;
        };

        P2pContext(syst::Dispatcher &dispatcher, syst::TcpConnection &&conn,
                   bool isIncoming, const NetworkAddress &remoteAddress, std::chrono::nanoseconds timedSyncInterval, const CORE_SYNC_DATA &timedSyncData);
        ~P2pContext();

        uint64_t getPeerId() const;
        uint16_t getPeerPort() const;
        const NetworkAddress &getRemoteAddress() const;
        bool isIncoming() const;

        void setPeerInfo(uint8_t protocolVersion, uint64_t id, uint16_t port);
        bool readCommand(LevinProtocol::Command &cmd);
        void writeMessage(const Message &msg);

        void start();
        void stop();

    private:
        uint8_t version = 0;
        const bool incoming;
        const NetworkAddress remoteAddress;
        uint64_t peerId = 0;
        uint16_t peerPort = 0;

        syst::Dispatcher &dispatcher;
        syst::ContextGroup contextGroup;
        const TimePoint timeStarted;
        bool stopped = false;
        TimePoint lastReadTime;

        // timed sync info
        const std::chrono::nanoseconds timedSyncInterval;
        const CORE_SYNC_DATA &timedSyncData;
        syst::Timer timedSyncTimer;
        syst::Event timedSyncFinished;

        syst::TcpConnection connection;
        syst::Event writeEvent;
        syst::Event readEvent;

        void timedSyncLoop();
    };

    P2pContext::Message makeReply(uint32_t command, const BinaryArray &data, uint32_t returnCode);
    P2pContext::Message makeRequest(uint32_t command, const BinaryArray &data);

    std::ostream &operator<<(std::ostream &s, const P2pContext &conn);

}
