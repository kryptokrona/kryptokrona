// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2018, The TurtleCoin Developers
// Copyright (c) 2018, The Karai Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "cryptonote_protocol/cryptonote_protocol_definitions.h"
#include "cryptonote_core/cryptonote_basic.h"
#include "cryptonote_core/difficulty.h"
#include "crypto/hash.h"

#include "blockchain_explorer_data.h"

#include "serialization/serialization_overloads.h"
#include "serialization/blockchain_explorer_data_serialization.h"
#include <cryptonote_core/icore_definitions.h>

#include <wallet_types.h>

namespace cryptonote
{
    //-----------------------------------------------
    #define CORE_RPC_STATUS_OK "OK"
    #define CORE_RPC_STATUS_BUSY "BUSY"

    struct EMPTY_STRUCT {
      void serialize(ISerializer &s) {}
    };

    struct STATUS_STRUCT {
      std::string status;

      void serialize(ISerializer &s) {
        KV_MEMBER(status)
      }
    };

    struct COMMAND_RPC_GET_HEIGHT {
      typedef EMPTY_STRUCT request;

      struct response {
        uint64_t height;
        uint32_t network_height;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(height)
          KV_MEMBER(network_height)
          KV_MEMBER(status)
        }
      };
    };

    struct COMMAND_RPC_GET_BLOCKS_FAST {

      struct request {
        std::vector<Crypto::Hash> block_ids; //*first 10 blocks id goes sequential, next goes in pow(2,n) offset, like 2, 4, 8, 16, 32, 64 and so on, and the last one is always genesis block */

        void serialize(ISerializer &s) {
          KV_MEMBER(block_ids);
        }
      };

      struct response {
        std::vector<RawBlock> blocks;
        uint64_t start_height;
        uint64_t current_height;
        std::string status;
      };
    };
    //-----------------------------------------------
    struct COMMAND_RPC_GET_TRANSACTIONS {
      struct request {
        std::vector<std::string> txs_hashes;

        void serialize(ISerializer &s) {
          KV_MEMBER(txs_hashes)
        }
      };

      struct response {
        std::vector<std::string> txs_as_hex; //transactions blobs as hex
        std::vector<std::string> missed_tx;  //not found transactions
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(txs_as_hex)
          KV_MEMBER(missed_tx)
          KV_MEMBER(status)
        }
      };
    };
    //-----------------------------------------------
    struct COMMAND_RPC_GET_POOL_CHANGES {
      struct request {
        Crypto::Hash tailBlockId;
        std::vector<Crypto::Hash> knownTxsIds;

        void serialize(ISerializer &s) {
          KV_MEMBER(tailBlockId)
          KV_MEMBER(knownTxsIds);
        }
      };

      struct response {
        bool isTailBlockActual;
        std::vector<BinaryArray> addedTxs;          // Added transactions blobs
        std::vector<Crypto::Hash> deletedTxsIds; // IDs of not found transactions
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(isTailBlockActual)
          KV_MEMBER(addedTxs)
          KV_MEMBER(deletedTxsIds);
          KV_MEMBER(status)
        }
      };
    };

    struct COMMAND_RPC_GET_POOL_CHANGES_LITE {
      struct request {
        Crypto::Hash tailBlockId;
        std::vector<Crypto::Hash> knownTxsIds;

        void serialize(ISerializer &s) {
          KV_MEMBER(tailBlockId)
          KV_MEMBER(knownTxsIds);
        }
      };

      struct response {
        bool isTailBlockActual;
        std::vector<TransactionPrefixInfo> addedTxs;          // Added transactions blobs
        std::vector<Crypto::Hash> deletedTxsIds; // IDs of not found transactions
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(isTailBlockActual)
          KV_MEMBER(addedTxs)
          KV_MEMBER(deletedTxsIds);
          KV_MEMBER(status)
        }
      };
    };

    //-----------------------------------------------
    struct COMMAND_RPC_GET_TX_GLOBAL_OUTPUTS_INDEXES {

      struct request {
        Crypto::Hash txid;

        void serialize(ISerializer &s) {
          KV_MEMBER(txid)
        }
      };

      struct response {
        std::vector<uint64_t> o_indexes;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(o_indexes)
          KV_MEMBER(status)
        }
      };
    };

    struct COMMAND_RPC_GET_GLOBAL_INDEXES_FOR_RANGE
    {
        struct request
        {
            uint64_t startHeight;
            uint64_t endHeight;

            void serialize(ISerializer &s)
            {
                KV_MEMBER(startHeight);
                KV_MEMBER(endHeight);
            }
        };

        struct response
        {
            std::unordered_map<Crypto::Hash, std::vector<uint64_t>> indexes;

            std::string status;

            void serialize(ISerializer &s)
            {
                KV_MEMBER(indexes)
                KV_MEMBER(status)
            }
        };
    };


    //-----------------------------------------------

    #pragma pack(push, 1)
    struct OutputEntry {
      uint32_t global_amount_index;
      Crypto::PublicKey out_key;

      void serialize(ISerializer &s) {
        KV_MEMBER(global_amount_index)
        KV_MEMBER(out_key);
      }
    };
    #pragma pack(pop)

    struct RandomOuts {
      uint64_t amount;
      std::vector<OutputEntry> outs;

      void serialize(ISerializer &s) {
        KV_MEMBER(amount)
        KV_MEMBER(outs);
      }
    };

    inline void to_json(nlohmann::json &j, const RandomOuts &r)
    {
        j = {
            {"amount", r.amount},
            {"outs", r.outs}
        };
    }

    inline void from_json(const nlohmann::json &j, RandomOuts &r)
    {
        r.amount = j.at("amount").get<uint64_t>();
        r.outs = j.at("outs").get<std::vector<OutputEntry>>();
    }

    inline void to_json(nlohmann::json &j, const OutputEntry &o)
    {
        j = {
            {"global_amount_index", o.global_amount_index},
            {"out_key", o.out_key}
        };
    }

    inline void from_json(const nlohmann::json &j, OutputEntry &o)
    {
        o.global_amount_index = j.at("global_amount_index").get<uint32_t>();
        o.out_key = j.at("out_key").get<Crypto::PublicKey>();
    }

    struct COMMAND_RPC_GET_RANDOM_OUTPUTS_FOR_AMOUNTS
    {
        struct request {
          std::vector<uint64_t> amounts;
          uint16_t outs_count;

          void serialize(ISerializer &s) {
            KV_MEMBER(amounts)
            KV_MEMBER(outs_count)
          }
        };

        struct response {
          std::vector<RandomOuts> outs;
          std::string status;

          void serialize(ISerializer &s) {
            KV_MEMBER(outs);
            KV_MEMBER(status)
          }
        };
    };

    //-----------------------------------------------
    struct COMMAND_RPC_SEND_RAW_TX {
      struct request {
        std::string tx_as_hex;

        request() {}
        explicit request(const Transaction &);

        void serialize(ISerializer &s) {
          KV_MEMBER(tx_as_hex)
        }
      };

      struct response {
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
        }
      };
    };
    //-----------------------------------------------
    struct COMMAND_RPC_START_MINING {
      struct request {
        std::string miner_address;
        uint64_t threads_count;

        void serialize(ISerializer &s) {
          KV_MEMBER(miner_address)
          KV_MEMBER(threads_count)
        }
      };

      struct response {
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
        }
      };
    };
    //-----------------------------------------------
    struct COMMAND_RPC_GET_INFO {
      typedef EMPTY_STRUCT request;

      struct response {
        std::string status;
        uint64_t height;
        uint64_t difficulty;
        uint64_t tx_count;
        uint64_t tx_pool_size;
        uint64_t alt_blocks_count;
        uint64_t outgoing_connections_count;
        uint64_t incoming_connections_count;
        uint64_t white_peerlist_size;
        uint64_t grey_peerlist_size;
        uint32_t last_known_block_index;
        uint32_t network_height;
        std::vector<uint64_t> upgrade_heights;
        uint64_t supported_height;
        uint32_t hashrate;
        uint8_t major_version;
        uint8_t minor_version;
        std::string version;
        uint64_t start_time;
        bool synced;
        bool testnet;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(height)
          KV_MEMBER(difficulty)
          KV_MEMBER(tx_count)
          KV_MEMBER(tx_pool_size)
          KV_MEMBER(alt_blocks_count)
          KV_MEMBER(outgoing_connections_count)
          KV_MEMBER(incoming_connections_count)
          KV_MEMBER(white_peerlist_size)
          KV_MEMBER(grey_peerlist_size)
          KV_MEMBER(last_known_block_index)
          KV_MEMBER(network_height)
          KV_MEMBER(upgrade_heights)
          KV_MEMBER(supported_height)
          KV_MEMBER(hashrate)
          KV_MEMBER(major_version)
          KV_MEMBER(minor_version)
          KV_MEMBER(start_time)
          KV_MEMBER(synced)
          KV_MEMBER(testnet)
          KV_MEMBER(version)
        }
      };
    };

    //-----------------------------------------------
    struct COMMAND_RPC_STOP_MINING {
      typedef EMPTY_STRUCT request;
      typedef STATUS_STRUCT response;
    };

    //-----------------------------------------------
    struct COMMAND_RPC_STOP_DAEMON {
      typedef EMPTY_STRUCT request;
      typedef STATUS_STRUCT response;
    };

    //
    struct COMMAND_RPC_GETBLOCKCOUNT {
      typedef std::vector<std::string> request;

      struct response {
        uint64_t count;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(count)
          KV_MEMBER(status)
        }
      };
    };

    struct COMMAND_RPC_GETBLOCKHASH {
      typedef std::vector<uint64_t> request;
      typedef std::string response;
    };

    struct COMMAND_RPC_GETBLOCKTEMPLATE {
      struct request {
        uint64_t reserve_size; //max 255 bytes
        std::string wallet_address;

        void serialize(ISerializer &s) {
          KV_MEMBER(reserve_size)
          KV_MEMBER(wallet_address)
        }
      };

      struct response {
        uint64_t difficulty;
        uint32_t height;
        uint64_t reserved_offset;
        std::string blocktemplate_blob;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(difficulty)
          KV_MEMBER(height)
          KV_MEMBER(reserved_offset)
          KV_MEMBER(blocktemplate_blob)
          KV_MEMBER(status)
        }
      };
    };

    struct COMMAND_RPC_GET_CURRENCY_ID {
      typedef EMPTY_STRUCT request;

      struct response {
        std::string currency_id_blob;

        void serialize(ISerializer &s) {
          KV_MEMBER(currency_id_blob)
        }
      };
    };

    struct COMMAND_RPC_SUBMITBLOCK {
      typedef std::vector<std::string> request;
      typedef STATUS_STRUCT response;
    };

    struct block_header_response {
      uint8_t major_version;
      uint8_t minor_version;
      uint64_t timestamp;
      std::string prev_hash;
      uint32_t nonce;
      bool orphan_status;
      uint32_t height;
      uint32_t depth;
      std::string hash;
      uint64_t difficulty;
      uint64_t reward;
      uint32_t num_txes;
      uint64_t block_size;

      void serialize(ISerializer &s) {
        KV_MEMBER(major_version)
        KV_MEMBER(minor_version)
        KV_MEMBER(timestamp)
        KV_MEMBER(prev_hash)
        KV_MEMBER(nonce)
        KV_MEMBER(orphan_status)
        KV_MEMBER(height)
        KV_MEMBER(depth)
        KV_MEMBER(hash)
        KV_MEMBER(difficulty)
        KV_MEMBER(reward)
        KV_MEMBER(num_txes)
        KV_MEMBER(block_size)
      }
    };

    struct BLOCK_HEADER_RESPONSE {
      std::string status;
      block_header_response block_header;

      void serialize(ISerializer &s) {
        KV_MEMBER(block_header)
        KV_MEMBER(status)
      }
    };


    struct COMMAND_RPC_GET_BLOCK_HEADERS_RANGE
    {
        struct request
        {
            uint64_t start_height;
            uint64_t end_height;

            void serialize(ISerializer &s) {
                KV_MEMBER(start_height)
                KV_MEMBER(end_height)
            }
            /*BEGIN_KV_SERIALIZE_MAP()
            KV_SERIALIZE(start_height)
            KV_SERIALIZE(end_height)
            END_KV_SERIALIZE_MAP()*/
        };

        struct response
        {
            std::string status;
            std::vector<block_header_response> headers;
            bool untrusted;

            void serialize(ISerializer &s) {
                KV_MEMBER(status)
                KV_MEMBER(headers)
                KV_MEMBER(untrusted)
            }
            /*BEGIN_KV_SERIALIZE_MAP()
            KV_SERIALIZE(status)
            KV_SERIALIZE(headers)
            KV_SERIALIZE(untrusted)
            END_KV_SERIALIZE_MAP()*/
        };
    };

    struct f_transaction_short_response {
      std::string hash;
      uint64_t fee;
      uint64_t amount_out;
      uint64_t size;

      void serialize(ISerializer &s) {
        KV_MEMBER(hash)
        KV_MEMBER(fee)
        KV_MEMBER(amount_out)
        KV_MEMBER(size)
      }
    };

    struct f_transaction_details_response {
      std::string hash;
      uint64_t size;
      std::string paymentId;
      uint64_t mixin;
      uint64_t fee;
      uint64_t amount_out;

      void serialize(ISerializer &s) {
        KV_MEMBER(hash)
        KV_MEMBER(size)
        KV_MEMBER(paymentId)
        KV_MEMBER(mixin)
        KV_MEMBER(fee)
        KV_MEMBER(amount_out)
      }
    };

    struct f_block_short_response {
      uint64_t difficulty;
      uint64_t timestamp;
      uint32_t height;
      std::string hash;
      uint64_t tx_count;
      uint64_t cumul_size;

      void serialize(ISerializer &s) {
        KV_MEMBER(difficulty)
        KV_MEMBER(timestamp)
        KV_MEMBER(height)
        KV_MEMBER(hash)
        KV_MEMBER(cumul_size)
        KV_MEMBER(tx_count)
      }
    };

    struct f_block_details_response {
      uint8_t major_version;
      uint8_t minor_version;
      uint64_t timestamp;
      std::string prev_hash;
      uint32_t nonce;
      bool orphan_status;
      uint32_t height;
      uint64_t depth;
      std::string hash;
      uint64_t difficulty;
      uint64_t reward;
      uint64_t blockSize;
      uint64_t sizeMedian;
      uint64_t effectiveSizeMedian;
      uint64_t transactionsCumulativeSize;
      std::string alreadyGeneratedCoins;
      uint64_t alreadyGeneratedTransactions;
      uint64_t baseReward;
      double penalty;
      uint64_t totalFeeAmount;
      std::vector<f_transaction_short_response> transactions;

      void serialize(ISerializer &s) {
        KV_MEMBER(major_version)
        KV_MEMBER(minor_version)
        KV_MEMBER(timestamp)
        KV_MEMBER(prev_hash)
        KV_MEMBER(nonce)
        KV_MEMBER(orphan_status)
        KV_MEMBER(height)
        KV_MEMBER(depth)
        KV_MEMBER(hash)
        KV_MEMBER(difficulty)
        KV_MEMBER(reward)
        KV_MEMBER(blockSize)
        KV_MEMBER(sizeMedian)
        KV_MEMBER(effectiveSizeMedian)
        KV_MEMBER(transactionsCumulativeSize)
        KV_MEMBER(alreadyGeneratedCoins)
        KV_MEMBER(alreadyGeneratedTransactions)
        KV_MEMBER(baseReward)
        KV_MEMBER(penalty)
        KV_MEMBER(transactions)
        KV_MEMBER(totalFeeAmount)
      }
    };
    struct COMMAND_RPC_GET_LAST_BLOCK_HEADER {
      typedef EMPTY_STRUCT request;
      typedef BLOCK_HEADER_RESPONSE response;
    };

    struct COMMAND_RPC_GET_BLOCK_HEADER_BY_HASH {
      struct request {
        std::string hash;

        void serialize(ISerializer &s) {
          KV_MEMBER(hash)
        }
      };

      typedef BLOCK_HEADER_RESPONSE response;
    };

    struct COMMAND_RPC_GET_BLOCK_HEADER_BY_HEIGHT {
      struct request {
        uint64_t height;

        void serialize(ISerializer &s) {
          KV_MEMBER(height)
        }
      };

      typedef BLOCK_HEADER_RESPONSE response;
    };

    struct F_COMMAND_RPC_GET_BLOCKS_LIST {
      struct request {
        uint64_t height;

        void serialize(ISerializer &s) {
          KV_MEMBER(height)
        }
      };

      struct response {
        std::vector<f_block_short_response> blocks; //transactions blobs as hex
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(blocks)
          KV_MEMBER(status)
        }
      };
    };

    struct F_COMMAND_RPC_GET_BLOCK_DETAILS {
      struct request {
        std::string hash;

        void serialize(ISerializer &s) {
          KV_MEMBER(hash)
        }
      };

      struct response {
        f_block_details_response block;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(block)
          KV_MEMBER(status)
        }
      };
    };

    struct F_COMMAND_RPC_GET_TRANSACTION_DETAILS {
      struct request {
        std::string hash;

        void serialize(ISerializer &s) {
          KV_MEMBER(hash)
        }
      };

      struct response {
        Transaction tx;
        f_transaction_details_response txDetails;
        f_block_short_response block;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(tx)
          KV_MEMBER(txDetails)
          KV_MEMBER(block)
          KV_MEMBER(status)
        }
      };
    };

    struct F_COMMAND_RPC_GET_POOL {
      typedef EMPTY_STRUCT request;

      struct response {
        std::vector<f_transaction_short_response> transactions; //transactions blobs as hex
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(transactions)
          KV_MEMBER(status)
        }
      };
    };
    struct COMMAND_RPC_QUERY_BLOCKS {
      struct request {
        std::vector<Crypto::Hash> block_ids; //*first 10 blocks id goes sequential, next goes in pow(2,n) offset, like 2, 4, 8, 16, 32, 64 and so on, and the last one is always genesis block */
        uint64_t timestamp;

        void serialize(ISerializer &s) {
          KV_MEMBER(block_ids);
          KV_MEMBER(timestamp)
        }
      };

      struct response {
        std::string status;
        uint64_t start_height;
        uint64_t current_height;
        uint64_t full_offset;
        std::vector<BlockFullInfo> items;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(start_height)
          KV_MEMBER(current_height)
          KV_MEMBER(full_offset)
          KV_MEMBER(items)
        }
      };
    };

    struct COMMAND_RPC_QUERY_BLOCKS_LITE {
      struct request {
        std::vector<Crypto::Hash> blockIds;
        uint64_t timestamp;

        void serialize(ISerializer &s) {
          KV_MEMBER(blockIds);
          KV_MEMBER(timestamp)
        }
      };

      struct response {
        std::string status;
        uint64_t startHeight;
        uint64_t currentHeight;
        uint64_t fullOffset;
        std::vector<BlockShortInfo> items;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(startHeight)
          KV_MEMBER(currentHeight)
          KV_MEMBER(fullOffset)
          KV_MEMBER(items)
        }
      };
    };

    struct COMMAND_RPC_QUERY_BLOCKS_DETAILED {
      struct request {
        std::vector<Crypto::Hash> blockIds;
        uint64_t timestamp;
        uint32_t blockCount;

        void serialize(ISerializer &s) {
          KV_MEMBER(blockIds);
          KV_MEMBER(timestamp)
          KV_MEMBER(blockCount)
        }
      };

      struct response {
        std::string status;
        uint64_t startHeight;
        uint64_t currentHeight;
        uint64_t fullOffset;
        std::vector<BlockDetails> blocks;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(startHeight)
          KV_MEMBER(currentHeight)
          KV_MEMBER(fullOffset)
          KV_MEMBER(blocks)
        }
      };
    };

    struct COMMAND_RPC_GET_WALLET_SYNC_DATA {
      struct request {
        std::vector<Crypto::Hash> blockIds;

        uint64_t startHeight;
        uint64_t startTimestamp;
        uint64_t blockCount;

        void serialize(ISerializer &s) {
          s(blockIds, "blockHashCheckpoints");
          KV_MEMBER(startHeight);
          KV_MEMBER(startTimestamp);
          KV_MEMBER(blockCount);
        }
      };

      struct response {
        std::string status;
        std::vector<WalletTypes::WalletBlockInfo> items;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(items);
        }
      };
    };

    struct COMMAND_RPC_GET_TRANSACTIONS_STATUS
    {
        struct request
        {
            std::unordered_set<Crypto::Hash> transactionHashes;

            void serialize(ISerializer &s)
            {
                KV_MEMBER(transactionHashes);
            }
        };

        struct response
        {
            std::string status;

            /* These transactions are in the transaction pool */
            std::unordered_set<Crypto::Hash> transactionsInPool;

            /* These transactions are in a block */
            std::unordered_set<Crypto::Hash> transactionsInBlock;

            /* We don't know anything about these hashes */
            std::unordered_set<Crypto::Hash> transactionsUnknown;

            void serialize(ISerializer &s)
            {
                KV_MEMBER(status);
                KV_MEMBER(transactionsInPool);
                KV_MEMBER(transactionsInBlock);
                KV_MEMBER(transactionsUnknown);
            }
        };
    };

    struct COMMAND_RPC_GET_BLOCKS_DETAILS_BY_HEIGHTS {
      struct request {
        std::vector<uint32_t> blockHeights;

        void serialize(ISerializer& s) {
          KV_MEMBER(blockHeights);
        }
      };

      struct response {
        std::vector<BlockDetails> blocks;
        std::string status;

        void serialize(ISerializer& s) {
          KV_MEMBER(status)
          KV_MEMBER(blocks)
        }
      };
    };

    struct COMMAND_RPC_GET_BLOCKS_DETAILS_BY_HASHES {
      struct request {
        std::vector<Crypto::Hash> blockHashes;

        void serialize(ISerializer& s) {
          KV_MEMBER(blockHashes);
        }
      };

      struct response {
        std::vector<BlockDetails> blocks;
        std::string status;

        void serialize(ISerializer& s) {
          KV_MEMBER(status)
          KV_MEMBER(blocks)
        }
      };
    };

    struct COMMAND_RPC_GET_BLOCK_DETAILS_BY_HEIGHT {
      struct request {
        uint32_t blockHeight;

        void serialize(ISerializer& s) {
          KV_MEMBER(blockHeight)
        }
      };

      struct response {
        BlockDetails block;
        std::string status;

        void serialize(ISerializer& s) {
          KV_MEMBER(status)
          KV_MEMBER(block)
        }
      };
    };

    struct COMMAND_RPC_GET_BLOCKS_HASHES_BY_TIMESTAMPS {
      struct request {
        uint64_t timestampBegin;
        uint64_t secondsCount;

        void serialize(ISerializer &s) {
          KV_MEMBER(timestampBegin)
          KV_MEMBER(secondsCount)
        }
      };

      struct response {
        std::vector<Crypto::Hash> blockHashes;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(blockHashes)
        }
      };
    };

    struct COMMAND_RPC_GET_TRANSACTION_HASHES_BY_PAYMENT_ID {
      struct request {
        Crypto::Hash paymentId;

        void serialize(ISerializer &s) {
          KV_MEMBER(paymentId)
        }
      };

      struct response {
        std::vector<Crypto::Hash> transactionHashes;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(transactionHashes);
        }
      };
    };

    struct COMMAND_RPC_GET_TRANSACTION_DETAILS_BY_HASHES {
      struct request {
        std::vector<Crypto::Hash> transactionHashes;

        void serialize(ISerializer &s) {
          KV_MEMBER(transactionHashes);
        }
      };

      struct response {
        std::vector<TransactionDetails> transactions;
        std::string status;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(transactions)
        }
      };
    };

    struct COMMAND_RPC_GET_PEERS {
      // TODO: rename peers to white_peers - do at v1
      typedef EMPTY_STRUCT request;

      struct response {
        std::string status;
        std::vector<std::string> peers;
        std::vector<std::string> gray_peers;

        void serialize(ISerializer &s) {
          KV_MEMBER(status)
          KV_MEMBER(peers)
          KV_MEMBER(gray_peers)
        }
      };
    };

    struct COMMAND_RPC_GET_FEE_ADDRESS {
      typedef EMPTY_STRUCT request;

      struct response {
        std::string address;
        uint32_t amount;
        std::string status;

        void serialize(ISerializer & s) {
          KV_MEMBER(address)
          KV_MEMBER(amount)
          KV_MEMBER(status)
        }
      };
    };
}
