// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <list>
#include <ostream>
#include <unordered_set>
#include <optional>

#include <boost/uuid/uuid.hpp>
#include "common/string_tools.h"
#include "crypto/hash.h"

#include "p2p/pending_lite_block.h"

namespace cryptonote
{

    struct CryptoNoteConnectionContext
    {
        uint8_t version;
        boost::uuids::uuid m_connection_id;
        uint32_t m_remote_ip = 0;
        uint32_t m_remote_port = 0;
        bool m_is_income = false;
        time_t m_started = 0;

        enum state
        {
            state_befor_handshake = 0, // default state
            state_synchronizing,
            state_idle,
            state_normal,
            state_sync_required,
            state_pool_sync_required,
            state_shutdown
        };

        state m_state = state_befor_handshake;
        std::optional<PendingLiteBlock> m_pending_lite_block;
        std::list<crypto::Hash> m_needed_objects;
        std::unordered_set<crypto::Hash> m_requested_objects;
        uint32_t m_remote_blockchain_height = 0;
        uint32_t m_last_response_height = 0;
    };

    inline std::string get_protocol_state_string(CryptoNoteConnectionContext::state s)
    {
        switch (s)
        {
        case CryptoNoteConnectionContext::state_befor_handshake:
            return "state_befor_handshake";
        case CryptoNoteConnectionContext::state_synchronizing:
            return "state_synchronizing";
        case CryptoNoteConnectionContext::state_idle:
            return "state_idle";
        case CryptoNoteConnectionContext::state_normal:
            return "state_normal";
        case CryptoNoteConnectionContext::state_sync_required:
            return "state_sync_required";
        case CryptoNoteConnectionContext::state_pool_sync_required:
            return "state_pool_sync_required";
        case CryptoNoteConnectionContext::state_shutdown:
            return "state_shutdown";
        default:
            return "unknown";
        }
    }

}

namespace std
{
    inline std::ostream &operator<<(std::ostream &s, const cryptonote::CryptoNoteConnectionContext &context)
    {
        return s << "[" << common::ipAddressToString(context.m_remote_ip) << ":" << context.m_remote_port << (context.m_is_income ? " INC" : " OUT") << "] ";
    }
}
