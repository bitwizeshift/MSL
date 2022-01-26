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
#ifndef MSL_QUANTITIES_ADDRESS_HPP
#define MSL_QUANTITIES_ADDRESS_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include "msl/utilities/intrinsics.hpp"

#include <cstdint> // std::uintptr_t
#include <functional> // std::less, std::greater,

namespace msl {

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A representation of an address in a numerical form
  ///
  /// This is used for semantic identity keeping. Once an address has been
  /// constructed from a pointer, there is no way to return it back to its
  /// strongly-typed form. This is intentional, as crucial information is lost
  /// such as the CV-qualifications.
  /////////////////////////////////////////////////////////////////////////////
  class address
  {
    //-------------------------------------------------------------------------
    // Static Factories
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs an address from a pointer
    ///
    /// \param p the pointer
    /// \return the address
    static constexpr auto of(const void* p) noexcept -> address;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    constexpr address() noexcept = default;
    constexpr address(const address& other) noexcept = default;
    constexpr address(address&& other) noexcept = default;

    constexpr auto operator=(const address& other) noexcept -> address& = default;
    constexpr auto operator=(address&& other) noexcept -> address& = default;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the underlying value of the address
    ///
    /// \return the value
    auto value() const noexcept -> std::uintptr_t;

    //-------------------------------------------------------------------------
    // Comparisons
    //-------------------------------------------------------------------------
  public:

    constexpr auto operator==(const address& other) const noexcept -> bool = default;
    constexpr auto operator<(const address& other) const noexcept -> bool;
    constexpr auto operator>(const address& other) const noexcept -> bool;
    constexpr auto operator<=(const address& other) const noexcept -> bool;
    constexpr auto operator>=(const address& other) const noexcept -> bool;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    // Pointers to values outside of the same sub-object are not orderable
    // using a numeric representation in any way; so instead we store a void
    // pointer that we compare using std::less/std::greater/etc, which
    // guarantees a total-ordering *of some kind* without any
    // undefined-behavior.
    const void* m_address = nullptr;

    //-------------------------------------------------------------------------
    // Private Constructorsdefault
    //-------------------------------------------------------------------------
  private:

    explicit constexpr address(const void* address) noexcept;
  };

} // namespace msl

//=============================================================================
// class : address
//=============================================================================

//-----------------------------------------------------------------------------
// Static Factories
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE constexpr
auto msl::address::of(const void* p)
  noexcept -> address
{
  return address{p};
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE
auto msl::address::value()
  const noexcept -> std::uintptr_t
{
  return reinterpret_cast<std::uintptr_t>(m_address);
}

//-----------------------------------------------------------------------------
// Comparisons
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE constexpr
auto msl::address::operator<(const address& other) const noexcept -> bool
{
  return std::less<const void*>{}(m_address, other.m_address);
}

MSL_FORCE_INLINE constexpr
auto msl::address::operator>(const address& other) const noexcept -> bool
{
  return std::greater<const void*>{}(m_address, other.m_address);
}

MSL_FORCE_INLINE constexpr
auto msl::address::operator<=(const address& other) const noexcept -> bool
{
  return std::less_equal<const void*>{}(m_address, other.m_address);
}

MSL_FORCE_INLINE constexpr
auto msl::address::operator>=(const address& other) const noexcept -> bool
{
  return std::greater_equal<const void*>{}(m_address, other.m_address);
}

//-----------------------------------------------------------------------------
// Private Constructors
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE constexpr
msl::address::address(const void* address)
  noexcept
  : m_address{address}
{

}

#endif /* MSL_QUANTITIES_ADDRESS_HPP */