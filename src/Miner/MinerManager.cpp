// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "MinerManager.h"

#include <System/EventLock.h>
#include <System/InterruptedException.h>
#include <System/Timer.h>
#include <thread>
#include <chrono>

#include "Common/FormatTools.h"
#include "Common/StringTools.h"
#include <config/CryptoNoteConfig.h>
#include "CryptoNoteCore/CachedBlock.h"
#include "CryptoNoteCore/CryptoNoteTools.h"
#include "CryptoNoteCore/CryptoNoteFormatUtils.h"
#include "CryptoNoteCore/TransactionExtra.h"
#include "Rpc/HttpClient.h"
#include "Rpc/CoreRpcServerCommandsDefinitions.h"
#include "Rpc/JsonRpc.h"

#include <zedwallet++/ColouredMsg.h>

using namespace CryptoNote;

namespace Miner {

namespace {

MinerEvent BlockMinedEvent()
{
    MinerEvent event;
    event.type = MinerEventType::BLOCK_MINED;
    return event;
}

MinerEvent BlockchainUpdatedEvent()
{
    MinerEvent event;
    event.type = MinerEventType::BLOCKCHAIN_UPDATED;
    return event;
}

void adjustMergeMiningTag(BlockTemplate& blockTemplate)
{
    CachedBlock cachedBlock(blockTemplate);

    if (blockTemplate.majorVersion >= BLOCK_MAJOR_VERSION_2)
    {
        CryptoNote::TransactionExtraMergeMiningTag mmTag;
        mmTag.depth = 0;
        mmTag.merkleRoot = cachedBlock.getAuxiliaryBlockHeaderHash();

        blockTemplate.parentBlock.baseTransaction.extra.clear();
        if (!CryptoNote::appendMergeMiningTagToExtra(blockTemplate.parentBlock.baseTransaction.extra, mmTag))
        {
            throw std::runtime_error("Couldn't append merge mining tag");
        }
    }
}

} // namespace

MinerManager::MinerManager(System::Dispatcher& dispatcher, const CryptoNote::MiningConfig& config) :
    m_dispatcher(dispatcher),
    m_contextGroup(dispatcher),
    m_config(config),
    m_miner(dispatcher),
    m_blockchainMonitor(dispatcher, m_config.daemonHost, m_config.daemonPort, m_config.scanPeriod),
    m_eventOccurred(dispatcher),
    m_httpEvent(dispatcher),
    m_lastBlockTimestamp(0)
{
    m_httpEvent.set();
}

void MinerManager::start()
{
    BlockMiningParameters params = requestMiningParameters(m_dispatcher, m_config.daemonHost, m_config.daemonPort, m_config.miningAddress);
    adjustBlockTemplate(params.blockTemplate);

    isRunning = true;

    startBlockchainMonitoring();
    std::thread reporter(std::bind(&MinerManager::printHashRate, this));
    startMining(params);

    eventLoop();
    isRunning = false;
}

void MinerManager::printHashRate()
{
    uint64_t last_hash_count = m_miner.getHashCount();

    while (isRunning)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));

        uint64_t current_hash_count = m_miner.getHashCount();

        double hashes = static_cast<double>((current_hash_count - last_hash_count) / 60);

        last_hash_count = current_hash_count;

        std::cout << SuccessMsg("\nMining at ")
                  << SuccessMsg(Common::get_mining_speed(hashes))
                  << "\n\n";
    }
}

void MinerManager::eventLoop()
{
    size_t blocksMined = 0;

    while(true)
    {
        MinerEvent event = waitEvent();

        switch (event.type)
        {
            case MinerEventType::BLOCK_MINED:
            {
                stopBlockchainMonitoring();

                if (submitBlock(m_minedBlock, m_config.daemonHost, m_config.daemonPort))
                {
                    m_lastBlockTimestamp = m_minedBlock.timestamp;

                    if (m_config.blocksLimit != 0 && ++blocksMined == m_config.blocksLimit)
                    {
                        std::cout << InformationMsg("Mined requested amount of blocks (")
                                  << InformationMsg(m_config.blocksLimit)
                                  << InformationMsg("). Quitting.\n");
                        return;
                    }
                }

                BlockMiningParameters params = requestMiningParameters(m_dispatcher, m_config.daemonHost, m_config.daemonPort, m_config.miningAddress);
                adjustBlockTemplate(params.blockTemplate);

                startBlockchainMonitoring();
                startMining(params);
                break;
            }
            case MinerEventType::BLOCKCHAIN_UPDATED:
            {
                stopMining();
                stopBlockchainMonitoring();
                BlockMiningParameters params = requestMiningParameters(m_dispatcher, m_config.daemonHost, m_config.daemonPort, m_config.miningAddress);
                adjustBlockTemplate(params.blockTemplate);

                startBlockchainMonitoring();
                startMining(params);
                break;
            }
        }
    }
}

MinerEvent MinerManager::waitEvent()
{
    while(m_events.empty())
    {
        m_eventOccurred.wait();
        m_eventOccurred.clear();
    }

    MinerEvent event = std::move(m_events.front());
    m_events.pop();

    return event;
}

void MinerManager::pushEvent(MinerEvent&& event)
{
    m_events.push(std::move(event));
    m_eventOccurred.set();
}

void MinerManager::startMining(const CryptoNote::BlockMiningParameters& params)
{
    m_contextGroup.spawn([this, params] ()
    {
        try
        {
            m_minedBlock = m_miner.mine(params, m_config.threadCount);
            pushEvent(BlockMinedEvent());
        }
        catch (const std::exception &)
        {
        }
    });
}

void MinerManager::stopMining()
{
    m_miner.stop();
}

void MinerManager::startBlockchainMonitoring()
{
    m_contextGroup.spawn([this] ()
    {
        try
        {
            m_blockchainMonitor.waitBlockchainUpdate();
            pushEvent(BlockchainUpdatedEvent());
        }
        catch (const std::exception &)
        {
        }
    });
}

void MinerManager::stopBlockchainMonitoring()
{
    m_blockchainMonitor.stop();
}

bool MinerManager::submitBlock(const BlockTemplate& minedBlock, const std::string& daemonHost, uint16_t daemonPort)
{
    CachedBlock cachedBlock(minedBlock);

    try
    {
        HttpClient client(m_dispatcher, daemonHost, daemonPort);

        COMMAND_RPC_SUBMITBLOCK::request request;
        request.emplace_back(Common::toHex(toBinaryArray(minedBlock)));

        COMMAND_RPC_SUBMITBLOCK::response response;

        System::EventLock lk(m_httpEvent);
        JsonRpc::invokeJsonRpcCommand(client, "submitblock", request, response);

        std::cout << SuccessMsg("\nBlock found! Hash: ")
                  << SuccessMsg(cachedBlock.getBlockHash()) << "\n\n";

        return true;
    }
    catch (const std::exception &e)
    {
        std::cout << WarningMsg("Failed to submit block: ")
                  << WarningMsg(e.what()) << std::endl;
        return false;
    }
}

BlockMiningParameters MinerManager::requestMiningParameters(System::Dispatcher& dispatcher, const std::string& daemonHost, uint16_t daemonPort, const std::string& miningAddress)
{
    while (true)
    {
        try
        {
            HttpClient client(dispatcher, daemonHost, daemonPort);

            COMMAND_RPC_GETBLOCKTEMPLATE::request request;
            request.wallet_address = miningAddress;
            request.reserve_size = 0;

            COMMAND_RPC_GETBLOCKTEMPLATE::response response;

            System::EventLock lk(m_httpEvent);
            JsonRpc::invokeJsonRpcCommand(client, "getblocktemplate", request, response);

            if (response.status != CORE_RPC_STATUS_OK)
            {
                std::cout << WarningMsg("Couldn't get block template: " + response.status) << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            BlockMiningParameters params;
            params.difficulty = response.difficulty;

            if(!fromBinaryArray(params.blockTemplate, Common::fromHex(response.blocktemplate_blob)))
            {
                std::cout << WarningMsg("Couldn't parse block template") << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            return params;
        }
        catch (const std::exception &e)
        {
            std::cout << WarningMsg("Couldn't get block template - Is your daemon open?\n")
                      << "(Error message: " << e.what() << ")\n"; 
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
    }
}

void MinerManager::adjustBlockTemplate(CryptoNote::BlockTemplate& blockTemplate) const
{
    adjustMergeMiningTag(blockTemplate);

    if (m_config.firstBlockTimestamp == 0)
    {
        /* no need to fix timestamp */
        return;
    }

    if (m_lastBlockTimestamp == 0)
    {
        blockTemplate.timestamp = m_config.firstBlockTimestamp;
    }
    else if (m_lastBlockTimestamp != 0 && m_config.blockTimestampInterval != 0)
    {
        blockTemplate.timestamp = m_lastBlockTimestamp + m_config.blockTimestampInterval;
    }
}

} //namespace Miner
