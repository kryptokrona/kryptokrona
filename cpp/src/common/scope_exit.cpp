// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "scope_exit.h"

namespace tools
{

    ScopeExit::ScopeExit(std::function<void()> &&handler) : m_handler(std::move(handler)),
                                                            m_cancelled(false)
    {
    }

    ScopeExit::~ScopeExit()
    {
        if (!m_cancelled)
        {
            m_handler();
        }
    }

    void ScopeExit::cancel()
    {
        m_cancelled = true;
    }

    void ScopeExit::resume()
    {
        m_cancelled = false;
    }

}
