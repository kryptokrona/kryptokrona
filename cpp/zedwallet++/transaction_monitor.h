// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <wallet_backend/wallet_backend.h>
#include <wallet_backend/thread_safe_queue.h>

class TransactionMonitor
{
public:
    TransactionMonitor(
        const std::shared_ptr<WalletBackend> walletBackend) : m_walletBackend(walletBackend),
                                                              m_shouldStop(false) {}

    void start();

    void stop();

    std::shared_ptr<std::mutex> getMutex() const;

private:
    std::atomic<bool> m_shouldStop;

    std::shared_ptr<WalletBackend> m_walletBackend;

    ThreadSafeQueue<wallet_types::Transaction> m_queuedTransactions;

    std::shared_ptr<std::mutex> m_mutex = std::make_shared<std::mutex>();
};
