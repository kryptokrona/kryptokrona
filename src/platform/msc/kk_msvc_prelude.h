// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

// This header is force-included into every translation unit on MSVC (via the
// compiler's /FI option; see the MSVC branch of the top-level CMakeLists.txt).
//
// MSVC's standard library headers are leaner than libc++/libstdc++ and do NOT
// transitively pull in <stdexcept>, so the many source files that use
// std::runtime_error without including it directly fail to compile (error C2039
// 'runtime_error' is not a member of 'std'). Force-including it everywhere fixes
// that in one place.
//
// Keep this list MINIMAL. It is force-included at the very top of every C++ TU,
// before that TU's own includes, so heavy headers here can break unrelated code
// via include-ordering (e.g. <chrono> drags in <format>/<string_view>, whose
// operator<< needs a complete <ostream> that isn't available yet -> "use of
// undefined type 'std::basic_ostream'"). Prefer fixing one-off needs at the
// source (e.g. <chrono> was added directly to rocksdb's env_win.cc).
//
// The __cplusplus guard is essential: the Visual Studio generator applies C++
// compile flags at the project level, so this header also gets force-included
// into the project's C sources (e.g. src/crypto/*.c). Including a C++ standard
// header from a .c file triggers "error STL1003: Unexpected compiler, expected
// C++ compiler", so we make this a no-op for C.

#pragma once

#ifdef __cplusplus
// <ostream> MUST come first. Being force-included at the very top of every TU,
// <stdexcept> pulls in <string> -> __msvc_string_view.hpp, whose operator<< for
// string_view is compiled against std::basic_ostream. If <ostream> hasn't been
// seen yet, basic_ostream is only forward-declared and MSVC errors with
// "C2027: use of undefined type 'std::basic_ostream'" (plus iostate/goodbit/
// badbit undeclared) in EVERY translation unit. Including <ostream> first makes
// basic_ostream complete before that operator is parsed.
#include <ostream>
#include <stdexcept>
#endif
