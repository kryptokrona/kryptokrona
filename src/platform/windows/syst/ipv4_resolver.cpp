// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "ipv4_resolver.h"
#include <cassert>
#include <random>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <ws2tcpip.h>
#include <syst/dispatcher.h>
#include <syst/error_message.h>
#include <syst/interrupted_exception.h>
#include <syst/ipv4_address.h>

namespace syst
{

    Ipv4Resolver::Ipv4Resolver() : dispatcher(nullptr)
    {
    }

    Ipv4Resolver::Ipv4Resolver(Dispatcher &dispatcher) : dispatcher(&dispatcher)
    {
    }

    Ipv4Resolver::Ipv4Resolver(Ipv4Resolver &&other) : dispatcher(other.dispatcher)
    {
        if (dispatcher != nullptr)
        {
            other.dispatcher = nullptr;
        }
    }

    Ipv4Resolver::~Ipv4Resolver()
    {
    }

    Ipv4Resolver &Ipv4Resolver::operator=(Ipv4Resolver &&other)
    {
        dispatcher = other.dispatcher;
        if (dispatcher != nullptr)
        {
            other.dispatcher = nullptr;
        }

        return *this;
    }

    Ipv4Address Ipv4Resolver::resolve(const std::string &host)
    {
        assert(dispatcher != nullptr);
        if (dispatcher->interrupted())
        {
            throw InterruptedException();
        }

        addrinfo hints = {0, AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};
        addrinfo *addressInfos;
        int result = getaddrinfo(host.c_str(), NULL, &hints, &addressInfos);
        if (result != 0)
        {
            throw std::runtime_error("Ipv4Resolver::resolve, getaddrinfo failed, " + errorMessage(result));
        }

        size_t count = 0;
        for (addrinfo *addressInfo = addressInfos; addressInfo != nullptr; addressInfo = addressInfo->ai_next)
        {
            ++count;
        }

        std::mt19937 generator{std::random_device()()};
        size_t index = std::uniform_int_distribution<size_t>(0, count - 1)(generator);
        addrinfo *addressInfo = addressInfos;
        for (size_t i = 0; i < index; ++i)
        {
            addressInfo = addressInfo->ai_next;
        }

        Ipv4Address address(ntohl(reinterpret_cast<sockaddr_in *>(addressInfo->ai_addr)->sin_addr.S_un.S_addr));
        freeaddrinfo(addressInfo);
        return address;
    }

}
