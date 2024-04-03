// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
//
// This file is part of Bytecoin.
//
// Bytecoin is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Bytecoin is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Bytecoin.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <vector>
#include <boost/variant.hpp>
#include "crypto_types.h"

namespace cryptonote {

struct BaseInput {
  uint32_t blockIndex;
};

struct KeyInput {
  uint64_t amount;
  std::vector<uint32_t> outputIndexes;
  crypto::KeyImage keyImage;
};

struct KeyOutput {
  crypto::PublicKey key;
};

typedef boost::variant<BaseInput, KeyInput> TransactionInput;

typedef boost::variant<KeyOutput> TransactionOutputTarget;

struct TransactionOutput {
  uint64_t amount;
  TransactionOutputTarget target;
};

struct TransactionPrefix {
  uint8_t version;
  uint64_t unlockTime;
  std::vector<TransactionInput> inputs;
  std::vector<TransactionOutput> outputs;
  std::vector<uint8_t> extra;
};

struct Transaction : public TransactionPrefix {
  std::vector<std::vector<crypto::Signature>> signatures;
};

struct BaseTransaction : public TransactionPrefix {
};

struct ParentBlock {
  uint8_t majorVersion;
  uint8_t minorVersion;
  crypto::Hash previousBlockHash;
  uint16_t transactionCount;
  std::vector<crypto::Hash> baseTransactionBranch;
  BaseTransaction baseTransaction;
  std::vector<crypto::Hash> blockchainBranch;
};

struct BlockHeader {
  uint8_t majorVersion;
  uint8_t minorVersion;
  uint32_t nonce;
  uint64_t timestamp;
  crypto::Hash previousBlockHash;
};

struct BlockTemplate : public BlockHeader {
  ParentBlock parentBlock;
  Transaction baseTransaction;
  std::vector<crypto::Hash> transactionHashes;
};

struct AccountPublicAddress {
  crypto::PublicKey spendPublicKey;
  crypto::PublicKey viewPublicKey;
};

struct AccountKeys {
  AccountPublicAddress address;
  crypto::SecretKey spendSecretKey;
  crypto::SecretKey viewSecretKey;
};

struct KeyPair {
  crypto::PublicKey publicKey;
  crypto::SecretKey secretKey;
};

using BinaryArray = std::vector<uint8_t>;

struct RawBlock {
  BinaryArray block; //BlockTemplate
  std::vector<BinaryArray> transactions;
};

inline void to_json(nlohmann::json &j, const cryptonote::KeyInput &k)
{
    j = {
        {"amount", k.amount},
        {"key_offsets", k.outputIndexes},
        {"k_image", k.keyImage}
    };
}

inline void from_json(const nlohmann::json &j, cryptonote::KeyInput &k)
{
    k.amount = j.at("amount").get<uint64_t>();
    k.outputIndexes = j.at("key_offsets").get<std::vector<uint32_t>>();
    k.keyImage = j.at("k_image").get<crypto::KeyImage>();
}

}
