// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <condition_variable>
#include <mutex>

namespace cryptonote
{
    // A shared/exclusive mutex with WRITER PREFERENCE.
    //
    // std::shared_mutex on glibc is reader-preferring: a thread waiting for the
    // exclusive lock only proceeds once the reader count reaches zero, and newly
    // arriving readers are allowed to jump ahead of it. Under sustained,
    // overlapping RPC read load (p2pool + wallets + block explorer all polling),
    // the reader count never hits zero, so the exclusive writer -- Core::addBlock,
    // the single funnel for applying new blocks -- can be starved indefinitely.
    // The daemon then stays fully responsive to RPC (readers) while its block
    // height silently stops advancing, until a restart clears it.
    //
    // This lock fixes that: once a writer is waiting, new readers block behind it,
    // so the writer acquires the lock as soon as the in-flight readers drain. In
    // steady state (a block every ~90s) the reader impact is a sub-millisecond
    // pause around each block; during catch-up, block application is (correctly)
    // prioritised over read RPCs. Interface-compatible with std::shared_lock and
    // std::unique_lock (lock()/unlock(), lock_shared()/unlock_shared(), try_*).
    class WriterPreferringSharedMutex
    {
    public:
        WriterPreferringSharedMutex() = default;
        WriterPreferringSharedMutex(const WriterPreferringSharedMutex &) = delete;
        WriterPreferringSharedMutex &operator=(const WriterPreferringSharedMutex &) = delete;

        // Exclusive (writer) ownership.
        void lock()
        {
            std::unique_lock<std::mutex> guard(m_mutex);
            ++m_waitingWriters;
            m_writerGate.wait(guard, [this] { return !m_activeWriter && m_activeReaders == 0; });
            --m_waitingWriters;
            m_activeWriter = true;
        }

        bool try_lock()
        {
            std::unique_lock<std::mutex> guard(m_mutex);
            if (m_activeWriter || m_activeReaders != 0)
            {
                return false;
            }
            m_activeWriter = true;
            return true;
        }

        void unlock()
        {
            std::unique_lock<std::mutex> guard(m_mutex);
            m_activeWriter = false;
            // Hand off to a waiting writer first (preference); otherwise release readers.
            if (m_waitingWriters > 0)
            {
                m_writerGate.notify_one();
            }
            else
            {
                m_readerGate.notify_all();
            }
        }

        // Shared (reader) ownership. Readers wait while a writer holds or is
        // waiting for the lock -- this is what gives writers preference.
        void lock_shared()
        {
            std::unique_lock<std::mutex> guard(m_mutex);
            m_readerGate.wait(guard, [this] { return !m_activeWriter && m_waitingWriters == 0; });
            ++m_activeReaders;
        }

        bool try_lock_shared()
        {
            std::unique_lock<std::mutex> guard(m_mutex);
            if (m_activeWriter || m_waitingWriters > 0)
            {
                return false;
            }
            ++m_activeReaders;
            return true;
        }

        void unlock_shared()
        {
            std::unique_lock<std::mutex> guard(m_mutex);
            --m_activeReaders;
            // The last reader out wakes a waiting writer, if any.
            if (m_activeReaders == 0 && m_waitingWriters > 0)
            {
                m_writerGate.notify_one();
            }
        }

    private:
        std::mutex m_mutex;
        std::condition_variable m_readerGate;
        std::condition_variable m_writerGate;
        int m_activeReaders = 0;
        int m_waitingWriters = 0;
        bool m_activeWriter = false;
    };
}
