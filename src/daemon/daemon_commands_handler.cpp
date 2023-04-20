// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include <boost/format.hpp>

#include <ctime>

#include <cryptonote_core/core.h>
#include <cryptonote_core/cryptonote_format_utils.h>
#include <cryptonote_core/currency.h>

#include <cryptonote_protocol/cryptonote_protocol_handler.h>

#include <daemon/daemon_commands_handler.h>

#include <p2p/net_node.h>

#include <rpc/json_rpc.h>

#include <serialization/serialization_tools.h>

#include <utilities/format_tools.h>
#include <utilities/coloured_msg.h>

#include "version.h"

namespace
{
    template <typename T>
    static bool print_as_json(const T &obj)
    {
        std::cout << cryptonote::storeToJson(obj) << ENDL;
        return true;
    }

    std::string printTransactionShortInfo(const cryptonote::CachedTransaction &transaction)
    {
        std::stringstream ss;

        ss << "id: " << transaction.getTransactionHash() << std::endl;
        ss << "fee: " << transaction.getTransactionFee() << std::endl;
        ss << "blobSize: " << transaction.getTransactionBinaryArray().size() << std::endl;

        return ss.str();
    }

    std::string printTransactionFullInfo(const cryptonote::CachedTransaction &transaction)
    {
        std::stringstream ss;
        ss << printTransactionShortInfo(transaction);
        ss << "JSON: \n"
           << cryptonote::storeToJson(transaction.getTransaction()) << std::endl;

        return ss.str();
    }

} // namespace

DaemonCommandsHandler::DaemonCommandsHandler(
    cryptonote::Core &core,
    cryptonote::NodeServer &srv,
    std::shared_ptr<logging::LoggerManager> log,
    cryptonote::RpcServer *prpc_server) : m_core(core),
                                          m_srv(srv),
                                          logger(log, "daemon"),
                                          m_logManager(log),
                                          m_prpc_server(prpc_server)
{
    m_consoleHandler.setHandler("exit", std::bind(&DaemonCommandsHandler::exit, this, std::placeholders::_1), "Shutdown the daemon");
    m_consoleHandler.setHandler("help", std::bind(&DaemonCommandsHandler::help, this, std::placeholders::_1), "Show this help");
    m_consoleHandler.setHandler("print_pl", std::bind(&DaemonCommandsHandler::print_pl, this, std::placeholders::_1), "Print peer list");
    m_consoleHandler.setHandler(
        "print_cn", std::bind(&DaemonCommandsHandler::print_cn, this, std::placeholders::_1), "Print connections");
    m_consoleHandler.setHandler(
        "print_bc",
        std::bind(&DaemonCommandsHandler::print_bc, this, std::placeholders::_1),
        "Print blockchain info in a given blocks range, print_bc <begin_height> [<end_height>]");
    m_consoleHandler.setHandler(
        "print_block",
        std::bind(&DaemonCommandsHandler::print_block, this, std::placeholders::_1),
        "Print block, print_block <block_hash> | <block_height>");
    m_consoleHandler.setHandler(
        "print_tx",
        std::bind(&DaemonCommandsHandler::print_tx, this, std::placeholders::_1),
        "Print transaction, print_tx <transaction_hash>");
    m_consoleHandler.setHandler(
        "print_pool",
        std::bind(&DaemonCommandsHandler::print_pool, this, std::placeholders::_1),
        "Print transaction pool (long format)");
    m_consoleHandler.setHandler(
        "print_pool_sh",
        std::bind(&DaemonCommandsHandler::print_pool_sh, this, std::placeholders::_1),
        "Print transaction pool (short format)");
    m_consoleHandler.setHandler(
        "set_log",
        std::bind(&DaemonCommandsHandler::set_log, this, std::placeholders::_1),
        "set_log <level> - Change current log level, <level> is a number 0-4");
    m_consoleHandler.setHandler("status", std::bind(&DaemonCommandsHandler::status, this, std::placeholders::_1), "Show daemon status");
}

//--------------------------------------------------------------------------------
std::string DaemonCommandsHandler::get_commands_str()
{
    std::stringstream ss;
    ss << cryptonote::CRYPTONOTE_NAME << " v" << PROJECT_VERSION << ENDL;
    ss << "Commands: " << ENDL;
    std::string usage = m_consoleHandler.getUsage();
    boost::replace_all(usage, "\n", "\n  ");
    usage.insert(0, "  ");
    ss << usage << ENDL;
    return ss.str();
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::exit(const std::vector<std::string> &args)
{
    std::cout << InformationMsg("================= EXITING ==================\n"
                                "== PLEASE WAIT, THIS MAY TAKE A LONG TIME ==\n"
                                "============================================\n");

    /* Set log to max when exiting. Sometimes this takes a while, and it helps
       to let users know the daemon is still doing stuff */
    m_logManager->setMaxLevel(logging::TRACE);
    m_consoleHandler.requestStop();
    m_srv.sendStopSignal();
    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::help(const std::vector<std::string> &args)
{
    std::cout << get_commands_str() << ENDL;
    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_pl(const std::vector<std::string> &args)
{
    m_srv.log_peerlist();
    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_cn(const std::vector<std::string> &args)
{
    m_srv.get_payload_object().log_connections();
    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_bc(const std::vector<std::string> &args)
{
    if (!args.size())
    {
        std::cout << "need block index parameter" << ENDL;
        return false;
    }

    uint32_t start_index = 0;
    uint32_t end_index = 0;
    uint32_t end_block_parametr = m_core.getTopBlockIndex();

    if (!common::fromString(args[0], start_index))
    {
        std::cout << "wrong starter block index parameter" << ENDL;
        return false;
    }

    if (args.size() > 1 && !common::fromString(args[1], end_index))
    {
        std::cout << "wrong end block index parameter" << ENDL;
        return false;
    }

    if (end_index == 0)
    {
        end_index = start_index;
    }

    if (end_index > end_block_parametr)
    {
        std::cout << "end block index parameter shouldn't be greater than " << end_block_parametr << ENDL;
        return false;
    }

    if (end_index < start_index)
    {
        std::cout << "end block index should be greater than or equal to starter block index" << ENDL;
        return false;
    }

    cryptonote::COMMAND_RPC_GET_BLOCK_HEADERS_RANGE::request req;
    cryptonote::COMMAND_RPC_GET_BLOCK_HEADERS_RANGE::response res;
    cryptonote::json_rpc::JsonRpcError error_resp;

    req.start_height = start_index;
    req.end_height = end_index;

    // TODO: implement m_is_rpc handling like in monero?
    if (!m_prpc_server->on_get_block_headers_range(req, res, error_resp) || res.status != CORE_RPC_STATUS_OK)
    {
        // TODO res.status handling
        std::cout << "Response status not CORE_RPC_STATUS_OK" << ENDL;
        return false;
    }

    const cryptonote::Currency &currency = m_core.getCurrency();

    bool first = true;
    for (cryptonote::block_header_response &header : res.headers)
    {
        if (!first)
        {
            std::cout << ENDL;
            first = false;
        }

        std::cout << "height: " << header.height << ", timestamp: " << header.timestamp
                  << ", difficulty: " << header.difficulty << ", size: " << header.block_size
                  << ", transactions: " << header.num_txes << ENDL
                  << "major version: " << unsigned(header.major_version)
                  << ", minor version: " << unsigned(header.minor_version) << ENDL << "block id: " << header.hash
                  << ", previous block id: " << header.prev_hash << ENDL << "difficulty: " << header.difficulty
                  << ", nonce: " << header.nonce << ", reward: " << currency.formatAmount(header.reward) << ENDL;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::set_log(const std::vector<std::string> &args)
{
    if (args.size() != 1)
    {
        std::cout << "use: set_log <log_level_number_0-4>" << ENDL;
        return true;
    }

    uint16_t l = 0;
    if (!common::fromString(args[0], l))
    {
        std::cout << "wrong number format, use: set_log <log_level_number_0-4>" << ENDL;
        return true;
    }

    ++l;

    if (l > logging::TRACE)
    {
        std::cout << "wrong number range, use: set_log <log_level_number_0-4>" << ENDL;
        return true;
    }

    m_logManager->setMaxLevel(static_cast<logging::Level>(l));
    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_block_by_height(uint32_t height)
{
    if (height - 1 > m_core.getTopBlockIndex())
    {
        std::cout << "block wasn't found. Current block chain height: " << m_core.getTopBlockIndex() + 1
                  << ", requested: " << height << std::endl;
        return false;
    }

    auto hash = m_core.getBlockHashByIndex(height - 1);
    std::cout << "block_id: " << hash << ENDL;
    print_as_json(m_core.getBlockByIndex(height - 1));

    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_block_by_hash(const std::string &arg)
{
    crypto::Hash block_hash;
    if (!parse_hash256(arg, block_hash))
    {
        return false;
    }

    if (m_core.hasBlock(block_hash))
    {
        print_as_json(m_core.getBlockByHash(block_hash));
    }
    else
    {
        std::cout << "block wasn't found: " << arg << std::endl;
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_block(const std::vector<std::string> &args)
{
    if (args.empty())
    {
        std::cout << "expected: print_block (<block_hash> | <block_height>)" << std::endl;
        return true;
    }

    const std::string &arg = args.front();
    try
    {
        uint32_t height = boost::lexical_cast<uint32_t>(arg);
        print_block_by_height(height);
    }
    catch (boost::bad_lexical_cast &)
    {
        print_block_by_hash(arg);
    }

    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_tx(const std::vector<std::string> &args)
{
    if (args.empty())
    {
        std::cout << "expected: print_tx <transaction hash>" << std::endl;
        return true;
    }

    const std::string &str_hash = args.front();
    crypto::Hash tx_hash;
    if (!parse_hash256(str_hash, tx_hash))
    {
        return true;
    }

    std::vector<crypto::Hash> tx_ids;
    tx_ids.push_back(tx_hash);
    std::vector<cryptonote::BinaryArray> txs;
    std::vector<crypto::Hash> missed_ids;
    m_core.getTransactions(tx_ids, txs, missed_ids);

    if (1 == txs.size())
    {
        cryptonote::CachedTransaction tx(txs.front());
        print_as_json(tx.getTransaction());
    }
    else
    {
        std::cout << "transaction wasn't found: <" << str_hash << '>' << std::endl;
    }

    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_pool(const std::vector<std::string> &args)
{
    std::cout << "Pool state: \n";
    auto pool = m_core.getPoolTransactions();

    for (const auto &tx : pool)
    {
        cryptonote::CachedTransaction ctx(tx);
        std::cout << printTransactionFullInfo(ctx) << "\n";
    }

    std::cout << std::endl;

    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::print_pool_sh(const std::vector<std::string> &args)
{
    std::cout << "Pool short state: \n";
    auto pool = m_core.getPoolTransactions();

    for (const auto &tx : pool)
    {
        cryptonote::CachedTransaction ctx(tx);
        std::cout << printTransactionShortInfo(ctx) << "\n";
    }

    std::cout << std::endl;

    return true;
}

//--------------------------------------------------------------------------------
bool DaemonCommandsHandler::status(const std::vector<std::string> &args)
{
    cryptonote::COMMAND_RPC_GET_INFO::request ireq;
    cryptonote::COMMAND_RPC_GET_INFO::response iresp;

    if (!m_prpc_server->on_get_info(ireq, iresp) || iresp.status != CORE_RPC_STATUS_OK)
    {
        std::cout << "Problem retrieving information from RPC server." << std::endl;
        return false;
    }

    std::cout << utilities::get_status_string(iresp) << std::endl;

    return true;
}
