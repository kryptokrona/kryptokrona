// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "iwallet.h"

#include <queue>
#include <unordered_map>

#include "ifusion_manager.h"
#include "wallet_indices.h"

#include "logging/logger_ref.h"
#include <syst/dispatcher.h>
#include <syst/event.h>
#include "transfers/transfers_synchronizer.h"
#include "transfers/blockchain_synchronizer.h"

namespace cryptonote
{

    struct PreparedTransaction
    {
        std::shared_ptr<ITransaction> transaction;
        std::vector<WalletTransfer> destinations;
        uint64_t neededMoney;
        uint64_t changeAmount;
    };

    class WalletGreen : public IWallet,
                        ITransfersObserver,
                        IBlockchainSynchronizerObserver,
                        ITransfersSynchronizerObserver,
                        public IFusionManager
    {
    public:
        WalletGreen(syst::Dispatcher &dispatcher, const Currency &currency, INode &node, std::shared_ptr<logging::ILogger> logger, uint32_t transactionSoftLockTime = 1);
        virtual ~WalletGreen();

        virtual void initializeWithViewKey(const std::string &path, const std::string &password, const crypto::SecretKey &viewSecretKey, const uint64_t scanHeight, const bool newAddress) override;
        virtual void load(const std::string &path, const std::string &password, std::string &extra) override;
        virtual void load(const std::string &path, const std::string &password) override;
        virtual void shutdown() override;

        virtual void changePassword(const std::string &oldPassword, const std::string &newPassword) override;
        virtual void save(WalletSaveLevel saveLevel = WalletSaveLevel::SAVE_ALL, const std::string &extra = "") override;
        virtual void reset(const uint64_t scanHeight) override;
        virtual void exportWallet(const std::string &path, bool encrypt = true, WalletSaveLevel saveLevel = WalletSaveLevel::SAVE_ALL, const std::string &extra = "") override;

        virtual size_t getAddressCount() const override;
        virtual std::string getAddress(size_t index) const override;
        virtual KeyPair getAddressSpendKey(size_t index) const override;
        virtual KeyPair getAddressSpendKey(const std::string &address) const override;
        virtual KeyPair getViewKey() const override;

        virtual std::string createAddress() override;
        virtual std::string createAddress(const crypto::SecretKey &spendSecretKey, const uint64_t scanHeight, const bool newAddress) override;
        virtual std::string createAddress(const crypto::PublicKey &spendPublicKey, const uint64_t scanHeight, const bool newAddress) override;

        virtual std::vector<std::string> createAddressList(const std::vector<crypto::SecretKey> &spendSecretKeys, const uint64_t scanHeight, const bool newAddress) override;

        virtual void deleteAddress(const std::string &address) override;

        virtual uint64_t getActualBalance() const override;
        virtual uint64_t getActualBalance(const std::string &address) const override;
        virtual uint64_t getPendingBalance() const override;
        virtual uint64_t getPendingBalance(const std::string &address) const override;

        virtual size_t getTransactionCount() const override;
        virtual WalletTransaction getTransaction(size_t transactionIndex) const override;

        virtual WalletTransactionWithTransfers getTransaction(const crypto::Hash &transactionHash) const override;
        virtual std::vector<TransactionsInBlockInfo> getTransactions(const crypto::Hash &blockHash, size_t count) const override;
        virtual std::vector<TransactionsInBlockInfo> getTransactions(uint32_t blockIndex, size_t count) const override;
        virtual std::vector<crypto::Hash> getBlockHashes(uint32_t blockIndex, size_t count) const override;
        virtual uint32_t getBlockCount() const override;
        virtual std::vector<WalletTransactionWithTransfers> getUnconfirmedTransactions() const override;
        virtual std::vector<size_t> getDelayedTransactionIds() const override;

        virtual size_t transfer(const TransactionParameters &transactionParameters) override;

        virtual size_t makeTransaction(const TransactionParameters &sendingTransaction) override;
        virtual void commitTransaction(size_t) override;
        virtual void rollbackUncommitedTransaction(size_t) override;

        size_t transfer(const PreparedTransaction &preparedTransaction);
        bool txIsTooLarge(const PreparedTransaction &p);
        size_t getTxSize(const PreparedTransaction &p);
        size_t getMaxTxSize();
        PreparedTransaction formTransaction(const TransactionParameters &sendingTransaction);
        void updateInternalCache();
        void clearCaches(bool clearTransactions, bool clearCachedData);
        void createViewWallet(const std::string &path, const std::string &password,
                              const std::string address,
                              const crypto::SecretKey &viewSecretKey,
                              const uint64_t scanHeight,
                              const bool newAddress);
        uint64_t getBalanceMinusDust(const std::vector<std::string> &addresses);

        virtual void start() override;
        virtual void stop() override;
        virtual WalletEvent getEvent() override;

        virtual size_t createFusionTransaction(uint64_t threshold, uint16_t mixin,
                                               const std::vector<std::string> &sourceAddresses = {}, const std::string &destinationAddress = "") override;
        virtual bool isFusionTransaction(size_t transactionId) const override;
        virtual IFusionManager::EstimateResult estimate(uint64_t threshold, const std::vector<std::string> &sourceAddresses = {}) const override;

    protected:
        struct NewAddressData
        {
            crypto::PublicKey spendPublicKey;
            crypto::SecretKey spendSecretKey;
        };

        void throwIfNotInitialized() const;
        void throwIfStopped() const;
        void throwIfTrackingMode() const;
        void doShutdown();
        void convertAndLoadWalletFile(const std::string &path, std::ifstream &&walletFileStream);
        static void decryptKeyPair(const EncryptedWalletRecord &cipher, crypto::PublicKey &publicKey, crypto::SecretKey &secretKey,
                                   uint64_t &creationTimestamp, const crypto::chacha8_key &key);
        void decryptKeyPair(const EncryptedWalletRecord &cipher, crypto::PublicKey &publicKey, crypto::SecretKey &secretKey, uint64_t &creationTimestamp) const;
        static EncryptedWalletRecord encryptKeyPair(const crypto::PublicKey &publicKey, const crypto::SecretKey &secretKey, uint64_t creationTimestamp,
                                                    const crypto::chacha8_key &key, const crypto::chacha8_iv &iv);
        EncryptedWalletRecord encryptKeyPair(const crypto::PublicKey &publicKey, const crypto::SecretKey &secretKey, uint64_t creationTimestamp) const;
        crypto::chacha8_iv getNextIv() const;
        static void incIv(crypto::chacha8_iv &iv);
        void incNextIv();
        void initWithKeys(const std::string &path, const std::string &password, const crypto::PublicKey &viewPublicKey, const crypto::SecretKey &viewSecretKey, const uint64_t scanHeight, const bool newAddress);
        std::string doCreateAddress(const crypto::PublicKey &spendPublicKey, const crypto::SecretKey &spendSecretKey, const uint64_t scanHeight, const bool newAddress);
        std::vector<std::string> doCreateAddressList(const std::vector<NewAddressData> &addressDataList, const uint64_t scanHeight, const bool newAddress);

        cryptonote::BlockDetails getBlock(const uint64_t blockHeight);

        uint64_t scanHeightToTimestamp(const uint64_t scanHeight);

        uint64_t getCurrentTimestampAdjusted();

        struct InputInfo
        {
            TransactionTypes::InputKeyInfo keyInfo;
            WalletRecord *walletRecord = nullptr;
            KeyPair ephKeys;
        };

        struct OutputToTransfer
        {
            TransactionOutputInformation out;
            WalletRecord *wallet;
        };

        struct ReceiverAmounts
        {
            cryptonote::AccountPublicAddress receiver;
            std::vector<uint64_t> amounts;
        };

        struct WalletOuts
        {
            WalletRecord *wallet;
            std::vector<TransactionOutputInformation> outs;
        };

        typedef std::pair<WalletTransfers::const_iterator, WalletTransfers::const_iterator> TransfersRange;

        struct AddressAmounts
        {
            int64_t input = 0;
            int64_t output = 0;
        };

        struct ContainerAmounts
        {
            ITransfersContainer *container;
            AddressAmounts amounts;
        };

#pragma pack(push, 1)
        struct ContainerStoragePrefix
        {
            uint8_t version;
            crypto::chacha8_iv nextIv;
            EncryptedWalletRecord encryptedViewKeys;
        };
#pragma pack(pop)

        typedef std::unordered_map<std::string, AddressAmounts> TransfersMap;

        virtual void onError(ITransfersSubscription *object, uint32_t height, std::error_code ec) override;

        virtual void onTransactionUpdated(ITransfersSubscription *object, const crypto::Hash &transactionHash) override;
        virtual void onTransactionUpdated(const crypto::PublicKey &viewPublicKey, const crypto::Hash &transactionHash,
                                          const std::vector<ITransfersContainer *> &containers) override;
        void transactionUpdated(const TransactionInformation &transactionInfo, const std::vector<ContainerAmounts> &containerAmountsList);

        virtual void onTransactionDeleted(ITransfersSubscription *object, const crypto::Hash &transactionHash) override;
        void transactionDeleted(ITransfersSubscription *object, const crypto::Hash &transactionHash);

        virtual void synchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount) override;
        virtual void synchronizationCompleted(std::error_code result) override;

        void onSynchronizationProgressUpdated(uint32_t processedBlockCount, uint32_t totalBlockCount);
        void onSynchronizationCompleted();

        virtual void onBlocksAdded(const crypto::PublicKey &viewPublicKey, const std::vector<crypto::Hash> &blockHashes) override;
        void blocksAdded(const std::vector<crypto::Hash> &blockHashes);

        virtual void onBlockchainDetach(const crypto::PublicKey &viewPublicKey, uint32_t blockIndex) override;
        void blocksRollback(uint32_t blockIndex);

        virtual void onTransactionDeleteBegin(const crypto::PublicKey &viewPublicKey, crypto::Hash transactionHash) override;
        void transactionDeleteBegin(crypto::Hash transactionHash);

        virtual void onTransactionDeleteEnd(const crypto::PublicKey &viewPublicKey, crypto::Hash transactionHash) override;
        void transactionDeleteEnd(crypto::Hash transactionHash);

        std::vector<WalletOuts> pickWalletsWithMoney() const;
        WalletOuts pickWallet(const std::string &address) const;
        std::vector<WalletOuts> pickWallets(const std::vector<std::string> &addresses) const;

        void updateBalance(cryptonote::ITransfersContainer *container);
        void unlockBalances(uint32_t height);

        const WalletRecord &getWalletRecord(const crypto::PublicKey &key) const;
        const WalletRecord &getWalletRecord(const std::string &address) const;
        const WalletRecord &getWalletRecord(cryptonote::ITransfersContainer *container) const;

        cryptonote::AccountPublicAddress parseAddress(const std::string &address) const;
        std::string addWallet(const NewAddressData &addressData, uint64_t scanHeight, bool newAddress);
        AccountKeys makeAccountKeys(const WalletRecord &wallet) const;
        size_t getTransactionId(const crypto::Hash &transactionHash) const;
        void pushEvent(const WalletEvent &event);
        bool isFusionTransaction(const WalletTransaction &walletTx) const;

        void prepareTransaction(std::vector<WalletOuts> &&wallets,
                                const std::vector<WalletOrder> &orders,
                                uint64_t fee,
                                uint16_t mixIn,
                                const std::string &extra,
                                uint64_t unlockTimestamp,
                                const DonationSettings &donation,
                                const cryptonote::AccountPublicAddress &changeDestinationAddress,
                                PreparedTransaction &preparedTransaction);

        size_t doTransfer(const TransactionParameters &transactionParameters);

        void checkIfEnoughMixins(std::vector<cryptonote::RandomOuts> &mixinResult, uint16_t mixIn) const;
        std::vector<WalletTransfer> convertOrdersToTransfers(const std::vector<WalletOrder> &orders) const;
        uint64_t countNeededMoney(const std::vector<cryptonote::WalletTransfer> &destinations, uint64_t fee) const;
        cryptonote::AccountPublicAddress parseAccountAddressString(const std::string &addressString) const;
        uint64_t pushDonationTransferIfPossible(const DonationSettings &donation, uint64_t freeAmount, uint64_t dustThreshold, std::vector<WalletTransfer> &destinations) const;
        void validateAddresses(const std::vector<std::string> &addresses) const;
        void validateOrders(const std::vector<WalletOrder> &orders) const;
        void validateChangeDestination(const std::vector<std::string> &sourceAddresses, const std::string &changeDestination, bool isFusion) const;
        void validateSourceAddresses(const std::vector<std::string> &sourceAddresses) const;
        void validateTransactionParameters(const TransactionParameters &transactionParameters) const;

        void requestMixinOuts(const std::vector<OutputToTransfer> &selectedTransfers,
                              uint16_t mixIn,
                              std::vector<cryptonote::RandomOuts> &mixinResult);

        void prepareInputs(const std::vector<OutputToTransfer> &selectedTransfers,
                           std::vector<cryptonote::RandomOuts> &mixinResult,
                           uint16_t mixIn,
                           std::vector<InputInfo> &keysInfo);

        uint64_t selectTransfers(uint64_t needeMoney,
                                 bool dust,
                                 uint64_t dustThreshold,
                                 std::vector<WalletOuts> &&wallets,
                                 std::vector<OutputToTransfer> &selectedTransfers);

        std::vector<ReceiverAmounts> splitDestinations(const std::vector<WalletTransfer> &destinations,
                                                       uint64_t dustThreshold, const Currency &currency);
        ReceiverAmounts splitAmount(uint64_t amount, const AccountPublicAddress &destination, uint64_t dustThreshold);

        std::unique_ptr<cryptonote::ITransaction> makeTransaction(const std::vector<ReceiverAmounts> &decomposedOutputs,
                                                                  std::vector<InputInfo> &keysInfo, const std::string &extra, uint64_t unlockTimestamp);

        void sendTransaction(const cryptonote::Transaction &cryptoNoteTransaction);
        size_t validateSaveAndSendTransaction(const ITransactionReader &transaction, const std::vector<WalletTransfer> &destinations, bool isFusion, bool send);

        size_t insertBlockchainTransaction(const TransactionInformation &info, int64_t txBalance);
        size_t insertOutgoingTransactionAndPushEvent(const crypto::Hash &transactionHash, uint64_t fee, const BinaryArray &extra, uint64_t unlockTimestamp);
        void updateTransactionStateAndPushEvent(size_t transactionId, WalletTransactionState state);
        bool updateWalletTransactionInfo(size_t transactionId, const cryptonote::TransactionInformation &info, int64_t totalAmount);
        bool updateTransactionTransfers(size_t transactionId, const std::vector<ContainerAmounts> &containerAmountsList,
                                        int64_t allInputsAmount, int64_t allOutputsAmount);
        TransfersMap getKnownTransfersMap(size_t transactionId, size_t firstTransferIdx) const;
        bool updateAddressTransfers(size_t transactionId, size_t firstTransferIdx, const std::string &address, int64_t knownAmount, int64_t targetAmount);
        bool updateUnknownTransfers(size_t transactionId, size_t firstTransferIdx, const std::unordered_set<std::string> &myAddresses,
                                    int64_t knownAmount, int64_t myAmount, int64_t totalAmount, bool isOutput);
        void appendTransfer(size_t transactionId, size_t firstTransferIdx, const std::string &address, int64_t amount);
        bool adjustTransfer(size_t transactionId, size_t firstTransferIdx, const std::string &address, int64_t amount);
        bool eraseTransfers(size_t transactionId, size_t firstTransferIdx, std::function<bool(bool, const std::string &)> &&predicate);
        bool eraseTransfersByAddress(size_t transactionId, size_t firstTransferIdx, const std::string &address, bool eraseOutputTransfers);
        bool eraseForeignTransfers(size_t transactionId, size_t firstTransferIdx, const std::unordered_set<std::string> &knownAddresses, bool eraseOutputTransfers);
        void pushBackOutgoingTransfers(size_t txId, const std::vector<WalletTransfer> &destinations);
        void insertUnlockTransactionJob(const crypto::Hash &transactionHash, uint32_t blockHeight, cryptonote::ITransfersContainer *container);
        void deleteUnlockTransactionJob(const crypto::Hash &transactionHash);
        void startBlockchainSynchronizer();
        void stopBlockchainSynchronizer();
        void addUnconfirmedTransaction(const ITransactionReader &transaction);
        void removeUnconfirmedTransaction(const crypto::Hash &transactionHash);

        void copyContainerStorageKeys(ContainerStorage &src, const crypto::chacha8_key &srcKey, ContainerStorage &dst, const crypto::chacha8_key &dstKey);
        static void copyContainerStoragePrefix(ContainerStorage &src, const crypto::chacha8_key &srcKey, ContainerStorage &dst, const crypto::chacha8_key &dstKey);
        void deleteOrphanTransactions(const std::unordered_set<crypto::PublicKey> &deletedKeys);
        static void encryptAndSaveContainerData(ContainerStorage &storage, const crypto::chacha8_key &key, const void *containerData, size_t containerDataSize);
        static void loadAndDecryptContainerData(ContainerStorage &storage, const crypto::chacha8_key &key, BinaryArray &containerData);
        void initTransactionPool();
        void loadSpendKeys();
        void loadContainerStorage(const std::string &path);
        void loadWalletCache(std::unordered_set<crypto::PublicKey> &addedKeys, std::unordered_set<crypto::PublicKey> &deletedKeys, std::string &extra);
        void saveWalletCache(ContainerStorage &storage, const crypto::chacha8_key &key, WalletSaveLevel saveLevel, const std::string &extra);
        void subscribeWallets();

        std::vector<OutputToTransfer> pickRandomFusionInputs(const std::vector<std::string> &addresses,
                                                             uint64_t threshold, size_t minInputCount, size_t maxInputCount);
        static ReceiverAmounts decomposeFusionOutputs(const AccountPublicAddress &address, uint64_t inputsAmount);

        enum class WalletState
        {
            INITIALIZED,
            NOT_INITIALIZED
        };

        enum class WalletTrackingMode
        {
            TRACKING,
            NOT_TRACKING,
            NO_ADDRESSES
        };

        WalletTrackingMode getTrackingMode() const;

        TransfersRange getTransactionTransfersRange(size_t transactionIndex) const;
        std::vector<TransactionsInBlockInfo> getTransactionsInBlocks(uint32_t blockIndex, size_t count) const;
        crypto::Hash getBlockHashByIndex(uint32_t blockIndex) const;

        std::vector<WalletTransfer> getTransactionTransfers(const WalletTransaction &transaction) const;
        void filterOutTransactions(WalletTransactions &transactions, WalletTransfers &transfers, std::function<bool(const WalletTransaction &)> &&pred) const;
        void initBlockchain(const crypto::PublicKey &viewPublicKey);
        cryptonote::AccountPublicAddress getChangeDestination(const std::string &changeDestinationAddress, const std::vector<std::string> &sourceAddresses) const;
        bool isMyAddress(const std::string &address) const;

        void deleteContainerFromUnlockTransactionJobs(const ITransfersContainer *container);
        std::vector<size_t> deleteTransfersForAddress(const std::string &address, std::vector<size_t> &deletedTransactions);
        void deleteFromUncommitedTransactions(const std::vector<size_t> &deletedTransactions);

        syst::Dispatcher &m_dispatcher;
        const Currency &m_currency;
        INode &m_node;
        mutable logging::LoggerRef m_logger;
        bool m_stopped;

        WalletsContainer m_walletsContainer;
        ContainerStorage m_containerStorage;
        UnlockTransactionJobs m_unlockTransactionsJob;
        WalletTransactions m_transactions;
        WalletTransfers m_transfers;                               // sorted
        mutable std::unordered_map<size_t, bool> m_fusionTxsCache; // txIndex -> isFusion
        UncommitedTransactions m_uncommitedTransactions;

        bool m_blockchainSynchronizerStarted;
        BlockchainSynchronizer m_blockchainSynchronizer;
        TransfersSyncronizer m_synchronizer;

        syst::Event m_eventOccurred;
        std::queue<WalletEvent> m_events;
        mutable syst::Event m_readyEvent;

        WalletState m_state;

        std::string m_password;
        crypto::chacha8_key m_key;
        std::string m_path;
        std::string m_extra; // workaround for wallet reset

        crypto::PublicKey m_viewPublicKey;
        crypto::SecretKey m_viewSecretKey;

        uint64_t m_actualBalance;
        uint64_t m_pendingBalance;

        uint32_t m_transactionSoftLockTime;

        BlockHashesContainer m_blockchain;

        friend std::ostream &operator<<(std::ostream &os, cryptonote::WalletGreen::WalletState state);
        friend std::ostream &operator<<(std::ostream &os, cryptonote::WalletGreen::WalletTrackingMode mode);
        friend class TransferListFormatter;
    };

} // namespace cryptonote
