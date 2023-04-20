// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include <cryptonote_core/cryptonote_serialization.h>
#include <cryptonote_core/cryptonote_tools.h>
#include <cryptonote_core/database_cache_data.h>
#include <serialization/serialization_overloads.h>

namespace cryptonote
{

    void ExtendedTransactionInfo::serialize(cryptonote::ISerializer &s)
    {
        s(static_cast<CachedTransactionInfo &>(*this), "cached_transaction");
        s(amountToKeyIndexes, "key_indexes");
    }

    void KeyOutputInfo::serialize(ISerializer &s)
    {
        s(publicKey, "public_key");
        s(transactionHash, "transaction_hash");
        s(unlockTime, "unlock_time");
        s(outputIndex, "output_index");
    }

}
