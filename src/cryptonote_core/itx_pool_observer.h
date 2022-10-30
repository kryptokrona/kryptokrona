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

<<<<<<<< HEAD:src/cryptonote_core/itx_pool_observer.h
namespace cryptonote
{
    class ITxPoolObserver {
    public:
      virtual ~ITxPoolObserver() {
      }
========
namespace cryptonote {

class ICoreObserver {
public:
  virtual ~ICoreObserver() {};
  virtual void blockchainUpdated() {};
  virtual void poolUpdated() {};
};
>>>>>>>> 20dc9b25 (Refactoring cryptonote package):src/cryptonote_core/icore_observer.h

      virtual void txDeletedFromPool() = 0;
    };
}
