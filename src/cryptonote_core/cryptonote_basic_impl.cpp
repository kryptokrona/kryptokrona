// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "cryptonote_basic_impl.h"
#include "cryptonote_format_utils.h"
#include "cryptonote_tools.h"
#include "cryptonote_serialization.h"

#include "common/base58.h"
#include "crypto/hash.h"
#include "common/int_util.h"

using namespace crypto;
using namespace common;

namespace cryptonote
{

    /************************************************************************/
    /* CryptoNote helper functions                                          */
    /************************************************************************/
    //-----------------------------------------------------------------------------------------------
    uint64_t getPenalizedAmount(uint64_t amount, size_t medianSize, size_t currentBlockSize)
    {
        static_assert(sizeof(size_t) >= sizeof(uint32_t), "size_t is too small");
        assert(currentBlockSize <= 2 * medianSize);
        assert(medianSize <= std::numeric_limits<uint32_t>::max());
        assert(currentBlockSize <= std::numeric_limits<uint32_t>::max());

        if (amount == 0)
        {
            return 0;
        }

        if (currentBlockSize <= medianSize)
        {
            return amount;
        }

        uint64_t productHi;
        uint64_t productLo = mul128(amount, currentBlockSize * (UINT64_C(2) * medianSize - currentBlockSize), &productHi);

        uint64_t penalizedAmountHi;
        uint64_t penalizedAmountLo;
        div128_32(productHi, productLo, static_cast<uint32_t>(medianSize), &penalizedAmountHi, &penalizedAmountLo);
        div128_32(penalizedAmountHi, penalizedAmountLo, static_cast<uint32_t>(medianSize), &penalizedAmountHi, &penalizedAmountLo);

        assert(0 == penalizedAmountHi);
        assert(penalizedAmountLo < amount);

        return penalizedAmountLo;
    }
    //-----------------------------------------------------------------------
    std::string getAccountAddressAsStr(uint64_t prefix, const AccountPublicAddress &adr)
    {
        BinaryArray ba;
        bool r = toBinaryArray(adr, ba);
        if (r)
        {
        }
        assert(r);
        return tools::base58::encode_addr(prefix, common::asString(ba));
    }
    //-----------------------------------------------------------------------
    bool parseAccountAddressString(uint64_t &prefix, AccountPublicAddress &adr, const std::string &str)
    {
        std::string data;

        return tools::base58::decode_addr(str, prefix, data) &&
               fromBinaryArray(adr, asBinaryArray(data)) &&
               // ::serialization::parse_binary(data, adr) &&
               check_key(adr.spendPublicKey) &&
               check_key(adr.viewPublicKey);
    }
    ////-----------------------------------------------------------------------
    // bool operator ==(const cryptonote::Transaction& a, const cryptonote::Transaction& b) {
    //   return getObjectHash(a) == getObjectHash(b);
    // }
    ////-----------------------------------------------------------------------
    // bool operator ==(const cryptonote::BlockTemplate& a, const cryptonote::BlockTemplate& b) {

    //  return cryptonote::get_block_hash(a) == cryptonote::get_block_hash(b);
    //}
}

//--------------------------------------------------------------------------------
bool parse_hash256(const std::string &str_hash, crypto::Hash &hash)
{
    return common::podFromHex(str_hash, hash);
}
