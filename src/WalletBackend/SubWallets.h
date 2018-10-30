// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#pragma once

#include <crypto/crypto.h>

#include <WalletBackend/SubWallet.h>

class SubWallets
{
    public:

        //////////////////
        /* Constructors */
        //////////////////

        SubWallets() = default;

        /* Creates a new wallet */
        SubWallets(
            const Crypto::SecretKey privateSpendKey,
            const Crypto::SecretKey privateViewKey,
            const std::string address,
            const uint64_t scanHeight,
            const bool newWallet);

        /* Creates a new view only subwallet */
        SubWallets(
            const Crypto::SecretKey privateViewKey,
            const std::string address,
            const uint64_t scanHeight,
            const bool newWallet);

        /* Copy constructor */
        SubWallets(const SubWallets &other);

        /////////////////////////////
        /* Public member functions */
        /////////////////////////////

        /* Adds a sub wallet with a random spend key */
        WalletError addSubWallet();

        /* Imports a sub wallet with the given private spend key */
        WalletError importSubWallet(
            const Crypto::SecretKey privateSpendKey,
            const uint64_t scanHeight,
            const bool newWallet);

        /* Imports a sub view only wallet with the given public spend key */
        WalletError importViewSubWallet(
            const Crypto::PublicKey privateSpendKey,
            const uint64_t scanHeight,
            const bool newWallet);

        /* Returns (height, timestamp) to begin syncing at. Only one (if any)
           of the values will be non zero */
        std::tuple<uint64_t, uint64_t> getMinInitialSyncStart() const;

        /* Converts the class to a json object */
        json toJson() const;

        /* Initializes the class from a json string */
        void fromJson(const json &j);

        /* Store a transaction */
        void addTransaction(const WalletTypes::Transaction tx);

        /* Store an outgoing tx, not yet in a block */
        void addUnconfirmedTransaction(const WalletTypes::Transaction tx);

        /* Generates a key image using the public+private spend key of the
           subwallet. Wallet must not be a view wallet (and must exist, but
           the WalletSynchronizer already checks this) */
        void completeAndStoreTransactionInput(
            const Crypto::PublicKey publicSpendKey,
            const Crypto::KeyDerivation derivation,
            const size_t outputIndex,
            WalletTypes::TransactionInput input);

        /* Get key images + amounts for the specified transfer amount. We
           can either take from all subwallets, or from some subset
           (usually just one address, e.g. if we're running a web wallet) */
        std::tuple<std::vector<WalletTypes::TxInputAndOwner>, uint64_t>
                getTransactionInputsForAmount(
            const uint64_t amount,
            const bool takeFromAll,
            std::vector<Crypto::PublicKey> subWalletsToTakeFrom) const;

        std::tuple<std::vector<WalletTypes::TxInputAndOwner>, uint64_t, uint64_t>
                getFusionTransactionInputs(
            const bool takeFromAll,
            std::vector<Crypto::PublicKey> subWalletsToTakeFrom,
            const uint64_t mixin) const;

        /* Get the owner of the key image, if any */
        std::tuple<bool, Crypto::PublicKey> getKeyImageOwner(
            const Crypto::KeyImage keyImage) const;

        std::string getDefaultChangeAddress() const;

        /* Get the sum of the balance of the subwallets pointed to. If
           takeFromAll, get the total balance from all subwallets. */
        std::tuple<uint64_t, uint64_t> getBalance(
            std::vector<Crypto::PublicKey> subWalletsToTakeFrom,
            const bool takeFromAll,
            const uint64_t currentHeight) const;

        /* Removes a spent key image from the store */
        void removeSpentKeyImage(
            const Crypto::KeyImage keyImage,
            const Crypto::PublicKey publicKey);

        /* Remove any transactions at this height or above, they were on a 
           forked chain */
        void removeForkedTransactions(uint64_t forkHeight);

        Crypto::SecretKey getPrivateViewKey() const;

        void markInputAsSpent(
            const Crypto::KeyImage keyImage,
            const Crypto::PublicKey publicKey,
            const uint64_t spendHeight);

        void markInputAsLocked(
            const Crypto::KeyImage keyImage,
            const Crypto::PublicKey publicKey);

        std::unordered_set<Crypto::Hash> getLockedTransactionsHashes() const;

        void removeCancelledTransactions(
            const std::unordered_set<Crypto::Hash> cancelledTransactions);

        bool isViewWallet() const;

        void reset(const uint64_t scanHeight);

        /////////////////////////////
        /* Public member variables */
        /////////////////////////////

        /* The public spend keys, used for verifying if a transaction is
           ours */
        std::vector<Crypto::PublicKey> m_publicSpendKeys;

    private:

        //////////////////////////////
        /* Private member functions */
        //////////////////////////////

        void throwIfViewWallet() const;

        //////////////////////////////
        /* Private member variables */
        //////////////////////////////

        /* The subwallets, indexed by public spend key */ 
        std::unordered_map<Crypto::PublicKey, SubWallet> m_subWallets;

        /* A vector of transactions */
        std::vector<WalletTypes::Transaction> m_transactions;

        /* Transactions which we sent, but haven't been added to a block yet */
        std::vector<WalletTypes::Transaction> m_lockedTransactions;

        Crypto::SecretKey m_privateViewKey;

        bool m_isViewWallet;

        /* Need a mutex for accessing inputs, transactions, and locked
           transactions, etc as these are modified on multiple threads */
        mutable std::mutex m_mutex;
};
