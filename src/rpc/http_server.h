// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <unordered_set>

#include <http/http_request.h>
#include <http/http_response.h>

#include <syst/context_group.h>
#include <syst/dispatcher.h>
#include <syst/tcp_listener.h>
#include <syst/tcp_connection.h>
#include <syst/event.h>

#include <logging/logger_ref.h>

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

    private:
        void acceptLoop();
        void connectionHandler(syst::TcpConnection &&conn);

        syst::ContextGroup workingContextGroup;
        logging::LoggerRef logger;
        syst::TcpListener m_listener;
        std::unordered_set<syst::TcpConnection *> m_connections;
    };

}
