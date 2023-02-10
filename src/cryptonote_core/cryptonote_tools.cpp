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
