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

#include "common/string_tools.h"
#include "crypto/crypto.h"
#include "crypto/hash.h"
#include "cryptonote_core/cryptonote_basic.h"


namespace cryptonote
{
  /************************************************************************/
  /* CryptoNote helper functions                                          */
  /************************************************************************/
  uint64_t getPenalizedAmount(uint64_t amount, size_t medianSize, size_t currentBlockSize);
  std::string getAccountAddressAsStr(uint64_t prefix, const AccountPublicAddress& adr);
  bool parseAccountAddressString(uint64_t& prefix, AccountPublicAddress& adr, const std::string& str);
}

bool parse_hash256(const std::string& str_hash, crypto::Hash& hash);
