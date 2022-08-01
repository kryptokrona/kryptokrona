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

#include "cryptonote_basic.h"
#include "crypto/chacha8.h"
#include "serialization/iserializer.h"
#include "crypto/crypto.h"

namespace crypto
{
    bool serialize(PublicKey& pubKey, common::StringView name, cryptonote::ISerializer& serializer);
    bool serialize(SecretKey& secKey, common::StringView name, cryptonote::ISerializer& serializer);
    bool serialize(Hash& h, common::StringView name, cryptonote::ISerializer& serializer);
    bool serialize(chacha8_iv& chacha, common::StringView name, cryptonote::ISerializer& serializer);
    bool serialize(KeyImage& keyImage, common::StringView name, cryptonote::ISerializer& serializer);
    bool serialize(Signature& sig, common::StringView name, cryptonote::ISerializer& serializer);
    bool serialize(EllipticCurveScalar& ecScalar, common::StringView name, cryptonote::ISerializer& serializer);
    bool serialize(EllipticCurvePoint& ecPoint, common::StringView name, cryptonote::ISerializer& serializer);
}

namespace cryptonote
{
    struct AccountKeys;
    struct TransactionExtraMergeMiningTag;

    enum class SerializationTag : uint8_t { Base = 0xff, Key = 0x2, Transaction = 0xcc, Block = 0xbb };

    void serialize(TransactionPrefix& txP, ISerializer& serializer);
    void serialize(Transaction& tx, ISerializer& serializer);
    void serialize(BaseTransaction& tx, ISerializer& serializer);
    void serialize(TransactionInput& in, ISerializer& serializer);
    void serialize(TransactionOutput& in, ISerializer& serializer);

    void serialize(BaseInput& gen, ISerializer& serializer);
    void serialize(KeyInput& key, ISerializer& serializer);

    void serialize(TransactionOutput& output, ISerializer& serializer);
    void serialize(TransactionOutputTarget& output, ISerializer& serializer);
    void serialize(KeyOutput& key, ISerializer& serializer);

    void serialize(BlockHeader& header, ISerializer& serializer);
    void serialize(BlockTemplate& block, ISerializer& serializer);
    void serialize(ParentBlockSerializer& pbs, ISerializer& serializer);
    void serialize(TransactionExtraMergeMiningTag& tag, ISerializer& serializer);

    void serialize(AccountPublicAddress& address, ISerializer& serializer);
    void serialize(AccountKeys& keys, ISerializer& s);

    void serialize(KeyPair& keyPair, ISerializer& serializer);
    void serialize(RawBlock& rawBlock, ISerializer& serializer);
}
