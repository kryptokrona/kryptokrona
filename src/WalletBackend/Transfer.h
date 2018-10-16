// Copyright (c) 2018, The TurtleCoin Developers
// 
// Please see the included LICENSE file for more information.

#include <CryptoNote.h>

#include <CryptoNoteCore/CryptoNoteFormatUtils.h>

#include <NodeRpcProxy/NodeRpcProxy.h>

#include <vector>

#include <WalletTypes.h>

std::vector<WalletTypes::TransactionDestination> setupDestinations(
    std::vector<std::pair<std::string, uint64_t>> addressesAndAmounts,
    const uint64_t changeRequired,
    const std::string changeAddress);

std::vector<WalletTypes::ObscuredInput> setupFakeInputs(
    const std::vector<WalletTypes::TxInputAndOwner> sources,
    const uint64_t mixin,
    const std::shared_ptr<CryptoNote::NodeRpcProxy> daemon);

std::tuple<std::vector<CryptoNote::KeyInput>, std::vector<Crypto::SecretKey>> setupInputs(
    const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
    const Crypto::SecretKey privateViewKey);

std::tuple<std::vector<WalletTypes::KeyOutput>, Crypto::PublicKey> setupOutputs(
    const std::vector<WalletTypes::TransactionDestination> destinations);

CryptoNote::Transaction generateRingSignatures(
    CryptoNote::Transaction tx,
    const std::vector<WalletTypes::ObscuredInput> inputsAndFakes,
    const std::vector<Crypto::SecretKey> tmpSecretKeys);
