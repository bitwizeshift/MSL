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
# define MSL_STANDARD_SOURCE_LOCATION
#elif __has_include(<experimental/source_location>)
# include <experimental/source_location>
# define MSL_EXPERIMENTAL_SOURCE_LOCATION
#elif defined(__has_builtin) && __has_builtin(__builtin_source_location)
# define MSL_BUILTIN_SOURCE_LOCATION
#elif defined(_MSC_VER)
# define MSL_BUILTIN_FILE_AND_LINE
#elif defined(__has_builtin) &&                                               \
      __has_builtin(__builtin_FILE) &&                                        \
      __has_builtin(__builtin_LINE) &&                                        \
      __has_builtin(__builtin_FUNCTION) &&                                    \
      __has_builtin(__builtin_COLUMN)
# define MSL_BUILTIN_FILE_AND_LINE 1
#else
# error "source_location cannot be implemented without knowing the relevant compiler intrinsics"
#endif

#include <cstdint>

namespace msl {
#if defined(MSL_EXPERIMENTAL_SOURCE_LOCATION)
  using std::experimental::source_location; // NOLINT(misc-unused-using-decls)
#elif defined(MSL_STANDARD_SOURCE_LOCATION)
  using std::source_location; // NOLINT(misc-unused-using-decls)
#elif defined(MSL_BUILTIN_SOURCE_LOCATION) || defined(MSL_BUILTIN_FILE_AND_LINE)


  /////////////////////////////////////////////////////////////////////////////
  /// \brief An implementation of C++20's `std::source_location` utility
  ///
  /// It is only possible to implement this using intrinsics, and so this
  /// leverages the underlying compiler's implementation.
  ///
  /// Unfortunately, this results in some `#ifdef`s in the following code in
  /// order for this to work, since different compilers are currently supporting
  /// this in different ways.
  /////////////////////////////////////////////////////////////////////////////
  class source_location
  {
    //-------------------------------------------------------------------------
    // Static Factories
    //-------------------------------------------------------------------------
  public:

    // NOTE: this should be `consteval`, but unfortunately clang has a bug
    // which prevents default-arguments calling consteval functions:
    // https://github.com/llvm/llvm-project/issues/48230
    //
    // This is not as good for performance, but it prevents the failure listed
    // above.
#if defined(MSL_BUILTIN_SOURCE_LOCATION)
    static constexpr auto current(void* p = __builtin_source_location())
      noexcept -> source_location;
#elif defined(MSL_BUILTIN_FILE_AND_LINE)
    static constexpr auto current(
      const char* file = __builtin_FILE(),
      const char* function = __builtin_FUNCTION(),
      unsigned line = __builtin_LINE(),
      unsigned column = __builtin_COLUMN()
    ) noexcept -> source_location;
#endif

    //-------------------------------------------------------------------------
    // Constructors
    //-------------------------------------------------------------------------
  public:

    constexpr source_location() = default;

    source_location(const source_location& other) = default;

    auto operator=(const source_location& other) -> source_location& = default;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    constexpr auto line() const noexcept -> std::uint32_t;
    constexpr auto column() const noexcept -> std::uint32_t;
    constexpr auto file_name() const noexcept -> const char*;
    constexpr auto function_name() const noexcept -> const char*;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    // using the same structure as what is defined in libstdc++:
    // https://github.com/gcc-mirror/gcc/blob/efbb17db52afd802300c4dcce208fab326ec2915/libstdc%2B%2B-v3/include/std/source_location#L78-L84
    struct impl {
      const char* file_name = "";
      const char* function_name = "";
      unsigned line = 0u;
      unsigned column = 0u;
    };

#if defined(MSL_BUILTIN_SOURCE_LOCATION)
    impl* m_impl = nullptr;
    constexpr explicit source_location(impl* i) : m_impl{i}{};
#elif defined(MSL_BUILTIN_FILE_AND_LINE)

    // use fake indirection so that the API is identical to `impl*`
    struct indirect_impl {
      constexpr auto operator->() -> impl* { return &m_impl; }
      constexpr auto operator->() const -> const impl* { return &m_impl; }
      impl m_impl;
    };

    indirect_impl m_impl;
    constexpr explicit source_location(impl i) : m_impl{i}{}
#endif // defined(MSL_BUILTIN_FILE_LOCATION)
  };

#endif // defined(MSL_BUILTIN_SOURCE_LOCATION) || defined(MSL_BUILTIN_FILE_AND_LINE)
} // namespace msl

#if defined(MSL_BUILTIN_SOURCE_LOCATION) || defined(MSL_BUILTIN_FILE_AND_LINE)

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

#if defined(MSL_BUILTIN_SOURCE_LOCATION)

inline constexpr
auto msl::source_location::current(void* p)
  noexcept -> source_location
{
  return source_location{static_cast<const impl*>(p)};
}

#elif defined(MSL_BUILTIN_FILE_AND_LINE)

inline constexpr
auto msl::source_location::current(
  const char* file,
  const char* function,
  unsigned line,
  unsigned column
) noexcept -> source_location
{
  return source_location{impl{file, function, line, column}};
}

#endif // defined(MSL_BUILTIN_FILE_AND_LINE)

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

inline constexpr
auto msl::source_location::line()
  const noexcept -> std::uint32_t
{
  return m_impl->line;
}

inline constexpr
auto msl::source_location::column()
  const noexcept -> std::uint32_t
{
  return m_impl->column;
}

inline constexpr
auto msl::source_location::file_name()
  const noexcept -> const char*
{
  return m_impl->file_name;
}

inline constexpr
auto msl::source_location::function_name()
  const noexcept -> const char*
{
  return m_impl->function_name;
}

#endif // defined(MSL_BUILTIN_SOURCE_LOCATION) || defined(MSL_BUILTIN_FILE_AND_LINE)


// Remove these symbols

#if defined(MSL_BUILTIN_SOURCE_LOCATION)
# undef MSL_BUILTIN_SOURCE_LOCATION
#endif

#if defined(MSL_BUILTIN_FILE_AND_LINE)
# undef MSL_BUILTIN_FILE_AND_LINE
#endif

#if defined(MSL_EXPERIMENTAL_SOURCE_LOCATION)
# undef MSL_EXPERIMENTAL_SOURCE_LOCATION
#endif

#if defined(MSL_STANDARD_SOURCE_LOCATION)
# undef MSL_STANDARD_SOURCE_LOCATION
#endif


#endif /* MSL_UTILITIES_SOURCE_LOCATION_HPP */
