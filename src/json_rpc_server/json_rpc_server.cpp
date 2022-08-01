// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "json_rpc_server.h"

#include <fstream>
#include <future>
#include <system_error>
#include <memory>
#include <sstream>
#include "http/http_parser_error_codes.h"

#include <system/tcp_connection.h>
#include <system/tcp_listener.h>
#include <system/tcp_stream.h>
#include <system/ipv4_address.h>
#include "http/http_parser.h"
#include "http/http_response.h"
#include "rpc/json_rpc.h"

#include "common/json_value.h"
#include "serialization/json_input_value_serializer.h"
#include "serialization/json_output_stream_serializer.h"

namespace cryptonote
{
    JsonRpcServer::JsonRpcServer(System::Dispatcher& sys, System::Event& stopEvent, std::shared_ptr<Logging::ILogger> loggerGroup, PaymentService::ConfigurationManager& config) :
      HttpServer(sys, loggerGroup),
      stopEvent(stopEvent),
      logger(loggerGroup, "json_rpc_server"),
      config(config)
    {
    }

    void JsonRpcServer::start(const std::string& bindAddress, uint16_t bindPort) {
      HttpServer::start(bindAddress, bindPort);
      stopEvent.wait();
      HttpServer::stop();
    }

    void JsonRpcServer::processRequest(const CryptoNote::HttpRequest& req, CryptoNote::HttpResponse& resp) {
      try {
        logger(Logging::TRACE) << "http request came: \n" << req;

        if (req.getUrl() == "/json_rpc") {
          std::istringstream jsonInputStream(req.getBody());
          Common::JsonValue jsonRpcRequest;
          Common::JsonValue jsonRpcResponse(Common::JsonValue::OBJECT);

          try {
            jsonInputStream >> jsonRpcRequest;
          } catch (std::runtime_error&) {
            logger(Logging::DEBUGGING) << "Couldn't parse request: \"" << req.getBody() << "\"";
            makeJsonParsingErrorResponse(jsonRpcResponse);
            resp.setStatus(CryptoNote::HttpResponse::STATUS_200);
            resp.setBody(jsonRpcResponse.toString());
            return;
          }

          processJsonRpcRequest(jsonRpcRequest, jsonRpcResponse);

          std::ostringstream jsonOutputStream;
          jsonOutputStream << jsonRpcResponse;

          if (config.serviceConfig.corsHeader != "") {
            resp.addHeader("Access-Control-Allow-Origin", config.serviceConfig.corsHeader);
          }

          resp.setStatus(CryptoNote::HttpResponse::STATUS_200);
          resp.setBody(jsonOutputStream.str());

        } else {
          logger(Logging::WARNING) << "Requested url \"" << req.getUrl() << "\" is not found";
          resp.setStatus(CryptoNote::HttpResponse::STATUS_404);
          return;
        }
      } catch (std::exception& e) {
        logger(Logging::WARNING) << "Error while processing http request: " << e.what();
        resp.setStatus(CryptoNote::HttpResponse::STATUS_500);
      }
    }

    void JsonRpcServer::prepareJsonResponse(const Common::JsonValue& req, Common::JsonValue& resp) {
      using Common::JsonValue;

      if (req.contains("id")) {
        resp.insert("id", req("id"));
      }

      resp.insert("jsonrpc", "2.0");
    }

    void JsonRpcServer::makeErrorResponse(const std::error_code& ec, Common::JsonValue& resp) {
      using Common::JsonValue;

      JsonValue error(JsonValue::OBJECT);

      JsonValue code;
      code = static_cast<int64_t>(CryptoNote::JsonRpc::errParseError); //Application specific error code

      JsonValue message;
      message = ec.message();

      JsonValue data(JsonValue::OBJECT);
      JsonValue appCode;
      appCode = static_cast<int64_t>(ec.value());
      data.insert("application_code", appCode);

      error.insert("code", code);
      error.insert("message", message);
      error.insert("data", data);

      resp.insert("error", error);
    }

    void JsonRpcServer::makeGenericErrorReponse(Common::JsonValue& resp, const char* what, int errorCode) {
      using Common::JsonValue;

      JsonValue error(JsonValue::OBJECT);

      JsonValue code;
      code = static_cast<int64_t>(errorCode);

      std::string msg;
      if (what) {
        msg = what;
      } else {
        msg = "Unknown application error";
      }

      JsonValue message;
      message = msg;

      error.insert("code", code);
      error.insert("message", message);

      resp.insert("error", error);

    }

    void JsonRpcServer::makeMethodNotFoundResponse(Common::JsonValue& resp) {
      using Common::JsonValue;

      JsonValue error(JsonValue::OBJECT);

      JsonValue code;
      code = static_cast<int64_t>(CryptoNote::JsonRpc::errMethodNotFound); //ambigous declaration of JsonValue::operator= (between int and JsonValue)

      JsonValue message;
      message = "Method not found";

      error.insert("code", code);
      error.insert("message", message);

      resp.insert("error", error);
    }

    void JsonRpcServer::makeInvalidPasswordResponse(Common::JsonValue& resp) {
      using Common::JsonValue;

      JsonValue error(JsonValue::OBJECT);

      JsonValue code;
      code = static_cast<int64_t>(CryptoNote::JsonRpc::errInvalidPassword);

      JsonValue message;
      message = "Invalid or no rpc password";

      error.insert("code", code);
      error.insert("message", message);

      resp.insert("error", error);
    }

    void JsonRpcServer::fillJsonResponse(const Common::JsonValue& v, Common::JsonValue& resp) {
      resp.insert("result", v);
    }

    void JsonRpcServer::makeJsonParsingErrorResponse(Common::JsonValue& resp) {
      using Common::JsonValue;

      resp = JsonValue(JsonValue::OBJECT);
      resp.insert("jsonrpc", "2.0");
      resp.insert("id", nullptr);

      JsonValue error(JsonValue::OBJECT);
      JsonValue code;
      code = static_cast<int64_t>(CryptoNote::JsonRpc::errParseError); //ambigous declaration of JsonValue::operator= (between int and JsonValue)

      JsonValue message = "Parse error";

      error.insert("code", code);
      error.insert("message", message);

      resp.insert("error", error);
    }
}
