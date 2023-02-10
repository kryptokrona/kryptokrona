// Copyright (c) 2018, The TurtleCoin Developers
//
// Please see the included LICENSE file for more information.

#include <cryptonote.h>

#include <cryptonote_core/cryptonote_format_utils.h>

#include <errors/errors.h>

#include <nigel/nigel.h>

#include <sub_wallets/sub_wallets.h>

#include <vector>

#include <wallet_types.h>

namespace SendTransaction
{
    std::tuple<Error, Crypto::Hash> sendFusionTransactionBasic(
        const std::shared_ptr<Nigel> daemon,
        const std::shared_ptr<SubWallets> subWallets);

    std::tuple<Error, Crypto::Hash> sendFusionTransactionAdvanced(
        const uint64_t mixin,
        const std::vector<std::string> addressesToTakeFrom,
        std::string destination,
        const std::shared_ptr<Nigel> daemon,
        const std::shared_ptr<SubWallets> subWallets);

    std::tuple<Error, Crypto::Hash> sendTransactionBasic(
        std::string destination,
        const uint64_t amount,
        std::string paymentID,
        const std::shared_ptr<Nigel> daemon,
        const std::shared_ptr<SubWallets> subWallets);

    std::tuple<Error, Crypto::Hash> sendTransactionAdvanced(
        std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
        const uint64_t mixin,
        const uint64_t fee,
        std::string paymentID,
        const std::vector<std::string> addressesToTakeFrom,
        std::string changeAddress,
        const std::shared_ptr<Nigel> daemon,
        const std::shared_ptr<SubWallets> subWallets,
        const uint64_t unlockTime);

    std::vector<WalletTypes::TransactionDestination> setupDestinations(
        std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
        const uint64_t changeRequired,
        const std::string changeAddress);

    std::tuple<Error, std::vector<WalletTypes::ObscuredInput>> prepareRingParticipants(
        std::vector<WalletTypes::TxInputAndOwner> sources,
        const uint64_t mixin,
        const std::shared_ptr<Nigel> daemon);

    std::tuple<Error, std::vector<cryptonote::KeyInput>, std::vector<Crypto::SecretKey>> setupInputs(
        const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
        const Crypto::SecretKey privateViewKey);

    std::tuple<std::vector<WalletTypes::KeyOutput>, cryptonote::KeyPair> setupOutputs(
        std::vector<WalletTypes::TransactionDestination> destinations);

    std::tuple<Error, cryptonote::Transaction> generateRingSignatures(
        cryptonote::Transaction tx,
        const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
        const std::vector<Crypto::SecretKey> tmpSecretKeys);

    std::vector<uint64_t> splitAmountIntoDenominations(uint64_t amount);

    std::vector<cryptonote::TransactionInput> keyInputToTransactionInput(
        const std::vector<cryptonote::KeyInput> keyInputs);

    std::vector<cryptonote::TransactionOutput> keyOutputToTransactionOutput(
        const std::vector<WalletTypes::KeyOutput> keyOutputs);

    Crypto::Hash getTransactionHash(cryptonote::Transaction tx);

    std::tuple<Error, std::vector<cryptonote::RandomOuts>> getRingParticipants(
        const uint64_t mixin,
        const std::shared_ptr<Nigel> daemon,
        const std::vector<WalletTypes::TxInputAndOwner> sources);

    struct TransactionResult
    {
        /* The error, if any */
        Error error;

        /* The raw transaction */
        cryptonote::Transaction transaction;

        /* The transaction outputs, before converted into boost uglyness, used
           for determining key inputs from the tx that belong to us */
        std::vector<WalletTypes::KeyOutput> outputs;

        /* The random key pair we generated */
        cryptonote::KeyPair txKeyPair;
    };

    TransactionResult makeTransaction(
        const uint64_t mixin,
        const std::shared_ptr<Nigel> daemon,
        const std::vector<WalletTypes::TxInputAndOwner> ourInputs,
        const std::string paymentID,
        const std::vector<WalletTypes::TransactionDestination> destinations,
        const std::shared_ptr<SubWallets> subWallets,
        const uint64_t unlockTime);

    std::tuple<Error, Crypto::Hash> relayTransaction(
        const cryptonote::Transaction tx,
        const std::shared_ptr<Nigel> daemon);

    std::tuple<cryptonote::KeyPair, Crypto::KeyImage> genKeyImage(
        const WalletTypes::ObscuredInput input,
        const Crypto::SecretKey privateViewKey);

    void storeSentTransaction(
        const Crypto::Hash hash,
        const uint64_t fee,
        const std::string paymentID,
        const std::vector<WalletTypes::TxInputAndOwner> ourInputs,
        const std::string changeAddress,
        const uint64_t changeRequired,
        const std::shared_ptr<SubWallets> subWallets);

    Error isTransactionPayloadTooBig(
        const cryptonote::Transaction tx,
        const uint64_t currentHeight);

    void storeUnconfirmedIncomingInputs(
        const std::shared_ptr<SubWallets> subWallets,
        const std::vector<WalletTypes::KeyOutput> keyOutputs,
        const Crypto::PublicKey txPublicKey,
        const Crypto::Hash txHash);

    /* Verify all amounts in the transaction given are PRETTY_AMOUNTS */
    bool verifyAmounts(const cryptonote::Transaction tx);

    /* Verify all amounts given are PRETTY_AMOUNTS */
    bool verifyAmounts(const std::vector<uint64_t> amounts);

    /* Verify fee is as expected */
    bool verifyTransactionFee(const uint64_t expectedFee, cryptonote::Transaction tx);
}
