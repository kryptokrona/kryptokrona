// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "http_client.h"

#include <http/http_parser.h>
#include <syst/ipv4_resolver.h>
#include <syst/ipv4_address.h>
#include <syst/tcp_connector.h>

namespace cryptonote
{

    HttpClient::HttpClient(syst::Dispatcher &dispatcher, const std::string &address, uint16_t port) : m_dispatcher(dispatcher), m_address(address), m_port(port)
    {
    }

    HttpClient::~HttpClient()
    {
        if (m_connected)
        {
            disconnect();
        }
    }

    void HttpClient::request(const HttpRequest &req, HttpResponse &res)
    {
        std::scoped_lock lock(m_mutex);

        if (!m_connected)
        {
            connect();
        }

        try
        {
            std::iostream stream(m_streamBuf.get());
            HttpParser parser;
            stream << req;
            stream.flush();
            parser.receiveResponse(stream, res);
        }
        catch (const std::exception &)
        {
            disconnect();
            throw;
        }
    }

    void HttpClient::connect()
    {
        try
        {
            auto ipAddr = syst::Ipv4Resolver(m_dispatcher).resolve(m_address);
            m_connection = syst::TcpConnector(m_dispatcher).connect(ipAddr, m_port);
            m_streamBuf.reset(new syst::TcpStreambuf(m_connection));
            m_connected = true;
        }
        catch (const std::exception &e)
        {
            throw ConnectException(e.what());
        }
    }

    bool HttpClient::isConnected() const
    {
        return m_connected;
    }

    void HttpClient::disconnect()
    {
        m_streamBuf.reset();
        try
        {
            m_connection.write(nullptr, 0); // Socket shutdown.
        }
        catch (std::exception &)
        {
            // Ignoring possible exception.
        }

        try
        {
            m_connection = syst::TcpConnection();
        }
        catch (std::exception &)
        {
            // Ignoring possible exception.
        }

        m_connected = false;
    }

    ConnectException::ConnectException(const std::string &whatArg) : std::runtime_error(whatArg.c_str())
    {
    }

}
