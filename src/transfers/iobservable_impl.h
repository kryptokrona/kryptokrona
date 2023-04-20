// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "common/observer_manager.h"

namespace cryptonote
{

    template <typename Observer, typename Base>
    class IObservableImpl : public Base
    {
    public:
        virtual void addObserver(Observer *observer) override
        {
            m_observerManager.add(observer);
        }

        virtual void removeObserver(Observer *observer) override
        {
            m_observerManager.remove(observer);
        }

    protected:
        tools::ObserverManager<Observer> m_observerManager;
    };

}
