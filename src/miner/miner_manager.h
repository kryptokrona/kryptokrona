// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <queue>

#include <sys/context_group.h>
#include <sys/event.h>

#include "blockchain_monitor.h"
#include "logging/logger_ref.h"
#include "miner.h"
#include "miner_event.h"
#include "mining_config.h"

namespace sys
{
    class Dispatcher;
}

namespace miner
{
    class MinerManager
    {
        public:
            MinerManager(
                sys::Dispatcher& dispatcher,
                const cryptonote::MiningConfig& config,
                const std::shared_ptr<httplib::Client> httpClient);

            void start();

        private:
            sys::ContextGroup m_contextGroup;
            cryptonote::MiningConfig m_config;
            cryptonote::Miner m_miner;
            BlockchainMonitor m_blockchainMonitor;

            sys::Event m_eventOccurred;
            std::queue<MinerEvent> m_events;
            bool isRunning;

            cryptonote::BlockTemplate m_minedBlock;

            uint64_t m_lastBlockTimestamp;

            std::shared_ptr<httplib::Client> m_httpClient = nullptr;

            void eventLoop();
            MinerEvent waitEvent();
            void pushEvent(MinerEvent&& event);
            void printHashRate();

            void startMining(const cryptonote::BlockMiningParameters& params);
            void stopMining();

            void startBlockchainMonitoring();
            void stopBlockchainMonitoring();

            bool submitBlock(const cryptonote::BlockTemplate& minedBlock);
            cryptonote::BlockMiningParameters requestMiningParameters();

            void adjustBlockTemplate(cryptonote::BlockTemplate& blockTemplate) const;
    };
}
