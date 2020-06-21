///////////////////////////////////////////////////////////////////////////////
/// \file active_cell.hpp
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
#ifndef MSL_CELLS_ACTIVE_CELL_HPP
#define MSL_CELLS_ACTIVE_CELL_HPP

#include "detail/byte_or_quantity.hpp"

#include "msl/utilities/quantity.hpp"
#include "msl/utilities/digital_quantity.hpp"
#include "msl/utilities/not_null.hpp"
#include "msl/utilities/attributes.hpp"
#include "msl/utilities/intrinsics.hpp"

#include <cstddef> // std::size_t
#include <type_traits> // std::make_signed_t
#include <iterator>    // std::reverse_iterator
#include <stdexcept>   // std::out_of_range
#include <string>      // std::to_string

namespace msl {

  //===========================================================================
  // class : active_cell
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A memory cell that contains an object with an active lifetime.
  ///
  /// This type contrasts the `cell` object, which only refers to cells that
  /// do not (yet) contain an active lifetime.
  ///
  /// Like the `cell` type, this class encodes the alignment and, where
  /// possible, the size (for array types).
  ///
  /// \tparam T the type that is active within the cell
  /// \tparam Align the alignment of the cell. Must be a power of two, and
  ///         greater than or equal to alignof(T)
  /////////////////////////////////////////////////////////////////////////////
  template <typename T, std::size_t Align = alignof(T)>
  class active_cell
  {
    static_assert(
      !std::is_void_v<T>,
      "active_cell<void> is ill-formed."
    );

    static_assert(
      !std::is_reference_v<T>,
      "active_cell<T&> is ill-formed."
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

    using element_type = initialized<T>;
    using pointer      = element_type*;
    using reference    = element_type&;

    using size_type   = bytes;
    using length_type = detail::byte_or_quantity<T>;
    using index_type  = std::make_signed_t<std::size_t>;

    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<T*>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs a cell that does not reference an active cell
    constexpr active_cell() noexcept;

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    active_cell(active_cell&& other) noexcept;
    active_cell(const active_cell&) = delete;

    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    explicit constexpr active_cell(not_null<initialized<T>*> p) noexcept;

    //-------------------------------------------------------------------------

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    /// \return reference to (*this)
    active_cell& operator=(active_cell&& other) noexcept;
    active_cell& operator=(const active_cell& other) = delete;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Accesses the underlying object referenced by the cell
    ///
    /// \return the pointer from the cell
    constexpr initialized<T>* operator->() const noexcept;

    /// \brief Accesses the underlying object referenced by the cell
    ///
    /// \return the reference to the pointed-to object
    constexpr initialized<T>& operator*() const noexcept;

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
    constexpr initialized<T>* data() const noexcept;

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
    // Iterators
    //-------------------------------------------------------------------------
  public:

    constexpr iterator begin() const noexcept;
    constexpr iterator end() const noexcept;

    constexpr reverse_iterator rbegin() const noexcept;
    constexpr reverse_iterator rend() const noexcept;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    initialized<T>* m_data;
  };

  //===========================================================================
  // class : active_cell<T[], Align>
  //===========================================================================

  template <typename T, std::size_t Align>
  class active_cell<T[], Align>
  {
    static_assert(
      !std::is_void_v<T>,
      "active_cell<void> is ill-formed."
    );

    static_assert(
      !std::is_reference_v<T>,
      "active_cell<T&> is ill-formed."
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

    using element_type = initialized<T>;
    using pointer      = element_type*;
    using reference    = element_type&;

    using size_type   = bytes;
    using length_type = detail::byte_or_quantity<T>;
    using index_type  = std::make_signed_t<std::size_t>;

    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<T*>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs a cell that does not reference an active cell
    constexpr active_cell() noexcept;

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    active_cell(active_cell&& other) noexcept;
    active_cell(const active_cell&) = delete;

    /// \{
    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    /// \param length the length of the active cell
    constexpr active_cell(not_null<initialized<T>*> p, quantity<T> length) noexcept;
    constexpr active_cell(not_null<initialized<T>*> p, std::size_t length) noexcept;
    /// \}

    //-------------------------------------------------------------------------

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    /// \return reference to (*this)
    active_cell& operator=(active_cell&& other) noexcept;
    active_cell& operator=(const active_cell& other) = delete;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Accesses the underlying object from this active cell at the
    ///        specified index
    ///
    /// This does not check for valid range
    ///
    /// \param idx the index
    /// \return reference to the object at the specified index
    constexpr initialized<T>& operator[](index_type idx) const noexcept;

    /// \brief Accesses the underlying object from this active cell at the
    ///        specified index
    ///
    /// This function will throw on out-of-bounds access
    ///
    /// \throws std::out_of_range if idx > length()
    /// \param idx the index
    /// \return reference to the object at the specified index
    constexpr initialized<T>& at(index_type idx) const;

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
    constexpr initialized<T>* data() const noexcept;

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
    // Iterators
    //-------------------------------------------------------------------------
  public:

    constexpr iterator begin() const noexcept;
    constexpr iterator end() const noexcept;

    constexpr reverse_iterator rbegin() const noexcept;
    constexpr reverse_iterator rend() const noexcept;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    initialized<T>* m_data;
    quantity<T> m_length;
  };

  //===========================================================================
  // class : active_cell<T[N], Align>
  //===========================================================================


  template <typename T, std::size_t N, std::size_t Align>
  class active_cell<T[N], Align>
  {
    static_assert(
      !std::is_void_v<T>,
      "active_cell<void> is ill-formed."
    );

    static_assert(
      !std::is_reference_v<T>,
      "active_cell<T&> is ill-formed."
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

    using element_type = initialized<T>;
    using pointer      = element_type*;
    using reference    = element_type&;

    using size_type   = bytes;
    using length_type = detail::byte_or_quantity<T>;
    using index_type  = std::make_signed_t<std::size_t>;

    using iterator = T*;
    using reverse_iterator = std::reverse_iterator<T*>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs a cell that does not reference an active cell
    constexpr active_cell() noexcept;

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    active_cell(active_cell&& other) noexcept;
    active_cell(const active_cell&) = delete;

    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    explicit constexpr active_cell(not_null<initialized<T>*> p) noexcept;

    //-------------------------------------------------------------------------

    /// \brief Moves the cell from \p other
    ///
    /// \post other.data() == nullptr
    ///
    /// \param other the other cell to move
    /// \return reference to (*this)
    active_cell& operator=(active_cell&& other) noexcept;
    active_cell& operator=(const active_cell& other) = delete;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Accesses the underlying object from this active cell at the
    ///        specified index
    ///
    /// This does not check for valid range
    ///
    /// \param idx the index
    /// \return reference to the object at the specified index
    constexpr initialized<T>& operator[](index_type idx) const noexcept;

    /// \brief Accesses the underlying object from this active cell at the
    ///        specified index
    ///
    /// This function will throw on out-of-bounds access
    ///
    /// \throws std::out_of_range if idx > length()
    /// \param idx the index
    /// \return reference to the object at the specified index
    constexpr initialized<T>& at(index_type idx) const;

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
    constexpr initialized<T>* data() const noexcept;

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
    // Iterators
    //-------------------------------------------------------------------------
  public:

    constexpr iterator begin() const noexcept;
    constexpr iterator end() const noexcept;

    constexpr reverse_iterator rbegin() const noexcept;
    constexpr reverse_iterator rend() const noexcept;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    initialized<T>* m_data;
  };

  //===========================================================================
  // non-member function : class : active_cell
  //===========================================================================

  //---------------------------------------------------------------------------
  // Equality Operators
  //---------------------------------------------------------------------------

  template <typename T, std::size_t Align>
  constexpr bool operator==(const active_cell<T, Align>& lhs, const active_cell<T, Align>& rhs) noexcept;
  template <typename T, std::size_t Align>
  constexpr bool operator!=(const active_cell<T, Align>& lhs, const active_cell<T, Align>& rhs) noexcept;

  template <typename T, std::size_t Align>
  constexpr bool operator==(const active_cell<T[], Align>& lhs, const active_cell<T[], Align>& rhs) noexcept;
  template <typename T, std::size_t Align>
  constexpr bool operator!=(const active_cell<T[], Align>& lhs, const active_cell<T[], Align>& rhs) noexcept;

  template <typename T, std::size_t N, std::size_t Align>
  constexpr bool operator==(const active_cell<T[N], Align>& lhs, const active_cell<T[N], Align>& rhs) noexcept;
  template <typename T, std::size_t N, std::size_t Align>
  constexpr bool operator!=(const active_cell<T[N], Align>& lhs, const active_cell<T[N], Align>& rhs) noexcept;

} // namespace msl

//=============================================================================
// class : active_cell
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::active_cell<T, Align>::active_cell()
  noexcept
  : m_data{nullptr}
{

}

template <typename T, std::size_t Align>
inline msl::active_cell<T, Align>::active_cell(active_cell&& other)
  noexcept
  : m_data{other.m_data}
{
  other.m_data = nullptr;
}

template <typename T, std::size_t Align>
constexpr msl::active_cell<T, Align>::active_cell(not_null<initialized<T>*> p)
  noexcept
  : m_data{p.as_nullable()}
{

}

//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
inline msl::active_cell<T, Align>&
  msl::active_cell<T, Align>::operator=(active_cell&& other)
  noexcept
{
  m_data = other.m_data;
  other.m_data = nullptr;

  return (*this);
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::initialized<T>* msl::active_cell<T, Align>::operator->()
  const noexcept
{
  MSL_ASSERT(m_data != nullptr);
  return data();
}

template <typename T, std::size_t Align>
constexpr msl::initialized<T>& msl::active_cell<T, Align>::operator*()
  const noexcept
{
  MSL_ASSERT(m_data != nullptr);
  return *data();
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::initialized<T>* msl::active_cell<T, Align>::data()
  const noexcept
{
  return intrinsics::assume_aligned<Align>(m_data);
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T, Align>::size_type
  msl::active_cell<T, Align>::size()
  const noexcept
{
  return length().count() * size_of<T>();
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T, Align>::length_type
  msl::active_cell<T, Align>::length()
  const noexcept
{
  // return either '0' or '1' depending on whether we are null
  return length_type{1};
}

template <typename T, std::size_t Align>
constexpr msl::active_cell<T, Align>::operator bool()
  const noexcept
{
  return m_data != nullptr;
}

//-----------------------------------------------------------------------------
// Iterators
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T, Align>::iterator
  msl::active_cell<T, Align>::begin()
  const noexcept
{
  return m_data;
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T, Align>::iterator
  msl::active_cell<T, Align>::end()
  const noexcept
{
  return m_data + 1;
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T, Align>::reverse_iterator
  msl::active_cell<T, Align>::rbegin()
  const noexcept
{
  return std::make_reverse_iterator(end());
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T, Align>::reverse_iterator
  msl::active_cell<T, Align>::rend()
  const noexcept
{
  return std::make_reverse_iterator(begin());
}


//=============================================================================
// class : active_cell<T[], Align>
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
inline constexpr msl::active_cell<T[], Align>::active_cell()
  noexcept
  : m_data{nullptr}
{

}

template <typename T, std::size_t Align>
inline msl::active_cell<T[], Align>::active_cell(active_cell<T[], Align>&& other)
  noexcept
  : m_data{other.m_data},
    m_length{other.m_length}
{
  other.m_data = nullptr;
}

template <typename T, std::size_t Align>
constexpr msl::active_cell<T[], Align>::active_cell(not_null<initialized<T>*> p,
                                                    quantity<T> length)
  noexcept
  : m_data{p.as_nullable()},
    m_length{length}
{

}

template <typename T, std::size_t Align>
constexpr msl::active_cell<T[], Align>::active_cell(not_null<initialized<T>*> p,
                                                    std::size_t length)
  noexcept
  : m_data{p.as_nullable()},
    m_length(length)
{

}

//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
inline msl::active_cell<T[], Align>&
  msl::active_cell<T[], Align>::operator=(active_cell<T[], Align>&& other)
  noexcept
{
  m_data = other.m_data;
  m_length = other.m_length;

  other.m_data = nullptr;
  return (*this);
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::initialized<T>&
msl::active_cell<T[], Align>::operator[](index_type idx)
  const noexcept
{
  MSL_ASSERT(idx >= 0 && idx < m_length);
  MSL_ASSERT(m_data != nullptr);

  return data()[idx];
}

template <typename T, std::size_t Align>
constexpr msl::initialized<T>&
  msl::active_cell<T[], Align>::at(index_type idx)
  const
{
  MSL_ASSERT(m_data != nullptr);

  if (idx < 0 || idx >= m_length) {
    throw std::out_of_range{
      "active_cell::at: index out of range, n = "  + std::to_string(idx)
    };
  }
  return data()[idx];
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr msl::initialized<T>* msl::active_cell<T[], Align>::data()
  const noexcept
{
  return intrinsics::assume_aligned<Align>(m_data);
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T[], Align>::size_type
  msl::active_cell<T[], Align>::size()
  const noexcept
{
  return length().count() * size_of<T>();
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T[], Align>::length_type
  msl::active_cell<T[], Align>::length()
  const noexcept
{
  return m_length;
}

template <typename T, std::size_t Align>
constexpr msl::active_cell<T[], Align>::operator bool()
  const noexcept
{
  return m_data != nullptr;
}

//-----------------------------------------------------------------------------
// Iterators
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T[], Align>::iterator
  msl::active_cell<T[], Align>::begin()
  const noexcept
{
  return m_data;
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T[], Align>::iterator
  msl::active_cell<T[], Align>::end()
  const noexcept
{
  return m_data + m_length.count();
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T[], Align>::reverse_iterator
  msl::active_cell<T[], Align>::rbegin()
  const noexcept
{
  return std::make_reverse_iterator(end());
}

template <typename T, std::size_t Align>
constexpr typename msl::active_cell<T[], Align>::reverse_iterator
  msl::active_cell<T[], Align>::rend()
  const noexcept
{
  return std::make_reverse_iterator(begin());
}

//=============================================================================
// class : active_cell<T[N], Align.
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
inline constexpr msl::active_cell<T[N], Align>::active_cell()
  noexcept
  : m_data{nullptr}
{

}

template <typename T, std::size_t N, std::size_t Align>
inline msl::active_cell<T[N], Align>::active_cell(active_cell<T[N], Align>&& other)
  noexcept
  : m_data{other.m_data}
{
  other.m_data = nullptr;
}

template <typename T, std::size_t N, std::size_t Align>
constexpr msl::active_cell<T[N], Align>::active_cell(not_null<initialized<T>*> p)
  noexcept
  : m_data{p.as_nullable()}
{

}

//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
inline msl::active_cell<T[N], Align>&
  msl::active_cell<T[N], Align>::operator=(active_cell<T[N], Align>&& other)
  noexcept
{
  m_data = other.m_data;

  other.m_data = nullptr;
  return (*this);
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
constexpr msl::initialized<T>&
msl::active_cell<T[N], Align>::operator[](index_type idx)
  const noexcept
{
  MSL_ASSERT(idx >= 0 && idx < N);
  MSL_ASSERT(m_data != nullptr);

  return data()[idx];
}

template <typename T, std::size_t N, std::size_t Align>
constexpr msl::initialized<T>&
  msl::active_cell<T[N], Align>::at(index_type idx)
  const
{
  MSL_ASSERT(m_data != nullptr);

  if (idx < 0 || idx >= N) {
    throw std::out_of_range{
      "active_cell::at: index out of range, n = "  + std::to_string(idx)
    };
  }
  return data()[idx];
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
constexpr msl::initialized<T>* msl::active_cell<T[N], Align>::data()
  const noexcept
{
  return intrinsics::assume_aligned<Align>(m_data);
}

template <typename T, std::size_t N, std::size_t Align>
constexpr typename msl::active_cell<T[N], Align>::size_type
  msl::active_cell<T[N], Align>::size()
  const noexcept
{
  return length().count() * size_of<T>();
}

template <typename T, std::size_t N, std::size_t Align>
constexpr typename msl::active_cell<T[N], Align>::length_type
  msl::active_cell<T[N], Align>::length()
  const noexcept
{
  return length_type{N};
}

template <typename T, std::size_t N, std::size_t Align>
constexpr msl::active_cell<T[N], Align>::operator bool()
  const noexcept
{
  return m_data != nullptr;
}

//-----------------------------------------------------------------------------
// Iterators
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
constexpr typename msl::active_cell<T[N], Align>::iterator
  msl::active_cell<T[N], Align>::begin()
  const noexcept
{
  return m_data;
}

template <typename T, std::size_t N, std::size_t Align>
constexpr typename msl::active_cell<T[N], Align>::iterator
  msl::active_cell<T[N], Align>::end()
  const noexcept
{
  return m_data + N;
}

template <typename T, std::size_t N, std::size_t Align>
constexpr typename msl::active_cell<T[N], Align>::reverse_iterator
  msl::active_cell<T[N], Align>::rbegin()
  const noexcept
{
  return std::make_reverse_iterator(end());
}

template <typename T, std::size_t N, std::size_t Align>
constexpr typename msl::active_cell<T[N], Align>::reverse_iterator
  msl::active_cell<T[N], Align>::rend()
  const noexcept
{
  return std::make_reverse_iterator(begin());
}

//=============================================================================
// non-member functions : class : cell
//=============================================================================

//-----------------------------------------------------------------------------
// Equality Operators
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
inline constexpr bool
  msl::operator==(const active_cell<T, Align>& lhs, const active_cell<T, Align>& rhs)
  noexcept
{
  return lhs.data() == rhs.data();
}

template <typename T, std::size_t Align>
inline constexpr bool
  msl::operator!=(const active_cell<T, Align>& lhs, const active_cell<T, Align>& rhs)
  noexcept
{
  return !(lhs == rhs);
}

template <typename T, std::size_t Align>
inline constexpr bool
  msl::operator==(const active_cell<T[], Align>& lhs, const active_cell<T[], Align>& rhs)
  noexcept
{
  return lhs.data() == rhs.data() && lhs.length() == rhs.length();
}

template <typename T, std::size_t Align>
inline constexpr bool
  msl::operator!=(const active_cell<T[], Align>& lhs, const active_cell<T[], Align>& rhs)
  noexcept
{
  return !(lhs == rhs);
}

template <typename T, std::size_t N, std::size_t Align>
inline constexpr bool
  msl::operator==(const active_cell<T[N], Align>& lhs, const active_cell<T[N], Align>& rhs)
  noexcept
{
  return lhs.data() == rhs.data();
}

template <typename T, std::size_t N, std::size_t Align>
inline constexpr bool
  msl::operator!=(const active_cell<T[N], Align>& lhs, const active_cell<T[N], Align>& rhs)
  noexcept
{
  return !(lhs == rhs);
}

#endif /* MSL_CELLS_ACTIVE_CELL_HPP */
