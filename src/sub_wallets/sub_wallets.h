// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <crypto/crypto.h>

#include <sub_wallets/sub_wallet.h>

class SubWallets
{
public:
    //////////////////
    /* Constructors */
    //////////////////

    SubWallets() = default;

    /* Creates a new wallet */
    SubWallets(
        const crypto::SecretKey privateSpendKey,
        const crypto::SecretKey privateViewKey,
        const std::string address,
        const uint64_t scanHeight,
        const bool newWallet);

    /* Creates a new view only subwallet */
    SubWallets(
        const crypto::SecretKey privateViewKey,
        const std::string address,
        const uint64_t scanHeight,
        const bool newWallet);

    /* Copy constructor */
    SubWallets(const SubWallets &other);

    /////////////////////////////
    /* Public member functions */
    /////////////////////////////

    /* Adds a sub wallet with a random spend key */
    std::tuple<Error, std::string, crypto::SecretKey> addSubWallet();

    /* Imports a sub wallet with the given private spend key */
    std::tuple<Error, std::string> importSubWallet(
        const crypto::SecretKey privateSpendKey,
        const uint64_t scanHeight);

    /* Imports a sub view only wallet with the given public spend key */
    std::tuple<Error, std::string> importViewSubWallet(
        const crypto::PublicKey privateSpendKey,
        const uint64_t scanHeight);

    Error deleteSubWallet(const std::string address);

    /* Returns (height, timestamp) to begin syncing at. Only one (if any)
       of the values will be non zero */
    std::tuple<uint64_t, uint64_t> getMinInitialSyncStart() const;

    /* Converts the class to a json object */
    void toJSON(rapidjson::Writer<rapidjson::StringBuffer> &writer) const;

    /* Initializes the class from a json string */
    void fromJSON(const JSONObject &j);

    /* Store a transaction */
    void addTransaction(const wallet_types::Transaction tx);

    /* Store an outgoing tx, not yet in a block */
    void addUnconfirmedTransaction(const wallet_types::Transaction tx);

    /* Generates a key image using the public+private spend key of the
       subwallet. Will return an uninitialized keyimage if a view wallet
       (and must exist, but the WalletSynchronizer already checks this) */
    crypto::KeyImage getTxInputKeyImage(
        const crypto::PublicKey publicSpendKey,
        const crypto::KeyDerivation derivation,
        const size_t outputIndex) const;

    void storeTransactionInput(
        const crypto::PublicKey publicSpendKey,
        const wallet_types::TransactionInput input);

    /* Get key images + amounts for the specified transfer amount. We
       can either take from all subwallets, or from some subset
       (usually just one address, e.g. if we're running a web wallet) */
    std::tuple<std::vector<wallet_types::TxInputAndOwner>, uint64_t>
    getTransactionInputsForAmount(
        const uint64_t amount,
        const bool takeFromAll,
        std::vector<crypto::PublicKey> subWalletsToTakeFrom,
        const uint64_t height) const;

    std::tuple<std::vector<wallet_types::TxInputAndOwner>, uint64_t, uint64_t>
    getFusionTransactionInputs(
        const bool takeFromAll,
        std::vector<crypto::PublicKey> subWalletsToTakeFrom,
        const uint64_t mixin,
        const uint64_t height) const;

    /* Get the owner of the key image, if any */
    std::tuple<bool, crypto::PublicKey> getKeyImageOwner(
        const crypto::KeyImage keyImage) const;

    /* Gets the primary address (normally first created) address */
    std::string getPrimaryAddress() const;

    /* Gets all the addresses in the subwallets container */
    std::vector<std::string> getAddresses() const;

    /* Gets the number of wallets in the container */
    uint64_t getWalletCount() const;

    /* Get the sum of the balance of the subwallets pointed to. If
       takeFromAll, get the total balance from all subwallets. */
    std::tuple<uint64_t, uint64_t> getBalance(
        std::vector<crypto::PublicKey> subWalletsToTakeFrom,
        const bool takeFromAll,
        const uint64_t currentHeight) const;

    /* Removes a spent key image from the store */
    void removeSpentKeyImage(
        const crypto::KeyImage keyImage,
        const crypto::PublicKey publicKey);

    /* Remove any transactions at this height or above, they were on a
       forked chain */
    void removeForkedTransactions(uint64_t forkHeight);

    crypto::SecretKey getPrivateViewKey() const;

    /* Gets the private spend key for the given public spend, if it exists */
    std::tuple<Error, crypto::SecretKey> getPrivateSpendKey(
        const crypto::PublicKey publicSpendKey) const;

    std::vector<crypto::SecretKey> getPrivateSpendKeys() const;

    crypto::SecretKey getPrimaryPrivateSpendKey() const;

    void markInputAsSpent(
        const crypto::KeyImage keyImage,
        const crypto::PublicKey publicKey,
        const uint64_t spendHeight);

    void markInputAsLocked(
        const crypto::KeyImage keyImage,
        const crypto::PublicKey publicKey);

    std::unordered_set<crypto::Hash> getLockedTransactionsHashes() const;

    void removeCancelledTransactions(
        const std::unordered_set<crypto::Hash> cancelledTransactions);

    bool isViewWallet() const;

    void reset(const uint64_t scanHeight);

    std::vector<wallet_types::Transaction> getTransactions() const;

    /* Note that this DOES NOT return incoming transactions in the pool. It only
       returns outgoing transactions which we sent but have not encountered in a
       block yet. */
    std::vector<wallet_types::Transaction> getUnconfirmedTransactions() const;

    std::tuple<Error, std::string> getAddress(
        const crypto::PublicKey spendKey) const;

    /* Store the private key used to create a transaction - can be used
       for auditing transactions */
    void storeTxPrivateKey(
        const crypto::SecretKey txPrivateKey,
        const crypto::Hash txHash);

    std::tuple<bool, crypto::SecretKey> getTxPrivateKey(
        const crypto::Hash txHash) const;

    void storeUnconfirmedIncomingInput(
        const wallet_types::UnconfirmedInput input,
        const crypto::PublicKey publicSpendKey);

    void convertSyncTimestampToHeight(
        const uint64_t timestamp,
        const uint64_t height);

    std::vector<std::tuple<std::string, uint64_t, uint64_t>> getBalances(
        const uint64_t currentHeight) const;

    /////////////////////////////
    /* Public member variables */
    /////////////////////////////

    /* The public spend keys, used for verifying if a transaction is
       ours */
    std::vector<crypto::PublicKey> m_publicSpendKeys;

private:
    //////////////////////////////
    /* Private member functions */
    //////////////////////////////

    void throwIfViewWallet() const;

    /* Deletes any transactions containing the given spend key, or just
       removes from the transfers array if there are multiple transfers
       in the tx */
    void deleteAddressTransactions(
        std::vector<wallet_types::Transaction> &txs,
        const crypto::PublicKey spendKey);

    //////////////////////////////
    /* Private member variables */
    //////////////////////////////

    /* The subwallets, indexed by public spend key */
    std::unordered_map<crypto::PublicKey, SubWallet> m_subWallets;

    /* A vector of transactions */
    std::vector<wallet_types::Transaction> m_transactions;

    /* Transactions which we sent, but haven't been added to a block yet */
    std::vector<wallet_types::Transaction> m_lockedTransactions;

    crypto::SecretKey m_privateViewKey;

    bool m_isViewWallet;

    /* Transaction private keys of sent transactions, used for auditing */
    std::unordered_map<crypto::Hash, crypto::SecretKey> m_transactionPrivateKeys;

    /* Need a mutex for accessing inputs, transactions, and locked
       transactions, etc as these are modified on multiple threads */
    mutable std::mutex m_mutex;
};
