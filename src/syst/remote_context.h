// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cassert>

#include <future>

#include <syst/dispatcher.h>
#include <syst/event.h>
#include <syst/interrupted_exception.h>

namespace syst
{

    template <class T = void>
    class RemoteContext
    {
    public:
        // Start a thread, execute operation in it, continue execution of current context.
        RemoteContext(Dispatcher &d, std::function<T()> &&operation)
            : dispatcher(d), event(d), procedure(std::move(operation)), future(std::async(std::launch::async, [this]
                                                                                          { return asyncProcedure(); })),
              interrupted(false)
        {
        }

        // Run other task on dispatcher until future is ready, then return lambda's result, or rethrow exception. UB if called more than once.
        T get() const
        {
            wait();
            return future.get();
        }

        // Run other task on dispatcher until future is ready.
        void wait() const
        {
            while (!event.get())
            {
                try
                {
                    event.wait();
                }
                catch (InterruptedException &)
                {
                    interrupted = true;
                }
            }

            if (interrupted)
            {
                dispatcher.interrupt();
            }
        }

        // Wait future to complete.
        ~RemoteContext()
        {
            try
            {
                wait();
            }
            catch (...)
            {
            }

            try
            {
                // windows future implementation doesn't wait for completion on destruction
                if (future.valid())
                {
                    future.wait();
                }
            }
            catch (...)
            {
            }
        }

    private:
        struct NotifyOnDestruction
        {
            NotifyOnDestruction(Dispatcher &d, Event &e) : dispatcher(d), event(e)
            {
            }

            ~NotifyOnDestruction()
            {
                // make a local copy; event reference will be dead when function is called
                auto localEvent = &event;
                // die if this throws...
                dispatcher.remoteSpawn([=]
                                       { localEvent->set(); });
            }

            Dispatcher &dispatcher;
            Event &event;
        };

        // This function is executed in future object
        T asyncProcedure()
        {
            NotifyOnDestruction guard(dispatcher, event);
            assert(procedure != nullptr);
            return procedure();
        }

        Dispatcher &dispatcher;
        mutable Event event;
        std::function<T()> procedure;
        mutable std::future<T> future;
        mutable bool interrupted;
    };

}
