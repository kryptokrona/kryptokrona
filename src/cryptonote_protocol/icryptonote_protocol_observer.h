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

#include <cstddef>
#include <cstdint>
#include <cstddef>

<<<<<<<< HEAD:src/cryptonote_protocol/icryptonote_protocol_observer.h
namespace cryptonote
{
    class ICryptoNoteProtocolObserver {
    public:
      virtual void peerCountUpdated(size_t count) {}
      virtual void lastKnownBlockHeightUpdated(uint32_t height) {}
      virtual void blockchainSynchronized(uint32_t topHeight) {}
    };
========
namespace cryptonote {

class ICoreInformation {
public:
  virtual ~ICoreInformation() {}

  virtual size_t getPoolTransactionCount() const = 0;
  virtual size_t getBlockchainTransactionCount() const = 0;
  virtual size_t getAlternativeBlockCount() const = 0;
  virtual std::vector<Transaction> getPoolTransactions() const = 0;
};

>>>>>>>> 20dc9b25 (Refactoring cryptonote package):src/cryptonote_core/icore_information.h
}
