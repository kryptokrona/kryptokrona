// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <system_error>

#include <sys/dispatcher.h>
#include <sys/event.h>
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

namespace sys
{
    class TcpConnection;
}

namespace cryptonote
{
    class JsonRpcServer : HttpServer {
    public:
      JsonRpcServer(sys::Dispatcher& sys, sys::Event& stopEvent, std::shared_ptr<logging::ILogger> loggerGroup, PaymentService::ConfigurationManager& config);
      JsonRpcServer(const JsonRpcServer&) = delete;

      void start(const std::string& bindAddress, uint16_t bindPort);

    protected:
      static void makeErrorResponse(const std::error_code& ec, common::JsonValue& resp);
      static void makeMethodNotFoundResponse(common::JsonValue& resp);
      static void makeInvalidPasswordResponse(common::JsonValue& resp);
      static void makeGenericErrorReponse(common::JsonValue& resp, const char* what, int errorCode = -32001);
      static void fillJsonResponse(const common::JsonValue& v, common::JsonValue& resp);
      static void prepareJsonResponse(const common::JsonValue& req, common::JsonValue& resp);
      static void makeJsonParsingErrorResponse(common::JsonValue& resp);

      virtual void processJsonRpcRequest(const common::JsonValue& req, common::JsonValue& resp) = 0;
      PaymentService::ConfigurationManager& config;

    private:
      // HttpServer
      virtual void processRequest(const cryptonote::HttpRequest& request, cryptonote::HttpResponse& response) override;

      sys::Event& stopEvent;
      logging::LoggerRef logger;
    };
}
