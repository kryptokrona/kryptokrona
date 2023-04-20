// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <chrono>

namespace syst
{

    class Dispatcher;

    class Timer
    {
    public:
        Timer();
        explicit Timer(Dispatcher &dispatcher);
        Timer(const Timer &) = delete;
        Timer(Timer &&other);
        ~Timer();
        Timer &operator=(const Timer &) = delete;
        Timer &operator=(Timer &&other);
        void sleep(std::chrono::nanoseconds duration);

    private:
        Dispatcher *dispatcher;
        void *context;
    };

}
