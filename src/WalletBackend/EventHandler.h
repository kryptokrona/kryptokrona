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
                /* Launch the function, and return instantly. This way we
                   can have multiple functions running at once.
                   
                   If you use std::cout in your function, you may experience
                   interleaved characters when printing. To resolve this,
                   consider using std::osyncstream.
                   Further reading: https://stackoverflow.com/q/14718124/8737306 */
                std::thread(m_function, args).detach();
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
};
