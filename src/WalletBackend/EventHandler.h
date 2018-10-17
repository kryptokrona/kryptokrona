// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <functional>

template <typename T>
class Event
{
    public:
        void subscribe(std::function<void(T)> function)
        {
            m_function = function;
        }

        void unsubscribe()
        {
            m_function = {};
        }

        void fire(T args)
        {
            if (m_function)
            {
                m_function(args);
            }
        }

    private:
        std::function<void(T)> m_function;
};

class EventHandler
{
    public:
        Event<int> onSynced;

        Event<WalletTypes::Transaction> onTransaction;

    private:
};
