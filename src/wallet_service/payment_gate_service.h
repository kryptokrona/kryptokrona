// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "configuration_manager.h"

#include <config/cli_header.h>
#include "logging/console_logger.h"
#include "logging/logger_group.h"
#include "logging/stream_logger.h"

#include "wallet_service/node_factory.h"
#include "wallet_service/wallet_service.h"

class PaymentGateService
{
public:
    PaymentGateService();

    bool init(int argc, char **argv);

    const payment_service::ConfigurationManager &getConfig() const { return config; }
    payment_service::WalletConfiguration getWalletConfig() const;
    const cryptonote::Currency getCurrency();

    void run();
    void stop();

    std::shared_ptr<logging::ILogger> getLogger() { return logger; }

private:
    void runInProcess(logging::LoggerRef &log);
    void runRpcProxy(logging::LoggerRef &log);

    void runWalletService(const cryptonote::Currency &currency, cryptonote::INode &node);

    syst::Dispatcher *dispatcher;
    syst::Event *stopEvent;
    payment_service::ConfigurationManager config;
    payment_service::WalletService *service;

    std::shared_ptr<logging::LoggerGroup> logger = std::make_shared<logging::LoggerGroup>();

    std::shared_ptr<cryptonote::CurrencyBuilder> currencyBuilder;

    std::ofstream fileStream;
    logging::StreamLogger fileLogger;
    logging::ConsoleLogger consoleLogger;
};
