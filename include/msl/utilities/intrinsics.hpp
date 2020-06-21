///////////////////////////////////////////////////////////////////////////////
/// \file intrinsics.hpp
///
/// \todo(Bitwize): Document this
///////////////////////////////////////////////////////////////////////////////

/*
  The MIT License (MIT)

  Copyright (c) 2020 Matthew Rodusek All rights reserved.

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
#ifndef MSL_UTILITIES_INTRINSICS_HPP
#define MSL_UTILITIES_INTRINSICS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <cstddef> // std::size_t
#include <cstdint> // std::uintptr_t

#if defined(__clang__) || defined(__GNUC__)
# define MSL_LIKELY(x) __builtin_expect(!!(x), 1)
# define MSL_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
# define MSL_LIKELY(x) x
# define MSL_UNLIKELY(x) x
#endif

#if defined(__clang__) || defined(__GNUC__)
# define MSL_FORCE_INLINE __attribute__((always_inline))
#elif defined(_MSC_VER)
# define MSL_FORCE_INLINE __forceinline
#else
# define MSL_FORCE_INLINE
#endif

namespace msl {

  struct intrinsics
  {
    intrinsics() = delete;
    ~intrinsics() = delete;

    template <std::size_t N, typename T>
    static constexpr T* assume_aligned(T* p) noexcept;

    template <typename T>
    static constexpr void unused(const T&);
  };

} // namespace msl

template <std::size_t N, typename T>
MSL_FORCE_INLINE constexpr T* msl::intrinsics::assume_aligned(T* p)
  noexcept
{
#if defined(__clang__) || (defined(__GNUC__) && !defined(__ICC))
  return static_cast<T*>(__builtin_assume_aligned(p, N));
#elif defined(_MSC_VER)
  if ((reinterpret_cast<std::uintptr_t>(p) & ((1 << N) - 1)) == 0) {
    return p;
  } else {
    __assume(0);
  }
#elif defined(__ICC)
  switch (N) {
    case 2:
      __assume_aligned(p, 2);
      break;
    case 4:
      __assume_aligned(p, 4);
      break;
    case 8:
      __assume_aligned(p, 8);
      break;
    case 16:
      __assume_aligned(p, 16);
      break;
    case 32:
      __assume_aligned(p, 32);
      break;
    case 64:
      __assume_aligned(p, 64);
      break;
    case 128:
      __assume_aligned(p, 128);
      break;
  }
  return p;
#else // Unknown compiler — do nothing
  return p;
#endif
}


#endif /* MSL_UTILITIES_INTRINSICS_HPP */
