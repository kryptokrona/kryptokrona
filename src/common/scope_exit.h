// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <functional>

namespace tools
{

    class ScopeExit
    {
    public:
        ScopeExit(std::function<void()> &&handler);
        ~ScopeExit();

        ScopeExit(const ScopeExit &) = delete;
        ScopeExit(ScopeExit &&) = delete;
        ScopeExit &operator=(const ScopeExit &) = delete;
        ScopeExit &operator=(ScopeExit &&) = delete;

        void cancel();
        void resume();

    private:
        std::function<void()> m_handler;
        bool m_cancelled;
    };

}
