// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <condition_variable>
#include <mutex>
#include <stdint.h>

namespace cryptonote
{

    class WalletAsyncContextCounter
    {
    public:
        WalletAsyncContextCounter() : m_asyncContexts(0) {}

        void addAsyncContext();
        void delAsyncContext();

        // returns true if contexts are finished before timeout
        void waitAsyncContextsFinish();

    private:
        uint32_t m_asyncContexts;
        std::condition_variable m_cv;
        std::mutex m_mutex;
    };

} // namespace cryptonote
