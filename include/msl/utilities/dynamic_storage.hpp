/*
 The MIT License (MIT)

 Copyright (c) 2022 Matthew Rodusek All rights reserved.

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
#ifndef MSL_UTILITIES_DYNAMIC_STORAGE_HPP
#define MSL_UTILITIES_DYNAMIC_STORAGE_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include "msl/utilities/intrinsics.hpp"
#include "msl/utilities/assert.hpp"

#include <cstdint>
#include <cstddef>
#include <concepts>
#include <limits>
#include <compare>

namespace msl {

  template <typename T>
    requires(std::integral<T>)
  inline constexpr auto dynamic_storage_sentinel = std::numeric_limits<T>::max();

  /////////////////////////////////////////////////////////////////////////////
  /// \brief Dynamic storage object that conditionally decides whether to use
  ///        static or runtime storage based on the supplied value
  ///
  /// \tparam T the type
  /// \tparam N the compile-time value supplied
  /// \tparam Sentinel the value to be used for runtime values
  /////////////////////////////////////////////////////////////////////////////
  template <typename T, T N, T Sentinel = dynamic_storage_sentinel<T>>
    requires(std::integral<T>)
  class dynamic_storage
  {
    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using value_type = T;

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
  public:

    static constexpr auto is_static_value = true;
    static constexpr auto sentinel = Sentinel;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    dynamic_storage() = default;
    explicit constexpr dynamic_storage(T n) noexcept;

    dynamic_storage(const dynamic_storage& other) = default;
    auto operator=(const dynamic_storage& other) -> dynamic_storage& = default;

    //-------------------------------------------------------------------------
    // Comparisons
    //-------------------------------------------------------------------------
  public:

    auto operator==(const dynamic_storage& other) const -> bool = default;
    auto operator<=>(const dynamic_storage& other) const = default;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    consteval auto value() const noexcept -> T;
  };

  template <typename T, T N>
  class dynamic_storage<T, N, N>
  {
    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using value_type = T;

    //-------------------------------------------------------------------------
    // Public Static Constants
    //-------------------------------------------------------------------------
  public:

    static constexpr auto is_static_value = false;
    static constexpr auto sentinel = N;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    explicit constexpr dynamic_storage(T n) noexcept;

    dynamic_storage(const dynamic_storage& other) = default;

    //-------------------------------------------------------------------------

    auto operator=(const dynamic_storage& other) -> dynamic_storage& = default;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    constexpr auto value() const noexcept -> T;

    //-------------------------------------------------------------------------
    // Comparisons
    //-------------------------------------------------------------------------
  public:

    auto operator==(const dynamic_storage& other) const -> bool = default;
    auto operator<=>(const dynamic_storage& other) const = default;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    T m_value;
  };

  //---------------------------------------------------------------------------
  // Aliases
  //---------------------------------------------------------------------------

  template <std::size_t N>
  using dynamic_size = dynamic_storage<std::size_t, N>;

  template <std::size_t N>
  using dynamic_uint8 = dynamic_storage<std::uint8_t, N>;
  template <std::size_t N>
  using dynamic_uint16 = dynamic_storage<std::uint16_t, N>;
  template <std::size_t N>
  using dynamic_uint32 = dynamic_storage<std::uint32_t, N>;
  template <std::size_t N>
  using dynamic_uint64 = dynamic_storage<std::uint64_t, N>;

  template <std::size_t N>
  using dynamic_int8 = dynamic_storage<std::int8_t, N>;
  template <std::size_t N>
  using dynamic_int16 = dynamic_storage<std::int16_t, N>;
  template <std::size_t N>
  using dynamic_int32 = dynamic_storage<std::int32_t, N>;
  template <std::size_t N>
  using dynamic_int64 = dynamic_storage<std::int64_t, N>;

} // namespace msl

//=============================================================================
// class : dynamic_storage<T,N,Sentinel>
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, T N, T Sentinel>
requires(std::integral<T>)
MSL_FORCE_INLINE constexpr
msl::dynamic_storage<T,N,Sentinel>::dynamic_storage(T n)
  noexcept
{
  MSL_ASSERT(n == N);
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, T N, T Sentinel>
requires(std::integral<T>)
consteval
auto msl::dynamic_storage<T,N,Sentinel>::value()
  const noexcept -> T
{
  return N;
}

//=============================================================================
// class : dynamic_storage<T,N,N>
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, T N>
MSL_FORCE_INLINE constexpr
msl::dynamic_storage<T,N,N>::dynamic_storage(T n)
  noexcept
  : m_value{n}
{
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, T N>
MSL_FORCE_INLINE constexpr
auto msl::dynamic_storage<T,N,N>::value()
  const noexcept -> T
{
  return m_value;
}

#endif /* MSL_UTILITIES_DYNAMIC_STORAGE_HPP */