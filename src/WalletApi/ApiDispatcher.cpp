// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

////////////////////////////////////
#include <WalletApi/ApiDispatcher.h>
////////////////////////////////////

#include <config/CryptoNoteConfig.h>

#include <cryptopp/modes.h>
#include <cryptopp/sha.h>
#include <cryptopp/pwdbased.h>

#include <iomanip>

#include <iostream>

#include "json.hpp"

#include <WalletApi/Constants.h>

#include <WalletBackend/ValidateParameters.h>

using namespace httplib;

ApiDispatcher::ApiDispatcher(
    const uint16_t bindPort,
    const bool acceptExternalRequests,
    const std::string rpcPassword,
    const std::string corsHeader) :
    m_port(bindPort),
    m_corsHeader(corsHeader)
{
    m_host = acceptExternalRequests ? "0.0.0.0" : "127.0.0.1";

    using namespace CryptoPP;

    /* Using SHA256 as the algorithm */
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf2;

    /* Salt of all zeros (this is bad...) */
    byte salt[16] = {};

    byte key[16];

    /* Hash the password with pbkdf2 */
    pbkdf2.DeriveKey(
        key, sizeof(key), 0, (byte *)rpcPassword.c_str(),
        rpcPassword.size(), salt, sizeof(salt), ApiConstants::PBKDF2_ITERATIONS
    );

    /* Store this later for rpc requests */
    m_hashedPassword = Common::podToHex(key);

    using namespace std::placeholders;

    /* Route the request through our middleware function, before forwarding
       to the specified function */
    const auto router = [this](const auto function)
    {
        return [this, function](const Request &req, Response &res)
        {
            /* Pass the inputted function with the arguments passed through
               to middleware */
            middleware(req, res, std::bind(function, this, _1, _2, _3));
        };
    };

    m_server.Post("/wallet/open", router(&ApiDispatcher::openWallet))
            .Post("/wallet/import/key", router(&ApiDispatcher::keyImportWallet))
            .Post("/wallet/import/seed", router(&ApiDispatcher::seedImportWallet))
            .Post("/wallet/import/view", router(&ApiDispatcher::importViewWallet))
            .Post("/wallet/create", router(&ApiDispatcher::createWallet))

            .Delete("/wallet", router(&ApiDispatcher::closeWallet))

            .Put("/save", router(&ApiDispatcher::saveWallet))
            .Put("/reset", router(&ApiDispatcher::resetWallet))
            .Put("/node", router(&ApiDispatcher::setNodeInfo))

            .Get("/node", router(&ApiDispatcher::getNodeInfo))
            .Get("/keys", router(&ApiDispatcher::getPrivateViewKey))
            .Get("/keys/" + ApiConstants::addressRegex, router(&ApiDispatcher::getSpendKeys))
            .Get("/keys/mnemonic/" + ApiConstants::addressRegex, router(&ApiDispatcher::getMnemonicSeed))

            /* Matches everything */
            .Options(".*", router(&ApiDispatcher::handleOptions));
}

void ApiDispatcher::start()
{
    m_server.listen(m_host, m_port);
}

void ApiDispatcher::stop()
{
    m_server.stop();
}

void ApiDispatcher::middleware(
    const Request &req,
    Response &res,
    std::function<std::tuple<WalletError, uint16_t>
        (const Request &req,
         Response &res,
         const nlohmann::json &body)> handler)
{
    std::cout << "Incoming " << req.method << " request: " << req.path << std::endl;

    nlohmann::json body;

    try
    {
        body = json::parse(req.body);
        std::cout << "Body:\n" << std::setw(4) << body << std::endl;
    }
    catch (const json::exception &)
    {
        /* Not neccessarily an error if body isn't needed */
    }

    /* Add the cors header if not empty string */
    if (m_corsHeader != "")
    {
        res.set_header("Access-Control-Allow-Origin", m_corsHeader);
    }

    /* TODO: Uncomment
    if (!checkAuthenticated(req, res))
    {
        return;
    }
    */
    
    try
    {
        const auto [error, statusCode] = handler(req, res, body);

        if (error)
        {
            /* Bad request */
            res.status = 400;

            nlohmann::json j {
                {"errorCode", error.getErrorCode()},
                {"errorMessage", error.getErrorMessage()}
            };

            /* Pretty print ;o */
            res.set_content(j.dump(4) + "\n", "application/json");
        }
        else
        {
            res.status = statusCode;
        }
    }
    /* Most likely a key was missing. Do the error handling here to make the
       rest of the code simpler */
    catch (const json::exception &e)
    {
        std::cout << "Caught JSON exception, likely missing required "
                     "json parameter: " << e.what() << std::endl;
        res.status = 400;
    }
    catch (const std::exception &e)
    {
        std::cout << "Caught unexpected exception: " << e.what() << std::endl;
        res.status = 500;
    }
}

bool ApiDispatcher::checkAuthenticated(const Request &req, Response &res) const
{
    if (!req.has_header("X-API-KEY"))
    {
        std::cout << "Rejecting unauthorized request: X-API-KEY header is missing.\n";

        /* Unauthorized */
        res.status = 401;
        return false;
    }

    std::string apiKey = req.get_header_value("X-API-KEY");

    if (apiKey == m_hashedPassword)
    {
        return true;
    }

    std::cout << "Rejecting unauthorized request: X-API-KEY is incorrect.\n"
                 "Expected: " << m_hashedPassword
              << "\nActual: " << apiKey << std::endl;

    res.status = 401;

    return false;
}

///////////////////
/* POST REQUESTS */
///////////////////

std::tuple<WalletError, uint16_t> ApiDispatcher::openWallet(
    const Request &req,
    Response &res,
    const nlohmann::json &body)
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletClosed())
    {
        return {SUCCESS, 403};
    }

    const auto [daemonHost, daemonPort, filename, password] = getDefaultWalletParams(body);

    WalletError error;

    std::tie(error, m_walletBackend) = WalletBackend::openWallet(
        filename, password, daemonHost, daemonPort
    );

    return {error, 200};
}

std::tuple<WalletError, uint16_t> ApiDispatcher::keyImportWallet(
    const Request &req,
    Response &res,
    const nlohmann::json &body)
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletClosed())
    {
        return {SUCCESS, 403};
    }

    const auto [daemonHost, daemonPort, filename, password] = getDefaultWalletParams(body);

    Crypto::SecretKey privateViewKey = body.at("privateViewKey").get<Crypto::SecretKey>();
    Crypto::SecretKey privateSpendKey = body.at("privateSpendKey").get<Crypto::SecretKey>();

    uint64_t scanHeight = 0;

    if (body.find("scanHeight") != body.end())
    {
        scanHeight = body.at("scanHeight").get<uint64_t>();
    }

    WalletError error;

    std::tie(error, m_walletBackend) = WalletBackend::importWalletFromKeys(
        privateSpendKey, privateViewKey, filename, password, scanHeight,
        daemonHost, daemonPort
    );

    return {error, 200};
}

std::tuple<WalletError, uint16_t> ApiDispatcher::seedImportWallet(
    const Request &req,
    Response &res,
    const nlohmann::json &body)
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletClosed())
    {
        return {SUCCESS, 403};
    }

    const auto [daemonHost, daemonPort, filename, password] = getDefaultWalletParams(body);

    std::string mnemonicSeed = body.at("mnemonicSeed").get<std::string>();

    uint64_t scanHeight = 0;

    if (body.find("scanHeight") != body.end())
    {
        scanHeight = body.at("scanHeight").get<uint64_t>();
    }

    WalletError error;

    std::tie(error, m_walletBackend) = WalletBackend::importWalletFromSeed(
        mnemonicSeed, filename, password, scanHeight, daemonHost, daemonPort
    );

    return {error, 200};
}

std::tuple<WalletError, uint16_t> ApiDispatcher::importViewWallet(
    const Request &req,
    Response &res,
    const nlohmann::json &body)
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletClosed())
    {
        return {SUCCESS, 403};
    }

    const auto [daemonHost, daemonPort, filename, password] = getDefaultWalletParams(body);

    std::string address = body.at("address").get<std::string>();
    Crypto::SecretKey privateViewKey = body.at("privateViewKey").get<Crypto::SecretKey>();

    uint64_t scanHeight = 0;

    if (body.find("scanHeight") != body.end())
    {
        scanHeight = body.at("scanHeight").get<uint64_t>();
    }

    WalletError error;

    std::tie(error, m_walletBackend) = WalletBackend::importViewWallet(
        privateViewKey, address, filename, password, scanHeight,
        daemonHost, daemonPort
    );
    
    return {error, 200};
}

std::tuple<WalletError, uint16_t> ApiDispatcher::createWallet(
    const Request &req,
    Response &res,
    const nlohmann::json &body)
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletClosed())
    {
        return {SUCCESS, 403};
    }

    const auto [daemonHost, daemonPort, filename, password] = getDefaultWalletParams(body);

    WalletError error;

    std::tie(error, m_walletBackend) = WalletBackend::createWallet(
        filename, password, daemonHost, daemonPort
    );

    return {error, 200};
}

/////////////////////
/* DELETE REQUESTS */
/////////////////////

std::tuple<WalletError, uint16_t> ApiDispatcher::closeWallet(
    const Request &req,
    Response &res,
    const nlohmann::json &body)
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletOpen())
    {
        return {SUCCESS, 403};
    }

    m_walletBackend = nullptr;

    return {SUCCESS, 200};
}

//////////////////
/* PUT REQUESTS */
//////////////////

std::tuple<WalletError, uint16_t> ApiDispatcher::saveWallet(
    const Request &req,
    Response &res,
    const nlohmann::json &body) const
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletOpen())
    {
        return {SUCCESS, 403};
    }

    m_walletBackend->save();

    return {SUCCESS, 200};
}

std::tuple<WalletError, uint16_t> ApiDispatcher::resetWallet(
    const Request &req,
    Response &res,
    const nlohmann::json &body)
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletOpen())
    {
        return {SUCCESS, 403};
    }

    uint64_t scanHeight = 0;
    uint64_t timestamp = 0;

    if (body.find("scanHeight") != body.end())
    {
        scanHeight = body.at("scanHeight").get<uint64_t>();
    }

    m_walletBackend->reset(scanHeight, timestamp);

    return {SUCCESS, 200};
}

std::tuple<WalletError, uint16_t> ApiDispatcher::setNodeInfo(
    const Request &req,
    Response &res,
    const nlohmann::json &body)
{
    std::scoped_lock lock(m_mutex);

    if (!assertWalletOpen())
    {
        return {SUCCESS, 403};
    }

    std::string daemonHost = body.at("daemonHost").get<std::string>();
    uint16_t daemonPort = body.at("daemonPort").get<uint16_t>();

    m_walletBackend->swapNode(daemonHost, daemonPort);

    return {SUCCESS, 202};
}

//////////////////
/* GET REQUESTS */
//////////////////

std::tuple<WalletError, uint16_t> ApiDispatcher::getNodeInfo(
    const Request &req,
    Response &res,
    const nlohmann::json &body) const
{
    if (!assertWalletOpen())
    {
        return {SUCCESS, 403};
    }

    const auto [daemonHost, daemonPort] = m_walletBackend->getNodeAddress();

    const auto [nodeFee, nodeAddress] = m_walletBackend->getNodeFee();

    nlohmann::json j {
        {"daemonHost", daemonHost},
        {"daemonPort", daemonPort},
        {"nodeFee", nodeFee},
        {"nodeAddress", nodeAddress}
    };

    res.set_content(j.dump(4) + "\n", "application/json");

    return {SUCCESS, 200};
}

std::tuple<WalletError, uint16_t> ApiDispatcher::getPrivateViewKey(
    const Request &req,
    Response &res,
    const nlohmann::json &body) const
{
    if (!assertWalletOpen())
    {
        return {SUCCESS, 403};
    }

    nlohmann::json j {
        {"privateViewKey", m_walletBackend->getPrivateViewKey()}
    };

    res.set_content(j.dump(4) + "\n", "application/json");

    return {SUCCESS, 200};
}

/* Gets the spend keys for the given address */
std::tuple<WalletError, uint16_t> ApiDispatcher::getSpendKeys(
    const Request &req,
    Response &res,
    const nlohmann::json &body) const
{
    if (!assertWalletOpen())
    {
        return {SUCCESS, 403};
    }

    if (!assertIsNotViewWallet())
    {
        return {ILLEGAL_VIEW_WALLET_OPERATION, 400};
    }

    /* Remove the keys prefix to get the address */
    std::string address = req.path.substr(std::string("/keys/").size());

    if (WalletError error = validateAddresses({address}, false); error != SUCCESS)
    {
        return {error, 400};
    }

    const auto [error, publicSpendKey, privateSpendKey] = m_walletBackend->getSpendKeys(address);

    if (error)
    {
        return {error, 400};
    }

    nlohmann::json j {
        {"publicSpendKey", publicSpendKey},
        {"privateSpendKey", privateSpendKey}
    };

    res.set_content(j.dump(4) + "\n", "application/json");

    return {SUCCESS, 200};
}

/* Gets the mnemonic seed for the given address (if possible) */
std::tuple<WalletError, uint16_t> ApiDispatcher::getMnemonicSeed(
    const Request &req,
    Response &res,
    const nlohmann::json &body) const
{
    if (!assertWalletOpen())
    {
        return {SUCCESS, 403};
    }

    if (!assertIsNotViewWallet())
    {
        return {ILLEGAL_VIEW_WALLET_OPERATION, 400};
    }

    /* Remove the keys prefix to get the address */
    std::string address = req.path.substr(std::string("/keys/mnemonic/").size());

    if (WalletError error = validateAddresses({address}, false); error != SUCCESS)
    {
        return {error, 400};
    }

    const auto [error, mnemonicSeed] = m_walletBackend->getMnemonicSeedForAddress(address);

    if (error)
    {
        return {error, 400};
    }

    nlohmann::json j {
        {"mnemonicSeed", mnemonicSeed}
    };

    res.set_content(j.dump(4) + "\n", "application/json");

    return {SUCCESS, 200};
}

//////////////////////
/* OPTIONS REQUESTS */
//////////////////////

std::tuple<WalletError, uint16_t> ApiDispatcher::handleOptions(
    const Request &req,
    Response &res,
    const nlohmann::json &body) const
{
    std::string supported = "OPTIONS, GET, POST, PUT, DELETE";

    if (m_corsHeader == "")
    {
        supported = "";
    }

    if (req.has_header("Access-Control-Request-Method"))
    {
        res.set_header("Access-Control-Allow-Methods", supported);
    }
    else
    {
        res.set_header("Allow", supported); 
    }

    return {SUCCESS, 200};
}

std::tuple<std::string, uint16_t, std::string, std::string>
    ApiDispatcher::getDefaultWalletParams(const nlohmann::json body) const
{
    std::string daemonHost = "127.0.0.1";
    uint16_t daemonPort = CryptoNote::RPC_DEFAULT_PORT;

    std::string filename = body.at("filename").get<std::string>();
    std::string password = body.at("password").get<std::string>();

    if (body.find("daemonHost") != body.end())
    {
        daemonHost = body.at("daemonHost").get<std::string>();
    }

    if (body.find("daemonPort") != body.end())
    {
        daemonPort = body.at("daemonPort").get<uint16_t>();
    }

    return {daemonHost, daemonPort, filename, password};
}

//////////////////////////
/* END OF API FUNCTIONS */
//////////////////////////

bool ApiDispatcher::assertIsNotViewWallet() const
{
    if (m_walletBackend->isViewWallet())
    {
        std::cout << "Client requested to perform an operation which requires "
                     "a non view wallet, but wallet is a view wallet" << std::endl;
        return false;
    }

    return true;
}

bool ApiDispatcher::assertWalletClosed() const
{
    if (m_walletBackend != nullptr)
    {
        std::cout << "Client requested to open a wallet, whilst once is already open" << std::endl;
        return false;
    }

    return true;
}

bool ApiDispatcher::assertWalletOpen() const
{
    if (m_walletBackend == nullptr)
    {
        std::cout << "Client requested to modify a wallet, whilst no wallet is open" << std::endl;
        return false;
    }

    return true;
}
