// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#pragma once

#include "cryptonote_basic.h"
#include "crypto/chacha8.h"
#include "serialization/iserializer.h"
#include "crypto/crypto.h"

namespace crypto
{

    bool serialize(PublicKey &pubKey, common::StringView name, cryptonote::ISerializer &serializer);
    bool serialize(SecretKey &secKey, common::StringView name, cryptonote::ISerializer &serializer);
    bool serialize(Hash &h, common::StringView name, cryptonote::ISerializer &serializer);
    bool serialize(chacha8_iv &chacha, common::StringView name, cryptonote::ISerializer &serializer);
    bool serialize(KeyImage &keyImage, common::StringView name, cryptonote::ISerializer &serializer);
    bool serialize(Signature &sig, common::StringView name, cryptonote::ISerializer &serializer);
    bool serialize(EllipticCurveScalar &ecScalar, common::StringView name, cryptonote::ISerializer &serializer);
    bool serialize(EllipticCurvePoint &ecPoint, common::StringView name, cryptonote::ISerializer &serializer);

}

namespace cryptonote
{

    struct AccountKeys;
    struct TransactionExtraMergeMiningTag;

    enum class SerializationTag : uint8_t
    {
        Base = 0xff,
        Key = 0x2,
        Transaction = 0xcc,
        Block = 0xbb
    };

    void serialize(TransactionPrefix &txP, ISerializer &serializer);
    void serialize(Transaction &tx, ISerializer &serializer);
    void serialize(BaseTransaction &tx, ISerializer &serializer);
    void serialize(TransactionInput &in, ISerializer &serializer);
    void serialize(TransactionOutput &in, ISerializer &serializer);

    void serialize(BaseInput &gen, ISerializer &serializer);
    void serialize(KeyInput &key, ISerializer &serializer);

    void serialize(TransactionOutput &output, ISerializer &serializer);
    void serialize(TransactionOutputTarget &output, ISerializer &serializer);
    void serialize(KeyOutput &key, ISerializer &serializer);

    void serialize(BlockHeader &header, ISerializer &serializer);
    void serialize(BlockTemplate &block, ISerializer &serializer);
    void serialize(ParentBlockSerializer &pbs, ISerializer &serializer);
    void serialize(TransactionExtraMergeMiningTag &tag, ISerializer &serializer);

    void serialize(AccountPublicAddress &address, ISerializer &serializer);
    void serialize(AccountKeys &keys, ISerializer &s);

    void serialize(KeyPair &keyPair, ISerializer &serializer);
    void serialize(RawBlock &rawBlock, ISerializer &serializer);

}
