// Copyright (c) 2019, The Kryptokrona Developers
//
// Please see the included LICENSE file for more information.

// This header is force-included into every translation unit on MSVC (via the
// compiler's /FI option; see the MSVC branch of the top-level CMakeLists.txt).
//
// MSVC's standard library headers are leaner than libc++/libstdc++ and do NOT
// transitively pull in <stdexcept> or <chrono>, so the many source files that
// use std::runtime_error or std::chrono without including them directly fail to
// compile (error C2039 'runtime_error' / C2653 'system_clock'). Force-including
// these headers everywhere fixes that in one place.
//
// The __cplusplus guard is essential: the Visual Studio generator applies C++
// compile flags at the project level, so this header also gets force-included
// into the project's C sources (e.g. src/crypto/*.c). Including a C++ standard
// header from a .c file triggers "error STL1003: Unexpected compiler, expected
// C++ compiler", so we make this a no-op for C.

#pragma once

#ifdef __cplusplus
#include <stdexcept>
#include <chrono>
#endif
