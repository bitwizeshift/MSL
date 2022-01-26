////////////////////////////////////////////////////////////////////////////////
/// \file source_location.hpp.h
///
/// \brief TODO(Bitwize): Add description
////////////////////////////////////////////////////////////////////////////////

/*
  The MIT License (MIT)

  Copyright (c) 2021 Matthew Rodusek All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef MSL_UTILITIES_SOURCE_LOCATION_HPP
#define MSL_UTILITIES_SOURCE_LOCATION_HPP

// clang-13 using libstdc++11.2 fails due to clang not defining
// __builtin_source_location as an intrinsic. Without this, we have to use the
// experimental source location that uses __builtin_LINE and other intrinsics
//
// There isn't a great way to detect the standard library used, thus this
// is checked by detecting clang on non macos systems, where libc++ isn't likely
// to be used.
#if defined(__clang__) && (defined(__linux__) || defined(_WIN32)) && \
    __has_include(<experimental/source_location>)
# include <experimental/source_location>
# define MSL_EXPERIMENTAL_SOURCE_LOCATION
#elif __has_include(<source_location>)
# include <source_location>
#elif __has_include(<experimental/source_location>)
# include <experimental/source_location>
# define MSL_EXPERIMENTAL_SOURCE_LOCATION
#else
# error "No 'source_location' header discovered"
#endif

namespace msl {
#if defined(MSL_EXPERIMENTAL_SOURCE_LOCATION)
  using std::experimental::source_location; // NOLINT(misc-unused-using-decls)
#else
  using std::source_location; // NOLINT(misc-unused-using-decls)
#endif
} // namespace msl

#endif /* MSL_UTILITIES_SOURCE_LOCATION_HPP */
