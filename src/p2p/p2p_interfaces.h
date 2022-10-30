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

#include <cstdint>
#include <memory>

<<<<<<<< HEAD:src/p2p/p2p_interfaces.h
#include <cryptonote.h>
========
namespace cryptonote {
>>>>>>>> 20dc9b25 (Refactoring cryptonote package):src/cryptonote_core/iupgrade_detector.h

namespace cryptonote
{
    struct P2pMessage {
      uint32_t type;
      BinaryArray data;
    };

    class IP2pConnection {
    public:
      virtual ~IP2pConnection();
      virtual void read(P2pMessage &message) = 0;
      virtual void write(const P2pMessage &message) = 0;
      virtual void stop() = 0;
    };

    class IP2pNode {
    public:
      virtual void stop() = 0;
    };
}
