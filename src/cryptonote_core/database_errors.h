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

#include <string>
#include <system_error>

namespace CryptoNote {
namespace error {

enum class DataBaseErrorCodes : int {
  NOT_INITIALIZED = 1,
  ALREADY_INITIALIZED,
  INTERNAL_ERROR,
  IO_ERROR
};

class DataBaseErrorCategory : public std::error_category {
public:
  static DataBaseErrorCategory INSTANCE;

  virtual const char* name() const throw() override {
    return "DataBaseErrorCategory";
  }

  virtual std::error_condition default_error_condition(int ev) const throw() override {
    return std::error_condition(ev, *this);
  }

  virtual std::string message(int ev) const override {
    switch (ev) {
      case static_cast<int>(DataBaseErrorCodes::NOT_INITIALIZED) : return "Object was not initialized";
      case static_cast<int>(DataBaseErrorCodes::ALREADY_INITIALIZED) : return "Object has been already initialized";
      case static_cast<int>(DataBaseErrorCodes::INTERNAL_ERROR) : return "Internal error";
      case static_cast<int>(DataBaseErrorCodes::IO_ERROR) : return "IO error";
      default: return "Unknown error";
    }
  }

private:
  DataBaseErrorCategory() {

  }
};

} //namespace error
} //namespace CryptoNote

inline std::error_code make_error_code(CryptoNote::error::DataBaseErrorCodes e) {
  return std::error_code(static_cast<int>(e), CryptoNote::error::DataBaseErrorCategory::INSTANCE);
}
