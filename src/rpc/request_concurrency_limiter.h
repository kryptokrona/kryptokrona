// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <cstddef>

#include <syst/dispatcher.h>
#include <syst/event.h>

namespace cryptonote
{
    // A cooperative counting semaphore for the RPC dispatcher.
    //
    // The daemon offloads every RPC handler onto its own worker thread
    // (HttpServer::offloadRequestProcessing + syst::RemoteContext, which does
    // std::async(std::launch::async) -- a NEW OS thread per request). Under an
    // RPC flood (p2pool miners + public callers hammering a public node) this
    // spawns unbounded threads and fills the Core read-lock with unbounded
    // concurrent readers, which starves the block-add writer and the p2p/sync
    // coroutines that share the single dispatcher -- the node stays answering
    // RPC while its height silently freezes.
    //
    // This limiter caps how many handlers run at once: a request acquires a slot
    // before its worker thread is spawned and releases it when the handler
    // returns. At the cap, further requests cooperatively yield (event.wait)
    // until a slot frees, applying backpressure instead of piling on threads.
    //
    // All operations run on the single dispatcher thread, so the counter needs
    // no atomics/locks: a context only yields inside event.wait(), never between
    // the count check and decrement.
    class RequestConcurrencyLimiter
    {
    public:
        RequestConcurrencyLimiter(syst::Dispatcher &dispatcher, size_t maxConcurrent)
            : m_available(maxConcurrent), m_event(dispatcher)
        {
        }

        RequestConcurrencyLimiter(const RequestConcurrencyLimiter &) = delete;
        RequestConcurrencyLimiter &operator=(const RequestConcurrencyLimiter &) = delete;

        void acquire()
        {
            while (m_available == 0)
            {
                m_event.clear();
                m_event.wait(); // yields this context until a slot is released
            }

            --m_available;
        }

        void release()
        {
            ++m_available;
            m_event.set(); // wake waiters to re-check (one will take the slot)
        }

    private:
        size_t m_available;
        syst::Event m_event;
    };

    // RAII slot: acquires on construction, releases on destruction (including
    // when the handler throws), so a slot is never leaked.
    class ConcurrencySlot
    {
    public:
        explicit ConcurrencySlot(RequestConcurrencyLimiter &limiter) : m_limiter(limiter)
        {
            m_limiter.acquire();
        }

        ~ConcurrencySlot()
        {
            m_limiter.release();
        }

        ConcurrencySlot(const ConcurrencySlot &) = delete;
        ConcurrencySlot &operator=(const ConcurrencySlot &) = delete;

    private:
        RequestConcurrencyLimiter &m_limiter;
    };
}
