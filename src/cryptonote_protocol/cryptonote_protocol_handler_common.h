// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cryptonote_protocol/icryptonote_protocol_query.h>
#include <cryptonote.h>

#include <vector>

namespace cryptonote
{
    struct NOTIFY_NEW_BLOCK_request;

    /************************************************************************/
    /*                                                                      */
    /************************************************************************/
    struct ICryptoNoteProtocol
    {
        virtual void relayBlock(NOTIFY_NEW_BLOCK_request &arg) = 0;
        virtual void relayTransactions(const std::vector<BinaryArray> &transactions) = 0;
    };

    struct ICryptoNoteProtocolHandler : ICryptoNoteProtocol, public ICryptoNoteProtocolQuery
    {
    };
}
