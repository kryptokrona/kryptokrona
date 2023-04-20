// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "blockchain_monitor.h"

#include "common/string_tools.h"

#include <syst/event_lock.h>
#include <syst/timer.h>
#include <syst/interrupted_exception.h>

#include "rpc/core_rpc_server_commands_definitions.h"
#include "rpc/json_rpc.h"

#include <utilities/coloured_msg.h>

using json = nlohmann::json;

BlockchainMonitor::BlockchainMonitor(
    syst::Dispatcher &dispatcher,
    const size_t pollingInterval,
    const std::shared_ptr<httplib::Client> httpClient) :

                                                         m_dispatcher(dispatcher),
                                                         m_pollingInterval(pollingInterval),
                                                         m_stopped(false),
                                                         m_sleepingContext(dispatcher),
                                                         m_httpClient(httpClient)
{
}

void BlockchainMonitor::waitBlockchainUpdate()
{
    m_stopped = false;

    auto lastBlockHash = requestLastBlockHash();

    while (!lastBlockHash && !m_stopped)
    {
        std::this_thread::sleep_for(std::chrono::seconds(m_pollingInterval));
        lastBlockHash = requestLastBlockHash();
    }

    while (!m_stopped)
    {
        m_sleepingContext.spawn([this]()
                                {
            syst::Timer timer(m_dispatcher);
            timer.sleep(std::chrono::seconds(m_pollingInterval)); });

        m_sleepingContext.wait();

        auto nextBlockHash = requestLastBlockHash();

        while (!nextBlockHash && !m_stopped)
        {
            std::this_thread::sleep_for(std::chrono::seconds(m_pollingInterval));
            nextBlockHash = requestLastBlockHash();
        }

        if (lastBlockHash.value() != nextBlockHash.value())
        {
            break;
        }
    }

    if (m_stopped)
    {
        throw syst::InterruptedException();
    }
}

void BlockchainMonitor::stop()
{
    m_stopped = true;

    m_sleepingContext.interrupt();
    m_sleepingContext.wait();
}

std::optional<crypto::Hash> BlockchainMonitor::requestLastBlockHash()
{
    json j = {
        {"jsonrpc", "2.0"},
        {"method", "getlastblockheader"},
        {"params", {}}};

    auto res = m_httpClient->Post("/json_rpc", j.dump(), "application/json");

    if (!res)
    {
        std::cout << WarningMsg("Failed to get block hash - Is your daemon open?\n");

        return std::nullopt;
    }

    if (res->status != 200)
    {
        std::stringstream stream;

        stream << "Failed to get block hash - received unexpected http "
               << "code from server: "
               << res->status << std::endl;

        std::cout << WarningMsg(stream.str()) << std::endl;

        return std::nullopt;
    }

    try
    {
        json j = json::parse(res->body);

        const std::string status = j.at("result").at("status").get<std::string>();

        if (status != "OK")
        {
            std::stringstream stream;

            stream << "Failed to get block hash from daemon. Response: "
                   << status << std::endl;

            std::cout << WarningMsg(stream.str());

            return std::nullopt;
        }

        return j.at("result").at("block_header").at("hash").get<crypto::Hash>();
    }
    catch (const json::exception &e)
    {
        std::stringstream stream;

        stream << "Failed to parse block hash from daemon. Received data:\n"
               << res->body << "\nParse error: " << e.what() << std::endl;

        std::cout << WarningMsg(stream.str());

        return std::nullopt;
    }
}
