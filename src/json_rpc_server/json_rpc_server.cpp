// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "json_rpc_server.h"

#include <fstream>
#include <future>
#include <system_error>
#include <memory>
#include <sstream>
#include "http/http_parser_error_codes.h"

#include <syst/tcp_connection.h>
#include <syst/tcp_listener.h>
#include <syst/tcp_stream.h>
#include <syst/ipv4_address.h>
#include "http/http_parser.h"
#include "http/http_response.h"
#include "rpc/json_rpc.h"

#include "common/json_value.h"
#include "serialization/json_input_value_serializer.h"
#include "serialization/json_output_stream_serializer.h"

namespace cryptonote
{

    JsonRpcServer::JsonRpcServer(syst::Dispatcher &sys, syst::Event &stopEvent, std::shared_ptr<logging::ILogger> loggerGroup, payment_service::ConfigurationManager &config) : HttpServer(sys, loggerGroup),
                                                                                                                                                                                stopEvent(stopEvent),
                                                                                                                                                                                logger(loggerGroup, "JsonRpcServer"),
                                                                                                                                                                                config(config)
    {
    }

    void JsonRpcServer::start(const std::string &bindAddress, uint16_t bindPort)
    {
        HttpServer::start(bindAddress, bindPort);
        stopEvent.wait();
        HttpServer::stop();
    }

    void JsonRpcServer::processRequest(const cryptonote::HttpRequest &req, cryptonote::HttpResponse &resp)
    {
        try
        {
            logger(logging::TRACE) << "HTTP request came: \n"
                                   << req;

            if (req.getUrl() == "/json_rpc")
            {
                std::istringstream jsonInputStream(req.getBody());
                common::JsonValue jsonRpcRequest;
                common::JsonValue jsonRpcResponse(common::JsonValue::OBJECT);

                try
                {
                    jsonInputStream >> jsonRpcRequest;
                }
                catch (std::runtime_error &)
                {
                    logger(logging::DEBUGGING) << "Couldn't parse request: \"" << req.getBody() << "\"";
                    makeJsonParsingErrorResponse(jsonRpcResponse);
                    resp.setStatus(cryptonote::HttpResponse::STATUS_200);
                    resp.setBody(jsonRpcResponse.toString());
                    return;
                }

                processJsonRpcRequest(jsonRpcRequest, jsonRpcResponse);

                std::ostringstream jsonOutputStream;
                jsonOutputStream << jsonRpcResponse;

                if (config.serviceConfig.corsHeader != "")
                {
                    resp.addHeader("Access-Control-Allow-Origin", config.serviceConfig.corsHeader);
                }

                resp.setStatus(cryptonote::HttpResponse::STATUS_200);
                resp.setBody(jsonOutputStream.str());
            }
            else
            {
                logger(logging::WARNING) << "Requested url \"" << req.getUrl() << "\" is not found";
                resp.setStatus(cryptonote::HttpResponse::STATUS_404);
                return;
            }
        }
        catch (std::exception &e)
        {
            logger(logging::WARNING) << "Error while processing http request: " << e.what();
            resp.setStatus(cryptonote::HttpResponse::STATUS_500);
        }
    }

    void JsonRpcServer::prepareJsonResponse(const common::JsonValue &req, common::JsonValue &resp)
    {
        using common::JsonValue;

        if (req.contains("id"))
        {
            resp.insert("id", req("id"));
        }

        resp.insert("jsonrpc", "2.0");
    }

    void JsonRpcServer::makeErrorResponse(const std::error_code &ec, common::JsonValue &resp)
    {
        using common::JsonValue;

        JsonValue error(JsonValue::OBJECT);

        JsonValue code;
        code = static_cast<int64_t>(cryptonote::json_rpc::errParseError); // Application specific error code

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

    void JsonRpcServer::makeGenericErrorReponse(common::JsonValue &resp, const char *what, int errorCode)
    {
        using common::JsonValue;

        JsonValue error(JsonValue::OBJECT);

        JsonValue code;
        code = static_cast<int64_t>(errorCode);

        std::string msg;
        if (what)
        {
            msg = what;
        }
        else
        {
            msg = "Unknown application error";
        }

        JsonValue message;
        message = msg;

        error.insert("code", code);
        error.insert("message", message);

        resp.insert("error", error);
    }

    void JsonRpcServer::makeMethodNotFoundResponse(common::JsonValue &resp)
    {
        using common::JsonValue;

        JsonValue error(JsonValue::OBJECT);

        JsonValue code;
        code = static_cast<int64_t>(cryptonote::json_rpc::errMethodNotFound); // ambigous declaration of JsonValue::operator= (between int and JsonValue)

        JsonValue message;
        message = "Method not found";

        error.insert("code", code);
        error.insert("message", message);

        resp.insert("error", error);
    }

    void JsonRpcServer::makeInvalidPasswordResponse(common::JsonValue &resp)
    {
        using common::JsonValue;

        JsonValue error(JsonValue::OBJECT);

        JsonValue code;
        code = static_cast<int64_t>(cryptonote::json_rpc::errInvalidPassword);

        JsonValue message;
        message = "Invalid or no rpc password";

        error.insert("code", code);
        error.insert("message", message);

        resp.insert("error", error);
    }

    void JsonRpcServer::fillJsonResponse(const common::JsonValue &v, common::JsonValue &resp)
    {
        resp.insert("result", v);
    }

    void JsonRpcServer::makeJsonParsingErrorResponse(common::JsonValue &resp)
    {
        using common::JsonValue;

        resp = JsonValue(JsonValue::OBJECT);
        resp.insert("jsonrpc", "2.0");
        resp.insert("id", nullptr);

        JsonValue error(JsonValue::OBJECT);
        JsonValue code;
        code = static_cast<int64_t>(cryptonote::json_rpc::errParseError); // ambigous declaration of JsonValue::operator= (between int and JsonValue)

        JsonValue message = "Parse error";

        error.insert("code", code);
        error.insert("message", message);

        resp.insert("error", error);
    }

}
