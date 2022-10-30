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

<<<<<<<< HEAD:src/cryptonote_core/add_block_error_condition.cpp
#include "add_block_error_condition.h"

namespace cryptonote
{
    namespace error
    {
        AddBlockErrorConditionCategory AddBlockErrorConditionCategory::INSTANCE;
========
#include "transaction_validation_errors.h"

namespace cryptonote {
namespace error {
>>>>>>>> 20dc9b25 (Refactoring cryptonote package):src/cryptonote_core/transaction_validation_errors.cpp


        std::error_condition make_error_condition(AddBlockErrorCondition e) {
          return std::error_condition(
              static_cast<int>(e),
              AddBlockErrorConditionCategory::INSTANCE);
        }
    }
}
