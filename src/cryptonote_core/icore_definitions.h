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
#include <cryptonote.h>
#include <crypto_types.h>
#include <wallet_types.h>

namespace cryptonote
{
    struct BlockFullInfo : public RawBlock {
      crypto::Hash block_id;
    };

    struct TransactionPrefixInfo {
      crypto::Hash txHash;
      TransactionPrefix txPrefix;
    };

    struct BlockShortInfo {
      crypto::Hash blockId;
      BinaryArray block;
      std::vector<TransactionPrefixInfo> txPrefixes;
    };

    void serialize(BlockFullInfo&, ISerializer&);
    void serialize(TransactionPrefixInfo&, ISerializer&);
    void serialize(BlockShortInfo&, ISerializer&);

    void serialize(WalletTypes::WalletBlockInfo &walletBlockInfo, ISerializer &s);
    void serialize(WalletTypes::RawTransaction &rawTransaction, ISerializer &s);
    void serialize(WalletTypes::RawCoinbaseTransaction &rawCoinbaseTransaction, ISerializer &s);
    void serialize(WalletTypes::KeyOutput &keyOutput, ISerializer &s);
}
