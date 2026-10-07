// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <memory>
#include <unordered_set>

#include <http/http_request.h>
#include <http/http_response.h>

#include <syst/context_group.h>
#include <syst/dispatcher.h>
#include <syst/tcp_listener.h>
#include <syst/tcp_connection.h>
#include <syst/event.h>

#include <logging/logger_ref.h>

#include "request_concurrency_limiter.h"

namespace cryptonote
{

    class HttpServer
    {

    public:
        HttpServer(syst::Dispatcher &dispatcher, std::shared_ptr<logging::ILogger> log);

        void start(const std::string &address, uint16_t port);
        void stop();

        virtual void processRequest(const HttpRequest &request, HttpResponse &response) = 0;

    protected:
        syst::Dispatcher &m_dispatcher;

        // Whether acceptLoop should run processRequest on a worker thread
        // (syst::RemoteContext) instead of inline on the cooperative dispatcher.
        // Off by default: the base behaviour is to process inline. Only servers
        // whose handlers are self-contained (e.g. the daemon's Core/DB reads,
        // whose cross-thread work is posted back via Dispatcher::remoteSpawn)
        // may opt in. It MUST stay off for servers whose handlers drive
        // dispatcher-bound state on this thread -- notably the wallet service's
        // JsonRpcServer, which operates a WalletService/WalletGreen bound to
        // this same dispatcher; running those off-dispatcher is undefined
        // behaviour and crashes the process.
        virtual bool offloadRequestProcessing() const { return false; }

        // Maximum number of offloaded request handlers allowed to run at once.
        // 0 means unlimited (the historical behaviour). A server that offloads
        // under hostile load (the daemon RpcServer) overrides this to bound the
        // worker-thread spawn rate and the number of concurrent Core readers, so
        // an RPC flood cannot starve the shared dispatcher's p2p/sync work.
        // Only consulted when offloadRequestProcessing() is true.
        virtual size_t concurrentRequestLimit() const { return 0; }

    private:
        void acceptLoop();
        void connectionHandler(syst::TcpConnection &&conn);

        syst::ContextGroup workingContextGroup;
        logging::LoggerRef logger;
        syst::TcpListener m_listener;
        std::unordered_set<syst::TcpConnection *> m_connections;

        // Bounds concurrent offloaded handlers; created in start() only when
        // offloadRequestProcessing() && concurrentRequestLimit() > 0.
        std::unique_ptr<RequestConcurrencyLimiter> m_requestLimiter;
    };

}
