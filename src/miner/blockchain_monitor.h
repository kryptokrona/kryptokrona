// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "crypto_types.h"

#include "httplib.h"

#include <optional>

#include <sys/context_group.h>
#include <sys/dispatcher.h>
#include <sys/event.h>

class BlockchainMonitor
{
    public:
        BlockchainMonitor(
            sys::Dispatcher& dispatcher,
            const size_t pollingInterval,
            const std::shared_ptr<httplib::Client> httpClient);

        void waitBlockchainUpdate();
        void stop();

    private:
        sys::Dispatcher& m_dispatcher;
        size_t m_pollingInterval;
        bool m_stopped;
        sys::ContextGroup m_sleepingContext;

        std::optional<crypto::Hash> requestLastBlockHash();

        std::shared_ptr<httplib::Client> m_httpClient = nullptr;
};
