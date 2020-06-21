///////////////////////////////////////////////////////////////////////////////
/// \file cell.hpp
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
#ifndef MSL_CELLS_CELL_HPP
#define MSL_CELLS_CELL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/byte_or_quantity.hpp"

#include "msl/utilities/quantity.hpp"
#include "msl/utilities/digital_quantity.hpp"
#include "msl/utilities/not_null.hpp"
#include "msl/utilities/attributes.hpp"
#include "msl/utilities/intrinsics.hpp"

#include <cstddef> // std::size_t
#include <type_traits> // std::make_signed_t

namespace msl {

  //===========================================================================
  // class : cell
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief Represents a memory cell aligned to a specific alignment.
  ///
  /// Memory cells are effectively "fat pointers" that are aware of both the
  /// underlying type of the pointed-to cell and the length of the cell, that
  /// encodes the alignment in its type.
  ///
  /// This allows for these "fat pointer" types to distinguish overload sets
  /// for both overaligned types and normal aligned types. For example, it's
  /// possible to do:
  ///
  /// \code
  /// void simd_add(cell<float[4],64>& lhs, cell<float[4],64>& rhs);
  /// void simd_add(cell<float[4]>& lhs, cell<float[4]>& rhs);
  /// \endcode
  ///
  /// to provide an overload set for float that may use SIMD instructions if
  /// possible.
  ///
  /// \note Unlike pointers, "cell" objects cannot take on the form of either a
  ///       covariant or hierarchically related type. This is by-design to
  ///       prevent loss of crucial information required for allocators
  ///
  /// \note "cell" objects, like pointers, do not propagate CV-qualifiers,
  ///       since the constness of the cell indicates whether the cell itself
  ///       may be reseated.
  ///
  /// \note "cell" objects do *not* refer to objects with an active lifetime;
  ///       they simply refer to memory segments that contain data suitably
  ///       aligned to operate on T types. To refer to an object that does have
  ///       a valid lifetime, see active_cell.
  ///
  /// \tparam T The underlying type of the cell
  /// \tparam Align The alignment of the cell (default is alignof(T)
  /////////////////////////////////////////////////////////////////////////////
  template <typename T, std::size_t Align = alignof(T)>
  class cell
  {
    static_assert(
      !std::is_void_v<T>,
      "cell<void> is ill-formed."
    );

    static_assert(
      !std::is_reference_v<T>,
      "cell<T&> is ill-formed."
    );

    static_assert(
      Align >= alignof(T),
      "Alignment cannot be under-aligned."
    );

    static_assert(
      (Align & (Align-1)) == Align,
      "Alignment must be a power-of-two."
    );

    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using element_type = uninitialized<T>;
    using pointer      = element_type*;
    using reference    = element_type&;

    using size_type   = bytes;
    using length_type = detail::byte_or_quantity<T>;
    using index_type  = std::make_signed_t<std::size_t>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs a cell that does not reference an active cell
    constexpr cell() noexcept;

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    cell(cell&& other) noexcept;
    cell(const cell&) = delete;

    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    explicit constexpr cell(not_null<uninitialized<T>*> p) noexcept;

    //-------------------------------------------------------------------------

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    /// \return reference to (*this)
    cell& operator=(cell&& other) noexcept;
    cell& operator=(const cell& other) = delete;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the pointer from this cell
    ///
    /// \note The data pointed to by cells are not initialized and should not
    ///       be dereferenced
    ///
    /// \return the underlying pointer
    constexpr uninitialized<T>* data() const noexcept;

    /// \brief Gets the size of this memory cell, in bytes
    ///
    /// \return the size of this cell in bytes
    constexpr size_type size() const noexcept;

    /// \brief Gets the length of this cell
    ///
    /// If the underlying type of the cell is bytes,
    ///
    /// \return
    constexpr length_type length() const noexcept;

    /// \brief Queries whether this cell contains a pointer
    ///
    /// \return true if this cell contains a pointer
    constexpr explicit operator bool() const noexcept;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    uninitialized<T>* m_data;
  };

  //===========================================================================
  // class : cell<T[], Align>
  //===========================================================================

  template <typename T, std::size_t Align>
  class cell<T[], Align>
  {
    static_assert(
      !std::is_void_v<T>,
      "cell<void> is ill-formed."
    );

    static_assert(
      !std::is_reference_v<T>,
      "cell<T&> is ill-formed."
    );

    static_assert(
      Align >= alignof(T),
      "Alignment cannot be under-aligned."
    );

    static_assert(
      (Align & (Align-1)) == Align,
      "Alignment must be a power-of-two."
    );

    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using element_type = uninitialized<T>;
    using pointer      = element_type*;
    using reference    = element_type&;

    using size_type   = bytes;
    using length_type = detail::byte_or_quantity<T>;
    using index_type  = std::make_signed_t<std::size_t>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs a cell that does not reference an active cell
    constexpr cell() noexcept;

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    cell(cell&& other) noexcept;
    cell(const cell&) = delete;

    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    constexpr cell(not_null<uninitialized<T>*> p, std::size_t length) noexcept;
    constexpr cell(not_null<uninitialized<T>*> p, length_type length) noexcept;

    //-------------------------------------------------------------------------

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    /// \return reference to (*this)
    cell& operator=(cell&& other) noexcept;
    cell& operator=(const cell& other) = delete;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the pointer from this cell
    ///
    /// \note The data pointed to by cells are not initialized and should not
    ///       be dereferenced
    ///
    /// \return the underlying pointer
    constexpr uninitialized<T>* data() const noexcept;

    /// \brief Gets the size of this memory cell, in bytes
    ///
    /// \return the size of this cell in bytes
    constexpr size_type size() const noexcept;

    /// \brief Gets the length of this cell
    ///
    /// If the underlying type of the cell is bytes,
    ///
    /// \return
    constexpr length_type length() const noexcept;

    /// \brief Queries whether this cell contains a pointer
    ///
    /// \return true if this cell contains a pointer
    constexpr explicit operator bool() const noexcept;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    uninitialized<T>* m_data;
    length_type m_length;
  };

  //===========================================================================
  // class : cell<T[N], Align>
  //===========================================================================

  template <typename T, std::size_t N, std::size_t Align>
  class cell<T[N], Align>
  {
    static_assert(
      !std::is_void_v<T>,
      "cell<void> is ill-formed."
    );

    static_assert(
      !std::is_reference_v<T>,
      "cell<T&> is ill-formed."
    );

    static_assert(
      Align >= alignof(T),
      "Alignment cannot be under-aligned."
    );

    static_assert(
      (Align & (Align-1)) == Align,
      "Alignment must be a power-of-two."
    );

    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using element_type    = uninitialized<T>;
    using pointer         = element_type*;
    using reference       = element_type&;

    using size_type   = bytes;
    using length_type = detail::byte_or_quantity<T>;
    using index_type  = std::make_signed_t<std::size_t>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs a cell that does not reference an active cell
    constexpr cell() noexcept;

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    cell(cell&& other) noexcept;
    cell(const cell&) = delete;

    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    explicit constexpr cell(not_null<uninitialized<T>*> p) noexcept;

    //-------------------------------------------------------------------------

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    /// \return reference to (*this)
    cell& operator=(cell&& other) noexcept;
    cell& operator=(const cell& other) = delete;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the pointer from this cell
    ///
    /// \note The data pointed to by cells are not initialized and should not
    ///       be dereferenced
    ///
    /// \return the underlying pointer
    constexpr uninitialized<T>* data() const noexcept;

    /// \brief Gets the size of this memory cell, in bytes
    ///
    /// \return the size of this cell in bytes
    constexpr size_type size() const noexcept;

    /// \brief Gets the length of this cell
    ///
    /// If the underlying type of the cell is bytes,
    ///
    /// \return
    constexpr length_type length() const noexcept;

    /// \brief Queries whether this cell contains a pointer
    ///
    /// \return true if this cell contains a pointer
    constexpr explicit operator bool() const noexcept;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    uninitialized<T>* m_data;
  };

  //===========================================================================
  // non-member functions : class : cell
  //===========================================================================

  //---------------------------------------------------------------------------
  // Equality Operators
  //---------------------------------------------------------------------------

  template <typename T, std::size_t Align>
  constexpr bool operator==(const cell<T, Align>& lhs, const cell<T, Align>& rhs) noexcept;
  template <typename T, std::size_t Align>
  constexpr bool operator!=(const cell<T, Align>& lhs, const cell<T, Align>& rhs) noexcept;

  template <typename T, std::size_t Align>
  constexpr bool operator==(const cell<T[], Align>& lhs, const cell<T[], Align>& rhs) noexcept;
  template <typename T, std::size_t Align>
  constexpr bool operator!=(const cell<T[], Align>& lhs, const cell<T[], Align>& rhs) noexcept;

  template <typename T, std::size_t N, std::size_t Align>
  constexpr bool operator==(const cell<T[N], Align>& lhs, const cell<T[N], Align>& rhs) noexcept;
  template <typename T, std::size_t N, std::size_t Align>
  constexpr bool operator!=(const cell<T[N], Align>& lhs, const cell<T[N], Align>& rhs) noexcept;

} // namespace msl

//=============================================================================
// definitions : class : cell
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::cell<T, Align>::cell()
  noexcept
  : m_data{nullptr}
{

}

template <typename T, std::size_t Align>
inline msl::cell<T, Align>::cell(cell&& other)
  noexcept
  : m_data{other.m_data}
{
  other.m_data = nullptr;
}

template <typename T, std::size_t Align>
inline constexpr msl::cell<T, Align>::cell(not_null<uninitialized<T>*> p)
  noexcept
  : m_data{p.get()}
{

}

//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
inline msl::cell<T, Align>& msl::cell<T, Align>::operator=(cell&& other)
  noexcept
{
  m_data = other.m_data;
  other.m_data = nullptr;

  return (*this);
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::uninitialized<T>* msl::cell<T, Align>::data()
  const noexcept
{
  return intrinsics::assume_aligned<Align>(m_data);
}

template <typename T, std::size_t Align>
constexpr typename msl::cell<T, Align>::size_type msl::cell<T, Align>::size()
  const noexcept
{
  return length().count() * size_of<T>();
}

template <typename T, std::size_t Align>
constexpr typename msl::cell<T, Align>::length_type msl::cell<T, Align>::length()
  const noexcept
{
  return length_type{1};
}

template <typename T, std::size_t Align>
inline constexpr msl::cell<T, Align>::operator bool()
  const noexcept
{
  return m_data != nullptr;
}

//=============================================================================
// definitions : class : cell<T[], Align>
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::cell<T[], Align>::cell()
  noexcept
  : m_data{nullptr},
    m_length{0}
{

}

template <typename T, std::size_t Align>
inline msl::cell<T[], Align>::cell(cell&& other)
  noexcept
  : m_data{other.m_data},
    m_length{other.m_length}
{
  other.m_data = nullptr;
}

template <typename T, std::size_t Align>
inline constexpr msl::cell<T[], Align>::cell(not_null<uninitialized<T>*> p, std::size_t length)
  noexcept
  : m_data{p.get()},
    m_length(length)
{

}

template <typename T, std::size_t Align>
inline constexpr msl::cell<T[], Align>::cell(not_null<uninitialized<T>*> p, length_type length)
  noexcept
  : m_data{p.get()},
    m_length{length}
{

}

//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
inline msl::cell<T[], Align>& msl::cell<T[], Align>::operator=(cell&& other)
  noexcept
{
  m_data = other.m_data;
  m_length = other.m_length;

  other.m_data = nullptr;

  return (*this);
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::uninitialized<T>* msl::cell<T[], Align>::data()
  const noexcept
{
  return intrinsics::assume_aligned<Align>(m_data);
}

template <typename T, std::size_t Align>
constexpr typename msl::cell<T[], Align>::size_type msl::cell<T[], Align>::size()
  const noexcept
{
  return length().count() * size_of<T>();
}

template <typename T, std::size_t Align>
constexpr typename msl::cell<T[], Align>::length_type msl::cell<T[], Align>::length()
  const noexcept
{
  return m_length;
}

template <typename T, std::size_t Align>
inline constexpr msl::cell<T[], Align>::operator bool()
  const noexcept
{
  return m_data != nullptr;
}

//=============================================================================
// definitions : class : cell<T[N], Align>
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
constexpr msl::cell<T[N], Align>::cell()
  noexcept
  : m_data{nullptr}
{

}

template <typename T, std::size_t N, std::size_t Align>
inline msl::cell<T[N], Align>::cell(cell&& other)
  noexcept
  : m_data{other.m_data}
{
  other.m_data = nullptr;
}

template <typename T, std::size_t N, std::size_t Align>
inline constexpr msl::cell<T[N], Align>::cell(not_null<uninitialized<T>*> p)
  noexcept
  : m_data{p.get()}
{

}

//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
inline msl::cell<T[N], Align>& msl::cell<T[N], Align>::operator=(cell&& other)
  noexcept
{
  m_data = other.m_data;
  other.m_data = nullptr;

  return (*this);
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
constexpr msl::uninitialized<T>* msl::cell<T[N], Align>::data()
  const noexcept
{
  return intrinsics::assume_aligned<Align>(m_data);
}

template <typename T, std::size_t N, std::size_t Align>
constexpr typename msl::cell<T[N], Align>::size_type msl::cell<T[N], Align>::size()
  const noexcept
{
  return length().count() * size_of<T>();
}

template <typename T, std::size_t N, std::size_t Align>
constexpr typename msl::cell<T[N], Align>::length_type msl::cell<T[N], Align>::length()
  const noexcept
{
  return length_type{N};
}

template <typename T, std::size_t N, std::size_t Align>
inline constexpr msl::cell<T[N], Align>::operator bool()
  const noexcept
{
  return m_data != nullptr;
}

//=============================================================================
// non-member functions : class : cell
//=============================================================================

//-----------------------------------------------------------------------------
// Equality Operators
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
inline constexpr bool
  msl::operator==(const cell<T, Align>& lhs, const cell<T, Align>& rhs)
  noexcept
{
  return lhs.data() == rhs.data();
}

template <typename T, std::size_t Align>
inline constexpr bool
  msl::operator!=(const cell<T, Align>& lhs, const cell<T, Align>& rhs)
  noexcept
{
  return !(lhs == rhs);
}

template <typename T, std::size_t Align>
inline constexpr bool
  msl::operator==(const cell<T[], Align>& lhs, const cell<T[], Align>& rhs)
  noexcept
{
  return lhs.data() == rhs.data() && lhs.length() == rhs.length();
}

template <typename T, std::size_t Align>
inline constexpr bool
  msl::operator!=(const cell<T[], Align>& lhs, const cell<T[], Align>& rhs)
  noexcept
{
  return !(lhs == rhs);
}

template <typename T, std::size_t N, std::size_t Align>
inline constexpr bool
  msl::operator==(const cell<T[N], Align>& lhs, const cell<T[N], Align>& rhs)
  noexcept
{
  return lhs.data() == rhs.data();
}

template <typename T, std::size_t N, std::size_t Align>
inline constexpr bool
  msl::operator!=(const cell<T[N], Align>& lhs, const cell<T[N], Align>& rhs)
  noexcept
{
  return !(lhs == rhs);
}

#endif /* MSL_CELLS_CELL_HPP */
