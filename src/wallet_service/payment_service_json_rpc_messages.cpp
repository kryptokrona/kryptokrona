// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "payment_service_json_rpc_messages.h"
#include "serialization/serialization_overloads.h"
#include "wallet_service.h"

namespace payment_service
{

    void Save::Request::serialize(cryptonote::ISerializer & /*serializer*/)
    {
    }

    void Save::Response::serialize(cryptonote::ISerializer & /*serializer*/)
    {
    }

    void Export::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(fileName, "fileName"))
        {
            throw RequestSerializationError();
        }
    }

    void Export::Response::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void Reset::Request::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(scanHeight, "scanHeight");
    }

    void Reset::Response::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void GetViewKey::Request::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void GetViewKey::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(viewSecretKey, "viewSecretKey");
    }

    void GetMnemonicSeed::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(address, "address"))
        {
            throw RequestSerializationError();
        }
    }

    void GetMnemonicSeed::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(mnemonicSeed, "mnemonicSeed");
    }

    void GetStatus::Request::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void GetStatus::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(blockCount, "blockCount");
        serializer(knownBlockCount, "knownBlockCount");
        serializer(localDaemonBlockCount, "localDaemonBlockCount");
        serializer(lastBlockHash, "lastBlockHash");
        serializer(peerCount, "peerCount");
    }

    void GetAddresses::Request::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void GetAddresses::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(addresses, "addresses");
    }

    void CreateAddress::Request::serialize(cryptonote::ISerializer &serializer)
    {
        bool hasSecretKey = serializer(spendSecretKey, "spendSecretKey");
        bool hasPublicKey = serializer(spendPublicKey, "spendPublicKey");

        bool hasNewAddress = serializer(newAddress, "newAddress");
        bool hasScanHeight = serializer(scanHeight, "scanHeight");

        if (hasSecretKey && hasPublicKey)
        {
            // TODO: replace it with error codes
            throw RequestSerializationError();
        }

        /* Can't specify both that it is a new address, and a height to begin
           scanning from */
        if (hasNewAddress && hasScanHeight)
        {
            throw RequestSerializationError();
        }
    }

    void CreateAddress::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(address, "address");
    }

    void CreateAddressList::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(spendSecretKeys, "spendSecretKeys"))
        {
            // TODO: replace it with error codes
            throw RequestSerializationError();
        }

        bool hasNewAddress = serializer(newAddress, "newAddress");
        bool hasScanHeight = serializer(scanHeight, "scanHeight");

        /* Can't specify both that it is a new address, and a height to begin
           scanning from */
        if (hasNewAddress && hasScanHeight)
        {
            throw RequestSerializationError();
        }
    }

    void CreateAddressList::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(addresses, "addresses");
    }

    void DeleteAddress::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(address, "address"))
        {
            throw RequestSerializationError();
        }
    }

    void DeleteAddress::Response::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void GetSpendKeys::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(address, "address"))
        {
            throw RequestSerializationError();
        }
    }

    void GetSpendKeys::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(spendSecretKey, "spendSecretKey");
        serializer(spendPublicKey, "spendPublicKey");
    }

    void GetBalance::Request::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(address, "address");
    }

    void GetBalance::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(availableBalance, "availableBalance");
        serializer(lockedAmount, "lockedAmount");
    }

    void GetBlockHashes::Request::serialize(cryptonote::ISerializer &serializer)
    {
        bool r = serializer(firstBlockIndex, "firstBlockIndex");
        r &= serializer(blockCount, "blockCount");

        if (!r)
        {
            throw RequestSerializationError();
        }
    }

    void GetBlockHashes::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(blockHashes, "blockHashes");
    }

    void TransactionHashesInBlockRpcInfo::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(blockHash, "blockHash");
        serializer(transactionHashes, "transactionHashes");
    }

    void GetTransactionHashes::Request::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(addresses, "addresses");

        if (serializer(blockHash, "blockHash") == serializer(firstBlockIndex, "firstBlockIndex"))
        {
            throw RequestSerializationError();
        }

        if (!serializer(blockCount, "blockCount"))
        {
            throw RequestSerializationError();
        }

        serializer(paymentId, "paymentId");
    }

    void GetTransactionHashes::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(items, "items");
    }

    void TransferRpcInfo::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(type, "type");
        serializer(address, "address");
        serializer(amount, "amount");
    }

    void TransactionRpcInfo::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(state, "state");
        serializer(transactionHash, "transactionHash");
        serializer(blockIndex, "blockIndex");
        serializer(timestamp, "timestamp");
        serializer(isBase, "isBase");
        serializer(unlockTime, "unlockTime");
        serializer(amount, "amount");
        serializer(fee, "fee");
        serializer(transfers, "transfers");
        serializer(extra, "extra");
        serializer(paymentId, "paymentId");
    }

    void GetTransaction::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(transactionHash, "transactionHash"))
        {
            throw RequestSerializationError();
        }
    }

    void GetTransaction::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(transaction, "transaction");
    }

    void TransactionsInBlockRpcInfo::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(blockHash, "blockHash");
        serializer(transactions, "transactions");
    }

    void GetTransactions::Request::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(addresses, "addresses");

        if (serializer(blockHash, "blockHash") == serializer(firstBlockIndex, "firstBlockIndex"))
        {
            throw RequestSerializationError();
        }

        if (!serializer(blockCount, "blockCount"))
        {
            throw RequestSerializationError();
        }

        serializer(paymentId, "paymentId");
    }

    void GetTransactions::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(items, "items");
    }

    void GetUnconfirmedTransactionHashes::Request::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(addresses, "addresses");
    }

    void GetUnconfirmedTransactionHashes::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(transactionHashes, "transactionHashes");
    }

    void WalletRpcOrder::serialize(cryptonote::ISerializer &serializer)
    {
        bool r = serializer(address, "address");
        r &= serializer(amount, "amount");

        if (!r)
        {
            throw RequestSerializationError();
        }
    }

    void SendTransaction::Request::serialize(cryptonote::ISerializer &serializer, const WalletService &service)
    {
        serializer(sourceAddresses, "addresses");

        if (!serializer(transfers, "transfers"))
        {
            throw RequestSerializationError();
        }

        serializer(changeAddress, "changeAddress");

        if (!serializer(fee, "fee"))
        {
            throw RequestSerializationError();
        }

        if (!serializer(anonymity, "anonymity"))
        {
            anonymity = service.getDefaultMixin();
        }

        bool hasExtra = serializer(extra, "extra");
        bool hasPaymentId = serializer(paymentId, "paymentId");

        if (hasExtra && hasPaymentId)
        {
            throw RequestSerializationError();
        }

        serializer(unlockTime, "unlockTime");
    }

    void SendTransaction::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(transactionHash, "transactionHash");
    }

    void CreateDelayedTransaction::Request::serialize(cryptonote::ISerializer &serializer, const WalletService &service)
    {
        serializer(addresses, "addresses");

        if (!serializer(transfers, "transfers"))
        {
            throw RequestSerializationError();
        }

        serializer(changeAddress, "changeAddress");

        if (!serializer(fee, "fee"))
        {
            throw RequestSerializationError();
        }

        if (!serializer(anonymity, "anonymity"))
        {
            anonymity = service.getDefaultMixin();
        }

        bool hasExtra = serializer(extra, "extra");
        bool hasPaymentId = serializer(paymentId, "paymentId");

        if (hasExtra && hasPaymentId)
        {
            throw RequestSerializationError();
        }

        serializer(unlockTime, "unlockTime");
    }

    void CreateDelayedTransaction::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(transactionHash, "transactionHash");
    }

    void GetDelayedTransactionHashes::Request::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void GetDelayedTransactionHashes::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(transactionHashes, "transactionHashes");
    }

    void DeleteDelayedTransaction::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(transactionHash, "transactionHash"))
        {
            throw RequestSerializationError();
        }
    }

    void DeleteDelayedTransaction::Response::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void SendDelayedTransaction::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(transactionHash, "transactionHash"))
        {
            throw RequestSerializationError();
        }
    }

    void SendDelayedTransaction::Response::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void SendFusionTransaction::Request::serialize(cryptonote::ISerializer &serializer, const WalletService &service)
    {
        if (!serializer(threshold, "threshold"))
        {
            throw RequestSerializationError();
        }

        if (!serializer(anonymity, "anonymity"))
        {
            anonymity = service.getDefaultMixin();
        }

        serializer(addresses, "addresses");
        serializer(destinationAddress, "destinationAddress");
    }

    void SendFusionTransaction::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(transactionHash, "transactionHash");
    }

    void EstimateFusion::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(threshold, "threshold"))
        {
            throw RequestSerializationError();
        }

        serializer(addresses, "addresses");
    }

    void EstimateFusion::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(fusionReadyCount, "fusionReadyCount");
        serializer(totalOutputCount, "totalOutputCount");
    }

    void CreateIntegratedAddress::Request::serialize(cryptonote::ISerializer &serializer)
    {
        if (!serializer(address, "address"))
        {
            throw RequestSerializationError();
        }

        if (!serializer(paymentId, "paymentId"))
        {
            throw RequestSerializationError();
        }
    }

    void CreateIntegratedAddress::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(integratedAddress, "integratedAddress");
    }

    void NodeFeeInfo::Request::serialize(cryptonote::ISerializer &serializer)
    {
    }

    void NodeFeeInfo::Response::serialize(cryptonote::ISerializer &serializer)
    {
        serializer(address, "address");
        serializer(amount, "amount");
    }

}
