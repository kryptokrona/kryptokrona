// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
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

class PaymentGateService {
public:
  PaymentGateService();

  bool init(int argc, char** argv);

  const PaymentService::ConfigurationManager& getConfig() const { return config; }
  PaymentService::WalletConfiguration getWalletConfig() const;
  const cryptonote::Currency getCurrency();

  void run();
  void stop();

  std::shared_ptr<logging::ILogger> getLogger() { return logger; }

private:

  void runInProcess(logging::LoggerRef& log);
  void runRpcProxy(logging::LoggerRef& log);

  void runWalletService(const cryptonote::Currency& currency, cryptonote::INode& node);

  system::Dispatcher* dispatcher;
  system::Event* stopEvent;
  PaymentService::ConfigurationManager config;
  PaymentService::WalletService* service;

  std::shared_ptr<logging::LoggerGroup> logger = std::make_shared<logging::LoggerGroup>();

  std::shared_ptr<cryptonote::CurrencyBuilder> currencyBuilder;

  std::ofstream fileStream;
  logging::StreamLogger fileLogger;
  logging::ConsoleLogger consoleLogger;
};
