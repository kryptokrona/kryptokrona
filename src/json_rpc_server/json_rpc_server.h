// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <system_error>

#include <syst/dispatcher.h>
#include <syst/event.h>
#include "logging/ilogger.h"
#include "logging/logger_ref.h"
#include "rpc/http_server.h"
#include "wallet_service/configuration_manager.h"

namespace cryptonote
{
    class HttpResponse;
    class HttpRequest;
}

namespace common
{
    class JsonValue;
}

namespace syst
{
    class TcpConnection;
}

namespace cryptonote
{

    class JsonRpcServer : HttpServer
    {
    public:
        JsonRpcServer(syst::Dispatcher &sys, syst::Event &stopEvent, std::shared_ptr<logging::ILogger> loggerGroup, payment_service::ConfigurationManager &config);
        JsonRpcServer(const JsonRpcServer &) = delete;

        void start(const std::string &bindAddress, uint16_t bindPort);

    protected:
        static void makeErrorResponse(const std::error_code &ec, common::JsonValue &resp);
        static void makeMethodNotFoundResponse(common::JsonValue &resp);
        static void makeInvalidPasswordResponse(common::JsonValue &resp);
        static void makeGenericErrorReponse(common::JsonValue &resp, const char *what, int errorCode = -32001);
        static void fillJsonResponse(const common::JsonValue &v, common::JsonValue &resp);
        static void prepareJsonResponse(const common::JsonValue &req, common::JsonValue &resp);
        static void makeJsonParsingErrorResponse(common::JsonValue &resp);

        virtual void processJsonRpcRequest(const common::JsonValue &req, common::JsonValue &resp) = 0;
        payment_service::ConfigurationManager &config;

    private:
        // HttpServer
        virtual void processRequest(const cryptonote::HttpRequest &request, cryptonote::HttpResponse &response) override;

        syst::Event &stopEvent;
        logging::LoggerRef logger;
    };

} // namespace cryptonote
