// Copyright (c) 2012-2017, The CryptoNote developers, The Bytecoin developers
// Copyright (c) 2014-2018, The Monero Project
// Copyright (c) 2018-2019, The TurtleCoin Developers
// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

#include "cryptonote_tools.h"
#include "cryptonote_format_utils.h"

using namespace cryptonote;

template <>
bool cryptonote::toBinaryArray(const BinaryArray &object, BinaryArray &binaryArray)
{
    try
    {
        common::VectorOutputStream stream(binaryArray);
        BinaryOutputStreamSerializer serializer(stream);
        std::string oldBlob = common::asString(object);
        serializer(oldBlob, "");
    }
    catch (std::exception &)
    {
        return false;
    }

    return true;
}

void cryptonote::getBinaryArrayHash(const BinaryArray &binaryArray, crypto::Hash &hash)
{
    cn_fast_hash(binaryArray.data(), binaryArray.size(), hash);
}

crypto::Hash cryptonote::getBinaryArrayHash(const BinaryArray &binaryArray)
{
    crypto::Hash hash;
    getBinaryArrayHash(binaryArray, hash);
    return hash;
}

uint64_t cryptonote::getInputAmount(const Transaction &transaction)
{
    uint64_t amount = 0;
    for (auto &input : transaction.inputs)
    {
        if (input.type() == typeid(KeyInput))
        {
            amount += boost::get<KeyInput>(input).amount;
        }
    }

    return amount;
}

std::vector<uint64_t> cryptonote::getInputsAmounts(const Transaction &transaction)
{
    std::vector<uint64_t> inputsAmounts;
    inputsAmounts.reserve(transaction.inputs.size());

    for (auto &input : transaction.inputs)
    {
        if (input.type() == typeid(KeyInput))
        {
            inputsAmounts.push_back(boost::get<KeyInput>(input).amount);
        }
    }

    return inputsAmounts;
}

uint64_t cryptonote::getOutputAmount(const Transaction &transaction)
{
    uint64_t amount = 0;
    for (auto &output : transaction.outputs)
    {
        amount += output.amount;
    }

    return amount;
}

void cryptonote::decomposeAmount(uint64_t amount, uint64_t dustThreshold, std::vector<uint64_t> &decomposedAmounts)
{
    decompose_amount_into_digits(
        amount, dustThreshold,
        [&](uint64_t amount)
        {
            decomposedAmounts.push_back(amount);
        },
        [&](uint64_t dust)
        {
            decomposedAmounts.push_back(dust);
        });
}
