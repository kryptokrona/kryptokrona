// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <exception>
#include <limits>
#include <vector>

#include "serialization/iserializer.h"

namespace payment_service
{

    /* Forward declaration to avoid circular dependency from including "WalletService.h" */
    class WalletService;

    class RequestSerializationError : public std::exception
    {

    public:
        virtual const char *what() const throw() override { return "Request error"; }
    };

    struct Save
    {
        struct Request
        {
            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct Export
    {
        struct Request
        {
            std::string fileName;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct Reset
    {
        struct Request
        {
            std::string viewSecretKey;

            uint64_t scanHeight = 0;

            bool newAddress = false;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetViewKey
    {
        struct Request
        {
            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::string viewSecretKey;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetMnemonicSeed
    {
        struct Request
        {
            std::string address;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::string mnemonicSeed;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetStatus
    {
        struct Request
        {
            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            uint32_t blockCount;
            uint32_t knownBlockCount;
            uint64_t localDaemonBlockCount;
            std::string lastBlockHash;
            uint32_t peerCount;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetAddresses
    {
        struct Request
        {
            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::vector<std::string> addresses;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct CreateAddress
    {
        struct Request
        {
            std::string spendSecretKey;
            std::string spendPublicKey;

            uint64_t scanHeight = 0;

            bool newAddress = false;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::string address;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct CreateAddressList
    {
        struct Request
        {
            std::vector<std::string> spendSecretKeys;

            uint64_t scanHeight = 0;

            bool newAddress = false;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::vector<std::string> addresses;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct DeleteAddress
    {
        struct Request
        {
            std::string address;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetSpendKeys
    {
        struct Request
        {
            std::string address;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::string spendSecretKey;
            std::string spendPublicKey;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetBalance
    {
        struct Request
        {
            std::string address;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            uint64_t availableBalance;
            uint64_t lockedAmount;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetBlockHashes
    {
        struct Request
        {
            uint32_t firstBlockIndex;
            uint32_t blockCount;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::vector<std::string> blockHashes;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct TransactionHashesInBlockRpcInfo
    {
        std::string blockHash;
        std::vector<std::string> transactionHashes;

        void serialize(cryptonote::ISerializer &serializer);
    };

    struct GetTransactionHashes
    {
        struct Request
        {
            std::vector<std::string> addresses;
            std::string blockHash;
            uint32_t firstBlockIndex = std::numeric_limits<uint32_t>::max();
            uint32_t blockCount;
            std::string paymentId;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::vector<TransactionHashesInBlockRpcInfo> items;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct TransferRpcInfo
    {
        uint8_t type;
        std::string address;
        int64_t amount;

        void serialize(cryptonote::ISerializer &serializer);
    };

    struct TransactionRpcInfo
    {
        uint8_t state;
        std::string transactionHash;
        uint32_t blockIndex;
        uint64_t timestamp;
        bool isBase;
        uint64_t unlockTime;
        int64_t amount;
        uint64_t fee;
        std::vector<TransferRpcInfo> transfers;
        std::string extra;
        std::string paymentId;

        void serialize(cryptonote::ISerializer &serializer);
    };

    struct GetTransaction
    {
        struct Request
        {
            std::string transactionHash;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            TransactionRpcInfo transaction;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct TransactionsInBlockRpcInfo
    {
        std::string blockHash;
        std::vector<TransactionRpcInfo> transactions;

        void serialize(cryptonote::ISerializer &serializer);
    };

    struct GetTransactions
    {
        struct Request
        {
            std::vector<std::string> addresses;
            std::string blockHash;
            uint32_t firstBlockIndex = std::numeric_limits<uint32_t>::max();
            uint32_t blockCount;
            std::string paymentId;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::vector<TransactionsInBlockRpcInfo> items;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetUnconfirmedTransactionHashes
    {
        struct Request
        {
            std::vector<std::string> addresses;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::vector<std::string> transactionHashes;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct WalletRpcOrder
    {
        std::string address;
        uint64_t amount;

        void serialize(cryptonote::ISerializer &serializer);
    };

    struct SendTransaction
    {
        struct Request
        {
            std::vector<std::string> sourceAddresses;
            std::vector<WalletRpcOrder> transfers;
            std::string changeAddress;
            uint64_t fee = 0;
            uint64_t anonymity;
            std::string extra;
            std::string paymentId;
            uint64_t unlockTime = 0;

            void serialize(cryptonote::ISerializer &serializer, const WalletService &service);
        };

        struct Response
        {
            std::string transactionHash;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct CreateDelayedTransaction
    {
        struct Request
        {
            std::vector<std::string> addresses;
            std::vector<WalletRpcOrder> transfers;
            std::string changeAddress;
            uint64_t fee = 0;
            uint64_t anonymity;
            std::string extra;
            std::string paymentId;
            uint64_t unlockTime = 0;

            void serialize(cryptonote::ISerializer &serializer, const WalletService &service);
        };

        struct Response
        {
            std::string transactionHash;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct GetDelayedTransactionHashes
    {
        struct Request
        {
            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::vector<std::string> transactionHashes;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct DeleteDelayedTransaction
    {
        struct Request
        {
            std::string transactionHash;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct SendDelayedTransaction
    {
        struct Request
        {
            std::string transactionHash;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct SendFusionTransaction
    {
        struct Request
        {
            uint64_t threshold;
            uint64_t anonymity;
            std::vector<std::string> addresses;
            std::string destinationAddress;

            void serialize(cryptonote::ISerializer &serializer, const WalletService &service);
        };

        struct Response
        {
            std::string transactionHash;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct EstimateFusion
    {
        struct Request
        {
            uint64_t threshold;
            std::vector<std::string> addresses;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            uint32_t fusionReadyCount;
            uint32_t totalOutputCount;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct CreateIntegratedAddress
    {
        struct Request
        {
            std::string address;
            std::string paymentId;

            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::string integratedAddress;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

    struct NodeFeeInfo
    {
        struct Request
        {
            void serialize(cryptonote::ISerializer &serializer);
        };

        struct Response
        {
            std::string address;
            uint32_t amount;

            void serialize(cryptonote::ISerializer &serializer);
        };
    };

} // namespace payment_service
