// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <syst/context_group.h>
#include <syst/dispatcher.h>
#include <syst/timer.h>

namespace syst
{

    template <typename T>
    class OperationTimeout
    {
    public:
        OperationTimeout(Dispatcher &dispatcher, T &object, std::chrono::nanoseconds timeout) : object(object), timerContext(dispatcher), timeoutTimer(dispatcher)
        {
            timerContext.spawn([this, timeout]()
                               {
      try {
        timeoutTimer.sleep(timeout);
        timerContext.interrupt();
      } catch (...) {
      } });
        }

        ~OperationTimeout()
        {
            timerContext.interrupt();
            timerContext.wait();
        }

    private:
        T &object;
        ContextGroup timerContext;
        Timer timeoutTimer;
    };

}
