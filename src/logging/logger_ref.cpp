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

<<<<<<<< HEAD:src/logging/logger_ref.cpp
#include "logger_ref.h"

namespace logging
{
    LoggerRef::LoggerRef(std::shared_ptr<ILogger> logger, const std::string& category) : logger(logger), category(category) {
    }
========
#include "add_block_error_condition.h"

namespace cryptonote {
namespace error {
>>>>>>>> 20dc9b25 (Refactoring cryptonote package):src/cryptonote_core/add_block_error_condition.cpp

    LoggerMessage LoggerRef::operator()(Level level, const std::string& color) const {
      return LoggerMessage(logger, category, level, color);
    }

    std::shared_ptr<ILogger> LoggerRef::getLogger() const {
      return logger;
    }
}
