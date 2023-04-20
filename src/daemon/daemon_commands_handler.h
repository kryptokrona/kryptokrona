// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "common/console_handler.h"

#include <logging/logger_ref.h>
#include <logging/logger_manager.h>
#include "rpc/rpc_server.h"
#include "rpc/core_rpc_server_commands_definitions.h"
#include "rpc/json_rpc.h"

namespace cryptonote
{
    class Core;
    class NodeServer;
}

class DaemonCommandsHandler
{
public:
    DaemonCommandsHandler(cryptonote::Core &core, cryptonote::NodeServer &srv, std::shared_ptr<logging::LoggerManager> log, cryptonote::RpcServer *prpc_server);

    bool start_handling()
    {
        m_consoleHandler.start();
        return true;
    }

    void stop_handling()
    {
        m_consoleHandler.stop();
    }

    bool exit(const std::vector<std::string> &args);

private:
    common::ConsoleHandler m_consoleHandler;
    cryptonote::Core &m_core;
    cryptonote::NodeServer &m_srv;
    logging::LoggerRef logger;
    std::shared_ptr<logging::LoggerManager> m_logManager;
    cryptonote::RpcServer *m_prpc_server;

    std::string get_commands_str();
    bool print_block_by_height(uint32_t height);
    bool print_block_by_hash(const std::string &arg);

    bool help(const std::vector<std::string> &args);
    bool print_pl(const std::vector<std::string> &args);
    bool show_hr(const std::vector<std::string> &args);
    bool hide_hr(const std::vector<std::string> &args);
    bool print_bc_outs(const std::vector<std::string> &args);
    bool print_cn(const std::vector<std::string> &args);
    bool print_bc(const std::vector<std::string> &args);
    bool print_bci(const std::vector<std::string> &args);
    bool set_log(const std::vector<std::string> &args);
    bool print_block(const std::vector<std::string> &args);
    bool print_tx(const std::vector<std::string> &args);
    bool print_pool(const std::vector<std::string> &args);
    bool print_pool_sh(const std::vector<std::string> &args);
    bool start_mining(const std::vector<std::string> &args);
    bool stop_mining(const std::vector<std::string> &args);
    bool status(const std::vector<std::string> &args);
};
