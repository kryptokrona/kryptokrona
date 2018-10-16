// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#include <CryptoNote.h>

#include <CryptoNoteCore/CryptoNoteFormatUtils.h>

#include <NodeRpcProxy/NodeRpcProxy.h>

#include <vector>

#include <WalletTypes.h>

#include <WalletBackend/SubWallets.h>
#include <WalletBackend/WalletErrors.h>

namespace SendTransaction
{
    std::tuple<WalletError, Crypto::Hash> sendTransactionBasic(
        std::string destination,
        const uint64_t amount,
        std::string paymentID,
        const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
        const std::shared_ptr<SubWallets> subWallets);

    std::tuple<WalletError, Crypto::Hash> sendTransactionAdvanced(
        std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
        const uint64_t mixin,
        const uint64_t fee,
        std::string paymentID,
        const std::vector<std::string> addressesToTakeFrom,
        const std::string changeAddress,
        const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon,
        const std::shared_ptr<SubWallets> subWallets);

    std::vector<WalletTypes::TransactionDestination> setupDestinations(
        std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
        const uint64_t changeRequired,
        const std::string changeAddress);

    std::vector<WalletTypes::ObscuredInput> setupFakeInputs(
        std::vector<WalletTypes::TxInputAndOwner> sources,
        const uint64_t mixin,
        const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon);

    std::tuple<std::vector<CryptoNote::KeyInput>, std::vector<Crypto::SecretKey>> setupInputs(
        const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
        const Crypto::SecretKey privateViewKey);

    std::tuple<std::vector<WalletTypes::KeyOutput>, Crypto::PublicKey> setupOutputs(
        std::vector<WalletTypes::TransactionDestination> destinations);

    CryptoNote::Transaction generateRingSignatures(
        CryptoNote::Transaction tx,
        const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
        const std::vector<Crypto::SecretKey> tmpSecretKeys);

    std::vector<uint64_t> splitAmountIntoDenominations(uint64_t amount);

    std::vector<CryptoNote::TransactionInput> keyInputToTransactionInput(
        const std::vector<CryptoNote::KeyInput> keyInputs);

    std::vector<CryptoNote::TransactionOutput> keyOutputToTransactionOutput(
        const std::vector<WalletTypes::KeyOutput> keyOutputs);

    Crypto::Hash getTransactionHash(CryptoNote::Transaction tx);
}
