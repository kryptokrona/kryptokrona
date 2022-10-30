// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <queue>

#include <system/context_group.h>
#include <system/event.h>

#include "blockchain_monitor.h"
#include "logging/logger_ref.h"
#include "miner.h"
#include "miner_event.h"
#include "mining_config.h"

namespace system
{
    class Dispatcher;
}

namespace miner
{
    class MinerManager
    {
        public:
            MinerManager(
                System::Dispatcher& dispatcher,
                const CryptoNote::MiningConfig& config,
                const std::shared_ptr<httplib::Client> httpClient);

            void start();

        private:
            System::ContextGroup m_contextGroup;
            CryptoNote::MiningConfig m_config;
            CryptoNote::Miner m_miner;
            BlockchainMonitor m_blockchainMonitor;

            System::Event m_eventOccurred;
            std::queue<MinerEvent> m_events;
            bool isRunning;

            CryptoNote::BlockTemplate m_minedBlock;

            uint64_t m_lastBlockTimestamp;

            std::shared_ptr<httplib::Client> m_httpClient = nullptr;

            void eventLoop();
            MinerEvent waitEvent();
            void pushEvent(MinerEvent&& event);
            void printHashRate();

            void startMining(const CryptoNote::BlockMiningParameters& params);
            void stopMining();

            void startBlockchainMonitoring();
            void stopBlockchainMonitoring();

            bool submitBlock(const CryptoNote::BlockTemplate& minedBlock);
            CryptoNote::BlockMiningParameters requestMiningParameters();

            void adjustBlockTemplate(CryptoNote::BlockTemplate& blockTemplate) const;
    };
}
