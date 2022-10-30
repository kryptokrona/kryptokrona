// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once 

#include <unordered_set>

#include <http/http_request.h>
#include <http/http_response.h>

#include <system/context_group.h>
#include <system/dispatcher.h>
#include <system/tcp_listener.h>
#include <system/tcp_connection.h>
#include <system/event.h>

#include <logging/logger_ref.h>

namespace cryptonote
{
    class HttpServer {

    public:

      HttpServer(System::Dispatcher& dispatcher, std::shared_ptr<Logging::ILogger> log);

      void start(const std::string& address, uint16_t port);
      void stop();

      virtual void processRequest(const HttpRequest& request, HttpResponse& response) = 0;

    protected:

      System::Dispatcher& m_dispatcher;

    private:

      void acceptLoop();
      void connectionHandler(System::TcpConnection&& conn);

      System::ContextGroup workingContextGroup;
      Logging::LoggerRef logger;
      System::TcpListener m_listener;
      std::unordered_set<System::TcpConnection*> m_connections;
    };
}
