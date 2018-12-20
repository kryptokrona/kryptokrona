// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include "BlockchainMonitor.h"

#include "Common/StringTools.h"

#include <System/EventLock.h>
#include <System/Timer.h>
#include <System/InterruptedException.h>

#include "Rpc/CoreRpcServerCommandsDefinitions.h"
#include "Rpc/JsonRpc.h"
#include "Rpc/HttpClient.h"

#include <zedwallet++/ColouredMsg.h>

BlockchainMonitor::BlockchainMonitor(System::Dispatcher& dispatcher, const std::string& daemonHost, uint16_t daemonPort, size_t pollingInterval):
    m_dispatcher(dispatcher),
    m_daemonHost(daemonHost),
    m_daemonPort(daemonPort),
    m_pollingInterval(pollingInterval),
    m_stopped(false),
    m_httpEvent(dispatcher),
    m_sleepingContext(dispatcher)
{
    m_httpEvent.set();
}

void BlockchainMonitor::waitBlockchainUpdate()
{
    m_stopped = false;

    Crypto::Hash lastBlockHash = requestLastBlockHash();

    while(!m_stopped)
    {
        m_sleepingContext.spawn([this] ()
        {
            System::Timer timer(m_dispatcher);
            timer.sleep(std::chrono::seconds(m_pollingInterval));
        });

        m_sleepingContext.wait();

        if (lastBlockHash != requestLastBlockHash())
        {
            break;
        }
    }

    if (m_stopped)
    {
        throw System::InterruptedException();
    }
}

void BlockchainMonitor::stop()
{
    m_stopped = true;

    m_sleepingContext.interrupt();
    m_sleepingContext.wait();
}

Crypto::Hash BlockchainMonitor::requestLastBlockHash()
{
    while (true)
    {
        try
        {
            CryptoNote::HttpClient client(m_dispatcher, m_daemonHost, m_daemonPort);

            CryptoNote::COMMAND_RPC_GET_LAST_BLOCK_HEADER::request request;
            CryptoNote::COMMAND_RPC_GET_LAST_BLOCK_HEADER::response response;

            System::EventLock lk(m_httpEvent);
            CryptoNote::JsonRpc::invokeJsonRpcCommand(client, "getlastblockheader", request, response);

            if (response.status != CORE_RPC_STATUS_OK)
            {
                std::cout << WarningMsg("Failed to get block hash - Is your daemon open?\n")
                          << "(Error message: " << response.status << ")\n";

                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            Crypto::Hash blockHash;

            if (!Common::podFromHex(response.block_header.hash, blockHash))
            {
                std::cout << WarningMsg("Failed to parse block hash: " + response.block_header.hash)
                          << std::endl;

                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
            }

            return blockHash;
        }
        catch (const std::exception &e)
        {
            std::cout << WarningMsg("Failed to get block hash - Is your daemon open?\n")
                      << "(Error message: " << e.what() << ")\n";

            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
    }
}
