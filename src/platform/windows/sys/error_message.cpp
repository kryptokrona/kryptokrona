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

<<<<<<<< HEAD:src/platform/windows/sys/error_message.cpp
#include "error_message.h"
========
#include "upgrade_manager.h"
>>>>>>>> 20dc9b25 (Refactoring cryptonote package):src/cryptonote_core/upgrade_manager.cpp

#ifndef NOMINMAX
#define NOMINMAX
#endif

<<<<<<<< HEAD:src/platform/windows/sys/error_message.cpp
#include <cstddef>
#include <windows.h>
========
namespace cryptonote {
>>>>>>>> 20dc9b25 (Refactoring cryptonote package):src/cryptonote_core/upgrade_manager.cpp

namespace sys
{
    std::string lastErrorMessage() {
      return errorMessage(GetLastError());
    }

    std::string errorMessage(int error) {
      struct Buffer {
        ~Buffer() {
          if (pointer != nullptr) {
            LocalFree(pointer);
          }
        }

        LPTSTR pointer = nullptr;
      } buffer;

      auto size = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, error,
                                MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), reinterpret_cast<LPTSTR>(&buffer.pointer), 0, nullptr);
      return "result=" + std::to_string(error) + ", " + std::string(buffer.pointer, size);
    }
}
