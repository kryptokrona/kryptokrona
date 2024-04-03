// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "payment_gate_service.h"

#include <future>

#include "common/signal_handler.h"
#include "common/util.h"
#include "logging/logger_ref.h"
#include "payment_service_json_rpc_server.h"

#include "common/scope_exit.h"
#include "node_rpc_proxy/node_rpc_proxy.h"
#include <syst/context.h>
#include "wallet/wallet_green.h"

#ifdef ERROR
#undef ERROR
#endif

#ifdef _WIN32 // why is this still here?
#include <direct.h>
#else
#include <unistd.h>
#endif

using namespace payment_service;

void changeDirectory(const std::string &path)
{
    if (chdir(path.c_str()))
    {
        throw std::runtime_error("Couldn't change directory to \'" + path + "\': " + strerror(errno));
    }
}

void stopSignalHandler(PaymentGateService *pg)
{
    pg->stop();
}

PaymentGateService::PaymentGateService() : dispatcher(nullptr),
                                           stopEvent(nullptr),
                                           config(),
                                           service(nullptr),
                                           fileLogger(logging::TRACE),
                                           consoleLogger(logging::INFO)
{
    currencyBuilder = std::make_shared<cryptonote::CurrencyBuilder>(logger);
    consoleLogger.setPattern("%D %T %L ");
    fileLogger.setPattern("%D %T %L ");
}

bool PaymentGateService::init(int argc, char **argv)
{
    if (!config.init(argc, argv))
    {
        return false;
    }

    logger->setMaxLevel(static_cast<logging::Level>(config.serviceConfig.logLevel));
    logger->setPattern("%D %T %L ");
    logger->addLogger(consoleLogger);

    if (!config.serviceConfig.serverRoot.empty())
    {
        changeDirectory(config.serviceConfig.serverRoot);
        logging::LoggerRef log(logger, "main");
        log(logging::INFO) << "Current working directory now is " << config.serviceConfig.serverRoot;
    }

    fileStream.open(config.serviceConfig.logFile, std::ofstream::app);

    if (!fileStream)
    {
        throw std::runtime_error("Couldn't open log file");
    }

    fileLogger.attachToStream(fileStream);
    logger->addLogger(fileLogger);

    return true;
}

WalletConfiguration PaymentGateService::getWalletConfig() const
{
    return WalletConfiguration{
        config.serviceConfig.containerFile,
        config.serviceConfig.containerPassword,
        config.serviceConfig.syncFromZero,
        config.serviceConfig.secretViewKey,
        config.serviceConfig.secretSpendKey,
        config.serviceConfig.mnemonicSeed,
        config.serviceConfig.scanHeight,
    };
}

const cryptonote::Currency PaymentGateService::getCurrency()
{
    return currencyBuilder->currency();
}

void PaymentGateService::run()
{

    syst::Dispatcher localDispatcher;
    syst::Event localStopEvent(localDispatcher);

    this->dispatcher = &localDispatcher;
    this->stopEvent = &localStopEvent;

    tools::SignalHandler::install(std::bind(&stopSignalHandler, this));

    logging::LoggerRef log(logger, "run");

    runRpcProxy(log);

    this->dispatcher = nullptr;
    this->stopEvent = nullptr;
}

void PaymentGateService::stop()
{
    logging::LoggerRef log(logger, "stop");

    log(logging::INFO, logging::BRIGHT_WHITE) << "Stop signal caught";

    if (dispatcher != nullptr)
    {
        dispatcher->remoteSpawn([&]()
                                {
      if (stopEvent != nullptr) {
        stopEvent->set();
      } });
    }
}

void PaymentGateService::runRpcProxy(logging::LoggerRef &log)
{
    log(logging::INFO) << "Starting Payment Gate with remote node, timeout: " << config.serviceConfig.initTimeout;
    cryptonote::Currency currency = currencyBuilder->currency();

    std::unique_ptr<cryptonote::INode> node(
        payment_service::NodeFactory::createNode(
            config.serviceConfig.daemonAddress,
            config.serviceConfig.daemonPort,
            config.serviceConfig.initTimeout,
            log.getLogger()));

    runWalletService(currency, *node);
}

void PaymentGateService::runWalletService(const cryptonote::Currency &currency, cryptonote::INode &node)
{
    payment_service::WalletConfiguration walletConfiguration{
        config.serviceConfig.containerFile,
        config.serviceConfig.containerPassword,
        config.serviceConfig.syncFromZero};

    std::unique_ptr<cryptonote::WalletGreen> wallet(new cryptonote::WalletGreen(*dispatcher, currency, node, logger));

    service = new payment_service::WalletService(currency, *dispatcher, node, *wallet, *wallet, walletConfiguration, logger);
    std::unique_ptr<payment_service::WalletService> serviceGuard(service);
    try
    {
        service->init();
    }
    catch (std::exception &e)
    {
        logging::LoggerRef(logger, "run")(logging::ERROR, logging::BRIGHT_RED) << "Failed to init walletService reason: " << e.what();
        return;
    }

    if (config.serviceConfig.printAddresses)
    {
        // print addresses and exit
        std::vector<std::string> addresses;
        service->getAddresses(addresses);
        for (const auto &address : addresses)
        {
            std::cout << "Address: " << address << std::endl;
        }
    }
    else
    {
        payment_service::PaymentServiceJsonRpcServer rpcServer(*dispatcher, *stopEvent, *service, logger, config);
        rpcServer.start(config.serviceConfig.bindAddress, config.serviceConfig.bindPort);

        logging::LoggerRef(logger, "PaymentGateService")(logging::INFO, logging::BRIGHT_WHITE) << "JSON-RPC server stopped, stopping wallet service...";

        try
        {
            service->saveWallet();
        }
        catch (std::exception &ex)
        {
            logging::LoggerRef(logger, "saveWallet")(logging::WARNING, logging::YELLOW) << "Couldn't save container: " << ex.what();
        }
    }
}
