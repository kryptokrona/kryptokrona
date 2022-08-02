// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "crypto_types.h"

#include "httplib.h"

#include <optional>

#include <system/context_group.h>
#include <system/dispatcher.h>
#include <system/event.h>

class BlockchainMonitor
{
    public:
        BlockchainMonitor(
            system::Dispatcher& dispatcher,
            const size_t pollingInterval,
            const std::shared_ptr<httplib::Client> httpClient);

        void waitBlockchainUpdate();
        void stop();

    private:
        system::Dispatcher& m_dispatcher;
        size_t m_pollingInterval;
        bool m_stopped;
        system::ContextGroup m_sleepingContext;

        std::optional<crypto::Hash> requestLastBlockHash();

        std::shared_ptr<httplib::Client> m_httpClient = nullptr;
};
