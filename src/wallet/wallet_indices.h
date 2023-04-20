// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include <map>
#include <unordered_map>

#include "itransfers_container.h"
#include "iwallet.h"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/multi_index/member.hpp>

#include "common/file_mapped_vector.h"
#include "crypto/chacha8.h"

namespace cryptonote
{

    const uint64_t ACCOUNT_CREATE_TIME_ACCURACY = 60 * 60 * 24;

    struct WalletRecord
    {
        crypto::PublicKey spendPublicKey;
        crypto::SecretKey spendSecretKey;
        cryptonote::ITransfersContainer *container = nullptr;
        uint64_t pendingBalance = 0;
        uint64_t actualBalance = 0;
        time_t creationTimestamp;
    };

#pragma pack(push, 1)
    struct EncryptedWalletRecord
    {
        crypto::chacha8_iv iv;
        // Secret key, public key and creation timestamp
        uint8_t data[sizeof(crypto::PublicKey) + sizeof(crypto::SecretKey) + sizeof(uint64_t)];
    };
#pragma pack(pop)

    struct RandomAccessIndex
    {
    };
    struct KeysIndex
    {
    };
    struct TransfersContainerIndex
    {
    };

    struct WalletIndex
    {
    };
    struct TransactionOutputIndex
    {
    };
    struct BlockHeightIndex
    {
    };

    struct TransactionHashIndex
    {
    };
    struct TransactionIndex
    {
    };
    struct BlockHashIndex
    {
    };

    typedef boost::multi_index_container<
        WalletRecord,
        boost::multi_index::indexed_by<
            boost::multi_index::random_access<boost::multi_index::tag<RandomAccessIndex>>,
            boost::multi_index::hashed_unique<boost::multi_index::tag<KeysIndex>,
                                              BOOST_MULTI_INDEX_MEMBER(WalletRecord, crypto::PublicKey, spendPublicKey)>,
            boost::multi_index::hashed_unique<boost::multi_index::tag<TransfersContainerIndex>,
                                              BOOST_MULTI_INDEX_MEMBER(WalletRecord, cryptonote::ITransfersContainer *, container)>>>
        WalletsContainer;

    struct UnlockTransactionJob
    {
        uint32_t blockHeight;
        cryptonote::ITransfersContainer *container;
        crypto::Hash transactionHash;
    };

    typedef boost::multi_index_container<
        UnlockTransactionJob,
        boost::multi_index::indexed_by<
            boost::multi_index::ordered_non_unique<boost::multi_index::tag<BlockHeightIndex>,
                                                   BOOST_MULTI_INDEX_MEMBER(UnlockTransactionJob, uint32_t, blockHeight)>,
            boost::multi_index::hashed_non_unique<boost::multi_index::tag<TransactionHashIndex>,
                                                  BOOST_MULTI_INDEX_MEMBER(UnlockTransactionJob, crypto::Hash, transactionHash)>>>
        UnlockTransactionJobs;

    typedef boost::multi_index_container<
        cryptonote::WalletTransaction,
        boost::multi_index::indexed_by<
            boost::multi_index::random_access<boost::multi_index::tag<RandomAccessIndex>>,
            boost::multi_index::hashed_unique<boost::multi_index::tag<TransactionIndex>,
                                              boost::multi_index::member<cryptonote::WalletTransaction, crypto::Hash, &cryptonote::WalletTransaction::hash>>,
            boost::multi_index::ordered_non_unique<boost::multi_index::tag<BlockHeightIndex>,
                                                   boost::multi_index::member<cryptonote::WalletTransaction, uint32_t, &cryptonote::WalletTransaction::blockHeight>>>>
        WalletTransactions;

    typedef common::FileMappedVector<EncryptedWalletRecord> ContainerStorage;
    typedef std::pair<uint64_t, cryptonote::WalletTransfer> TransactionTransferPair;
    typedef std::vector<TransactionTransferPair> WalletTransfers;
    typedef std::map<uint64_t, cryptonote::Transaction> UncommitedTransactions;

    typedef boost::multi_index_container<
        crypto::Hash,
        boost::multi_index::indexed_by<
            boost::multi_index::random_access<
                boost::multi_index::tag<BlockHeightIndex>>,
            boost::multi_index::hashed_unique<
                boost::multi_index::tag<BlockHashIndex>,
                boost::multi_index::identity<crypto::Hash>>>>
        BlockHashesContainer;

}
