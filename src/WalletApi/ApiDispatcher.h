// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <WalletBackend/WalletBackend.h>

#include "httplib.h"

class ApiDispatcher
{
    public:
        //////////////////
        /* Constructors */
        //////////////////

        ApiDispatcher(
            const uint16_t bindPort,
            const bool acceptExternalRequests,
            const std::string rpcPassword,
            std::string corsHeader);

        /////////////////////////////
        /* Public member functions */
        /////////////////////////////

        /* Starts the server */
        void start();

        /* Stops the server */
        void stop();
        
    private:

        //////////////////////////////
        /* Private member functions */
        //////////////////////////////

        /* Check authentication and log, then forward on to the handler if
           applicable */
        void middleware(
            const httplib::Request &req,
            httplib::Response &res,
            std::function<std::tuple<WalletError, uint16_t>
                (const nlohmann::json body, httplib::Response &res)> handler);

        /* Verifies that the request has the correct X-API-KEY, and sends a 401
           if it is not. */
        bool checkAuthenticated(
            const httplib::Request &req,
            httplib::Response &res) const;
        
        /* Opens a wallet */
        std::tuple<WalletError, uint16_t> openWallet(
            const nlohmann::json body,
            httplib::Response &res);

        std::tuple<WalletError, uint16_t> keyImportWallet(
            const nlohmann::json body,
            httplib::Response &res);

        std::tuple<WalletError, uint16_t> seedImportWallet(
            const nlohmann::json body,
            httplib::Response &res);

        std::tuple<WalletError, uint16_t> importViewWallet(
            const nlohmann::json body,
            httplib::Response &res);

        std::tuple<WalletError, uint16_t> createWallet(
            const nlohmann::json body,
            httplib::Response &res);

        /* Close and save the wallet */
        std::tuple<WalletError, uint16_t> closeWallet(
            const nlohmann::json body,
            httplib::Response &res);

        /* Handles an OPTIONS request */
        void handleOptions(
            const httplib::Request &req,
            httplib::Response &res) const;

        /* Extracts {host, port, filename, password}, from body */
        std::tuple<std::string, uint16_t, std::string, std::string>
            getDefaultWalletParams(const nlohmann::json body) const;

        /* Assert the wallet is closed */
        bool assertWalletClosed() const;

        /* Assert the wallet is open */
        bool assertWalletOpen() const;
        
        //////////////////////////////
        /* Private member variables */
        //////////////////////////////

        std::shared_ptr<WalletBackend> m_walletBackend = nullptr;

        /* Our server instance */
        httplib::Server m_server;

        /* The --rpc-password hashed with pbkdf2 */
        std::string m_hashedPassword;

        /* Need a mutex when opening / closing wallet, so we can't open two
           at once or something */
        std::mutex m_mutex;

        /* The server host */
        std::string m_host;

        /* The server port */
        uint16_t m_port;

        /* The header to use with 'Access-Control-Allow-Origin'. If empty string,
           header is not added. */
        std::string m_corsHeader;
};
