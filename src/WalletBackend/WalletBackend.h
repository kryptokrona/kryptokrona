// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include "CryptoTypes.h"

#include "json.hpp"

#include <Logging/LoggerManager.h>

#include <NodeRpcProxy/NodeRpcProxy.h>

#include <string>

#include <tuple>

#include <vector>

#include <WalletBackend/SubWallets.h>
#include <WalletBackend/WalletErrors.h>
#include <WalletBackend/WalletSynchronizer.h>

using nlohmann::json;

class WalletBackend
{
    public:

        /////////////////////////
        /* Public Constructors */
        /////////////////////////

        /* Very heavily suggested to not call this directly. Call one of the
           below functions to correctly initialize a wallet. This is left
           public so the json serialization works correctly. */
        WalletBackend();

        /* Deconstructor */
        ~WalletBackend();

        /* Delete the copy constructor */
        WalletBackend(const WalletBackend &) = delete;

        /* Delete the assignment operator */
        WalletBackend & operator=(const WalletBackend &) = delete;

        /* Delete the move constructor */
        WalletBackend(WalletBackend && old) = delete;

        /* Delete the move assignment operator */
        WalletBackend & operator=(WalletBackend && old) = delete;

        /////////////////////////////
        /* Public static functions */
        /////////////////////////////

        /* Imports a wallet from a mnemonic seed. Returns the wallet class,
           or an error. */
        static std::tuple<WalletError, std::shared_ptr<WalletBackend>> importWalletFromSeed(
            const std::string mnemonicSeed,
            const std::string filename,
            const std::string password,
            const uint64_t scanHeight,
            const std::string daemonHost,
            const uint16_t daemonPort);

        /* Imports a wallet from a private spend key and a view key. Returns
           the wallet class, or an error. */
        static std::tuple<WalletError, std::shared_ptr<WalletBackend>> importWalletFromKeys(
            const Crypto::SecretKey privateSpendKey,
            const Crypto::SecretKey privateViewKey,
            const std::string filename,
            const std::string password,
            const uint64_t scanHeight,
            const std::string daemonHost,
            const uint16_t daemonPort);

        /* Imports a view wallet from a private view key and an address.
           Returns the wallet class, or an error. */
        static std::tuple<WalletError, std::shared_ptr<WalletBackend>> importViewWallet(
            const Crypto::SecretKey privateViewKey,
            const std::string address,
            const std::string filename,
            const std::string password,
            const uint64_t scanHeight,
            const std::string daemonHost,
            const uint16_t daemonPort);

        /* Creates a new wallet with the given filename and password */
        static std::tuple<WalletError, std::shared_ptr<WalletBackend>> createWallet(
            const std::string filename,
            const std::string password,
            const std::string daemonHost,
            const uint16_t daemonPort);

        /* Opens a wallet already on disk with the given filename + password */
        static std::tuple<WalletError, std::shared_ptr<WalletBackend>> openWallet(
            const std::string filename,
            const std::string password,
            const std::string daemonHost,
            const uint16_t daemonPort);

        /* Create an integrated address from an address + paymentID */
        static std::tuple<WalletError, std::string> createIntegratedAddress(
            const std::string address,
            const std::string paymentID);

        /////////////////////////////
        /* Public member functions */
        /////////////////////////////

        /* Save the wallet to disk */
        WalletError save() const;

        /* Converts the class to a json object */
        json toJson() const;

        /* Initializes the class from a json string */
        WalletError fromJson(const json &j);

        /* Initializes the class from a json string, and inits the stuff we
           can't init from the json */
        WalletError fromJson(
            const json &j,
            const std::string filename,
            const std::string password,
            const std::string daemonHost,
            const uint16_t daemonPort);
        
        /* Send a transaction of amount to destination with paymentID */
        std::tuple<WalletError, Crypto::Hash> sendTransactionBasic(
            const std::string destination,
            const uint64_t amount,
            const std::string paymentID);

        /* Advanced send transaction, specify mixin, change address, etc */
        std::tuple<WalletError, Crypto::Hash> sendTransactionAdvanced(
            const std::vector<std::pair<std::string, uint64_t>> destinations,
            const uint64_t mixin,
            const uint64_t fee,
            const std::string paymentID,
            const std::vector<std::string> subWalletsToTakeFrom,
            const std::string changeAddress);

        /* Send a fusion using default mixin, default destination, and
           taking from all subwallets */
        std::tuple<WalletError, Crypto::Hash> sendFusionTransactionBasic();

        /* Send a fusion with advanced options */
        std::tuple<WalletError, Crypto::Hash> sendFusionTransactionAdvanced(
            const uint64_t mixin,
            const std::vector<std::string> subWalletsToTakeFrom,
            const std::string destinationAddress);

        /* Get the balance for one subwallet (error, unlocked, locked) */
        std::tuple<WalletError, uint64_t, uint64_t> getBalance(
            const std::string address) const;

        /* Get the balance for all subwallets */
        std::tuple<uint64_t, uint64_t> getTotalBalance() const;

        uint64_t getTotalUnlockedBalance() const;

        /* Make a new sub wallet (gens a privateSpendKey) */
        WalletError addSubWallet();

        /* Import a sub wallet with the given privateSpendKey */
        WalletError importSubWallet(
            const Crypto::SecretKey privateSpendKey,
            const uint64_t scanHeight,
            const bool newWallet);

        /* Import a view only sub wallet with the given publicSpendKey */
        WalletError importViewSubWallet(
            const Crypto::PublicKey publicSpendKey,
            const uint64_t scanHeight,
            const bool newWallet);

        /* Scan the blockchain, starting from scanHeight / timestamp */
        void reset(uint64_t scanHeight, uint64_t timestamp);

        /* Is the wallet a view only wallet */
        bool isViewWallet() const;

        /* Get the filename of the wallet on disk */
        std::string getWalletLocation() const;

        /* Get the primary address */
        std::string getPrimaryAddress() const;

        /* wallet sync height, local blockchain sync height,
           remote blockchain sync height */
        std::tuple<uint64_t, uint64_t, uint64_t> getSyncStatus() const;

        /* Get the wallet password */
        std::string getWalletPassword() const;

        /* Change the wallet password and save the wallet with the new password */
        WalletError changePassword(const std::string newPassword);

        /* Get all private spend keys, and private view key */
        std::tuple<std::vector<Crypto::SecretKey>, Crypto::SecretKey> getAllPrivateKeys() const;

        /* Get the private spend and private view for the primary address */
        std::tuple<Crypto::SecretKey, Crypto::SecretKey> getPrimaryAddressPrivateKeys() const;

        /* Get the primary address mnemonic seed, if possible */
        std::tuple<bool, std::string> getMnemonicSeed() const;

        /* Get all transactions */
        std::vector<WalletTypes::Transaction> getTransactions() const;

        std::vector<WalletTypes::Transaction> getUnconfirmedTransactions() const;

        /* Get sync heights, hashrate, peer count */
        WalletTypes::WalletStatus getStatus() const;

        /* Returns transactions in the range [startHeight, endHeight - 1] - so if
           we give 1, 100, it will return transactions from block 1 to block 99 */
        std::vector<WalletTypes::Transaction> getTransactionsRange(
            const uint64_t startHeight, const uint64_t endHeight) const;

        /* Get the node fee and address ({0, ""} if empty) */
        std::tuple<uint64_t, std::string> getNodeFee() const;

        /* Swap to a different daemon node */
        WalletError swapNode(std::string daemonHost, uint16_t daemonPort);
        
        /////////////////////////////
        /* Public member variables */
        /////////////////////////////

        std::shared_ptr<EventHandler> m_eventHandler;

    private:

        //////////////////////////
        /* Private constructors */
        //////////////////////////

        /* Standard Constructor */
        WalletBackend(
            const std::string filename,
            const std::string password,
            const Crypto::SecretKey privateSpendKey,
            const Crypto::SecretKey privateViewKey,
            const uint64_t scanHeight,
            const bool newWallet,
            const std::string daemonHost,
            const uint16_t daemonPort);

        /* View wallet constructor */
        WalletBackend(
            const std::string filename,
            const std::string password,
            const Crypto::SecretKey privateViewKey,
            const std::string address,
            const uint64_t scanHeight,
            const std::string daemonHost,
            const uint16_t daemonPort);

        //////////////////////////////
        /* Private member functions */
        //////////////////////////////

        WalletError unsafeSave() const;

        WalletError init();

        //////////////////////////////
        /* Private member variables */
        //////////////////////////////

        /* The filename the wallet is saved to */
        std::string m_filename;

        /* The password the wallet is encrypted with */
        std::string m_password;

        /* The sub wallets container (Using a shared_ptr here so
           the WalletSynchronizer has access to it) */
        std::shared_ptr<SubWallets> m_subWallets;

        /* The daemon connection */
        std::shared_ptr<CryptoNote::NodeRpcProxy> m_daemon;

        /* The log manager */
        std::shared_ptr<Logging::LoggerManager> m_logManager;

        /* The logger instance (Need to keep around because the daemon
           constructor takes a reference to the variable, so if it goes out
           of scope we segfault... :facepalm: */
        std::shared_ptr<Logging::LoggerRef> m_logger;

        /* We use a shared pointer here, because we start the thread in the
           class, with the class as a context, hence, when we go to move the
           WalletSynchronizer class, the thread gets moved() across, but it
           is still pointing to a class which has been moved from, which
           is undefined behaviour. So, none of our changes to the
           WalletSynchronizer class reflect in the thread.

           The ideal way to fix this would probably to disable move semantics,
           and just assign once - however this is pretty tricky to do, as
           we want to use the factory pattern so we're not initializing
           with crappy data, and can return a meaningful error to the user
           rather than having to throw() or check isInitialized() everywhere.

           More info here: https://stackoverflow.com/q/43203869/8737306
           
           PS: I want to die */
        std::shared_ptr<WalletSynchronizer> m_walletSynchronizer;
};
