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
  const CryptoNote::Currency getCurrency();

  void run();
  void stop();

  std::shared_ptr<Logging::ILogger> getLogger() { return logger; }

private:

  void runInProcess(Logging::LoggerRef& log);
  void runRpcProxy(Logging::LoggerRef& log);

  void runWalletService(const CryptoNote::Currency& currency, CryptoNote::INode& node);

  System::Dispatcher* dispatcher;
  System::Event* stopEvent;
  PaymentService::ConfigurationManager config;
  PaymentService::WalletService* service;

  std::shared_ptr<Logging::LoggerGroup> logger = std::make_shared<Logging::LoggerGroup>();

  std::shared_ptr<CryptoNote::CurrencyBuilder> currencyBuilder;

  std::ofstream fileStream;
  Logging::StreamLogger fileLogger;
  Logging::ConsoleLogger consoleLogger;
};
