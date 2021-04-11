////////////////////////////////////////////////////////////////////////////////
/// \file intrinsics.hpp
///
/// \brief This header provides a collection of useful compiler-intrinsics in
///        the form of `constexpr functions
////////////////////////////////////////////////////////////////////////////////

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

#if defined(_MSC_VER)
# pragma once
#endif // defined(_MSC_VER)

#include <cstddef> // std::size_t
#include <cstdint> // std::uintptr_t

#if !defined(__clang__) || defined(__GNUC__) || defined(_MSC_VER)
# include <cstdlib> // std::abort
#endif

#if defined(__clang__) || defined(__GNUC__)
# define MSL_FORCE_INLINE [[gnu::always_inline]] inline
#elif defined(_MSC_VER)
# define MSL_FORCE_INLINE __forceinline
#else
# define MSL_FORCE_INLINE
#endif

#if defined(__clang__) || defined(__GNUC__)
# define MSL_COLD [[gnu::cold]]
# define MSL_HOT [[gnu::hot]]
#else
# define MSL_COLD
# define MSL_HOT
#endif

#if __has_cpp_attribute(likely) >= 201803L
# define MSL_LIKELY [[likely]]
#else
# define MSL_LIKELY
#endif

#if __has_cpp_attribute(unlikely) >= 201803L
# define MSL_UNLIKELY [[unlikely]]
#else
# define MSL_UNLIKELY
#endif

namespace msl {

  struct intrinsics
  {
    intrinsics() = delete;
    ~intrinsics() = delete;

    //-------------------------------------------------------------------------
    // Unused
    //-------------------------------------------------------------------------

    /// \brief A meta function to indicate to the compiler that a sequence
    ///        of types is not used
    ///
    /// This is used to suppress 'unused template argument' warnings, and
    /// to indicate to the reader that a type is intentionally unused
    template <typename...>
    static constexpr auto suppress_unused() noexcept -> void {}

    /// \brief A meta function to indicate to the compiler that a sequence
    ///        of values is unused
    ///
    /// This is used to suppress 'unused variable' warnings, and to indicate
    /// to the reader that a variable is intentionally unused
    template <typename...Types>
    static constexpr auto suppress_unused([[maybe_unused]] const Types&...) noexcept -> void {}

    /// \brief A meta function to indicate to the compiler that a static value
    ///        is unused
    ///
    /// This is used to suppress 'unused variable' warnings, and to indicate
    /// to the reader that a variable is intentionally unused
    template <auto...>
    static constexpr auto suppress_unused() noexcept -> void{}

    /// \brief Provides a hint to the compiler that the pointer \p p is aligned
    ///        to the \p N boundary
    ///
    /// \note The returned pointer is the only pointer known to the compiler to
    ///       be aligned; the input should be discarded.
    ///
    /// \tparam N the alignment boundary of the pointer
    /// \param p the pointer to indicate alignment of
    /// \return a pointer that the compiler knows is aligned
    template <std::size_t N, typename T>
    [[nodiscard]]
    static constexpr auto assume_aligned(T* p) noexcept -> T*;

    /// \brief Provides a hint to the compiler that the pointer \p p is not null
    ///
    /// \note The returned pointer is the only pointer known to the compiler to
    ///        be not-null; the input should be discarded
    ///
    /// This may help to avoid certain generated assemblies which make
    /// assumptions on the result of a pointer being null by allowing any
    /// related code paths to be completely optimized out.
    ///
    /// \param p the pointer to indicate is not-null
    /// \return a pointer that the compiler knows is not null
    template <typename T>
#if defined(__GNUC__) || defined(__clang__)
    [[gnu::returns_nonnull]]
#endif
    [[nodiscard]]
    static constexpr auto assume_not_null(T* p) noexcept -> T*;

    /// \brief Provides a hint to the compiler that the current path leading to
    ///        this call is not reachable
    ///
    /// In the event that an unknown compiler is used, this will simply call
    /// std::abort if executed.
    [[noreturn]]
    static auto unreachable() noexcept -> void;

  };

} // namespace msl

template <std::size_t N, typename T>
MSL_FORCE_INLINE constexpr
auto msl::intrinsics::assume_aligned(T* p)
  noexcept -> T*
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

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::intrinsics::assume_not_null(T* p)
  noexcept -> T*
{
#if defined(_MSC_VER)
  if (p != nullptr) {
    return p;
  } else {
    __assume(0);
  }
#else
  // For gcc / clang, [[gnu::returns_nonnull]] upholds the non-nullability
  // invariant
  return p;
#endif
}

MSL_FORCE_INLINE
auto msl::intrinsics::unreachable() noexcept -> void
{
#if defined(_MSC_VER)
  __assume(0);
#elif defined(__GNUC__) || defined(__clang__)
  __builtin_unreachable();
#else
  // We have no other way to convey that this function will not return to help
  // the optimizer. The best we really can do is to just call std::abort, which
  // should only result in a single `call` instruction *at worst* if the compiler
  // can't see that a branch isn't taken.
  std::abort();
#endif
}
#endif /* MSL_UTILITIES_INTRINSICS_HPP */
