// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <json.hpp>
#include <config/cryptonote_config.h>
#include <Logging/ILogger.h>
#include "Common/PathTools.h"
#include "Common/Util.h"

using nlohmann::json;

namespace DaemonConfig
{
    struct DaemonConfiguration
    {
        DaemonConfiguration()
        {
            std::stringstream logfile;
            logfile << CryptoNote::CRYPTONOTE_NAME << "d.log";

            dataDirectory = Tools::getDefaultDataDirectory();
            checkPoints = "default";
            logFile = logfile.str();
            logLevel = Logging::WARNING;
            dbMaxOpenFiles = CryptoNote::DATABASE_DEFAULT_MAX_OPEN_FILES;
            dbReadCacheSizeMB = CryptoNote::DATABASE_READ_BUFFER_MB_DEFAULT_SIZE;
            dbThreads = CryptoNote::DATABASE_DEFAULT_BACKGROUND_THREADS_COUNT;
            dbWriteBufferSizeMB = CryptoNote::DATABASE_WRITE_BUFFER_MB_DEFAULT_SIZE;
            p2pInterface = "0.0.0.0";
            p2pPort = CryptoNote::P2P_DEFAULT_PORT;
            p2pExternalPort = 0;
            rpcInterface = "127.0.0.1";
            rpcPort = CryptoNote::RPC_DEFAULT_PORT;
            noConsole = false;
            enableBlockExplorer = false;
            localIp = false;
            hideMyPort = false;
            help = false;
            version = false;
            osVersion = false;
            printGenesisTx = false;
            dumpConfig = false;
        }

        std::string dataDirectory;
        std::string logFile;
        std::string feeAddress;
        std::string rpcInterface;
        std::string p2pInterface;
        std::string checkPoints;

        std::vector<std::string> peers;
        std::vector<std::string> priorityNodes;
        std::vector<std::string> exclusiveNodes;
        std::vector<std::string> seedNodes;
        std::vector<std::string> enableCors;

        int logLevel;
        int feeAmount;
        int rpcPort;
        int p2pPort;
        int p2pExternalPort;
        int dbThreads;
        int dbMaxOpenFiles;
        int dbWriteBufferSizeMB;
        int dbReadCacheSizeMB;

        bool noConsole;
        bool enableBlockExplorer;
        bool localIp;
        bool hideMyPort;

        std::string configFile;
        std::string outputFile;
        std::vector<std::string> genesisAwardAddresses;
        bool help;
        bool version;
        bool osVersion;
        bool printGenesisTx;
        bool dumpConfig;
    };

    DaemonConfiguration initConfiguration(const char *path);

    bool updateConfigFormat(const std::string configFile, DaemonConfiguration &config);
    void handleSettings(int argc, char *argv[], DaemonConfiguration &config);
    void handleSettings(const std::string configFile, DaemonConfiguration &config);
    void asFile(const DaemonConfiguration &config, const std::string &filename);
    std::string asString(const DaemonConfiguration &config);
    json asJSON(const DaemonConfiguration &config);
}