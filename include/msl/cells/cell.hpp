///////////////////////////////////////////////////////////////////////////////
/// \file cell.hpp
///
/// \todo(Bitwize): Document this
///////////////////////////////////////////////////////////////////////////////

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
#ifndef MSL_CELLS_CELL_HPP
#define MSL_CELLS_CELL_HPP

#include "msl/pointers/not_null.hpp"
#include "msl/pointers/traversal_utilities.hpp"
#include "msl/utilities/assert.hpp"
#include "msl/utilities/intrinsics.hpp"
#include "msl/quantities/digital_quantity.hpp"
#include "msl/quantities/quantity.hpp"

#include <cstddef>     // std::size_t
#include <type_traits> // std::make_signed_t, std::enable_if_t
#include <stdexcept>   // std::out_of_range
#include <concepts>    // std::convertible_to
#include <bit>         // std::has_single_bit

namespace msl::detail {

  [[noreturn]]
  auto throw_cell_out_of_range(std::size_t index, std::size_t length) -> void;

  template <typename From, typename To>
  using match_const_t = std::conditional_t<
    std::is_const_v<From>,
    std::add_const_t<To>,
    To
  >;
  template <typename From, typename To>
  using match_volatile_t = std::conditional_t<
    std::is_volatile_v<From>,
    std::add_volatile_t<To>,
    To
  >;

  template <typename From, typename To>
  using match_cv_t = match_const_t<From, match_volatile_t<From, To>>;

} // namespace msl::detail

namespace msl {

  //===========================================================================
  // class : cell
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief Represents a memory cell containing an active object which is
  ///        aligned to a specific alignment.
  ///
  /// Memory cells are effectively non-nullable fat-pointers that know their
  /// boundaries. In particular, it is aware of:
  ///
  /// * the underlying type of the pointed-to cell,
  /// * the length of the sequence (when contiguous) to avoid out-of-bounds
  ///   access, and
  /// * the alignment of the pointer
  ///
  /// If a cell does not statically encode the length (e.g. `T[]`), then the
  /// length becomes a runtime value (a true "fat pointer").
  ///
  /// This allows for cells to distinguish overload sets for both over-aligned
  /// types and normal aligned types. For example, it's possible to write code
  /// such as:
  ///
  /// ```cpp
  /// // SIMD does not require aligned-load
  /// void simd_add(const cell<float[4],64> lhs, const cell<float[4],64>& rhs);
  ///
  /// // SIMD must perform aligned-load
  /// void simd_add(cell<float[4]> lhs, cell<float[4]>& rhs);
  /// ```
  ///
  /// to provide an overload set for float that may use SIMD instructions if
  /// possible.
  ///
  /// \note Unlike pointers, "cell" objects cannot take on the form of either a
  ///       covariant or hierarchically related type. This is by-design to
  ///       prevent loss of crucial information required for allocators.
  ///
  /// \note Unlike pointers, "cell" objects cannot legally take on the value of
  ///       "null".
  ///
  /// \note "cell" objects, like pointers, do not propagate CV-qualifiers,
  ///       since the constness of the cell indicates whether the cell itself
  ///       may be reseated.
  ///
  /// \note "cell" objects can never be null. A `cell<std::byte[]>` is the
  ///       closest semantic-equivalent to a `void*`.
  ///
  /// \tparam T The underlying type of the cell
  /// \tparam Align The alignment of the cell (default is alignof(T))
  /////////////////////////////////////////////////////////////////////////////
  template <typename T, std::size_t Align = alignof(T)>
  class cell
  {
    static_assert(
      !std::is_void_v<T>,
      "cell<void> is ill-formed. Use cell<byte[]> to refer to byte sequences."
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
      std::has_single_bit(Align),
      "Alignment must be a power-of-two."
    );

    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using element_type = T;
    using pointer      = element_type*;
    using reference    = element_type&;
    using size_type    = uquantity<std::remove_cv_t<T>>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    cell() = delete;

    /// \brief Copies the cell from \p other
    ///
    /// \param other the other cell to copy
    constexpr cell(const cell& other) noexcept = default;

    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    template <typename U>
    constexpr explicit cell(not_null<U*> p) noexcept
      requires(std::convertible_to<U(*)[], T(*)[]>);

    /// \brief Constructs a cell from a cell with a lower alignment
    ///
    /// This overload is explicit if `Align` is less than the known alignment of
    /// \p other, as this forces the user to acknowledge the new alignment
    /// specification.
    ///
    /// \pre The cell `other.data()` is aligned to an `Align` boundary
    /// \tparam UAlign the alignment of the cell being converted
    /// \param other the other cell to copy
    template <typename U, std::size_t UAlign>
    explicit((UAlign < Align))
    constexpr cell(const cell<U,UAlign>& other) noexcept
      requires(std::convertible_to<U(*)[], T(*)[]>);

    //-------------------------------------------------------------------------

    /// \brief Copies the cell from \p other
    ///
    /// \param other the other cell to copy
    /// \return reference to (*this)
    auto operator=(const cell& other) -> cell& = default;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the pointer from this cell
    ///
    /// \return the underlying pointer
    [[nodiscard]]
    constexpr auto data() const noexcept -> not_null<T*>;

    /// \brief Gets the size of this memory cell, in bytes
    ///
    /// \return the size of this cell in bytes
    [[nodiscard]]
    constexpr auto size_in_bytes() const noexcept -> bytes;

    /// \brief Gets the number of elements addressable in the contiguous
    ///        sequence denoted by this cell
    ///
    /// If the underlying type of the cell is bytes, the length is in the
    /// `bytes` type; otherwise it is a `capacity`
    ///
    /// \note A `cell<T>` always has a length of `1`, since it is not a
    ///       contiguous sequence
    ///
    /// \return the number of addressable elements
    [[nodiscard]]
    constexpr auto size() const noexcept -> size_type;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    [[nodiscard]]
    constexpr auto operator->() const noexcept -> not_null<T*>;
    [[nodiscard]]
    constexpr auto operator*() const noexcept -> T&;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    not_null<T*> m_data;
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
      std::has_single_bit(Align),
      "Alignment must be a power-of-two."
    );

    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using element_type = T;
    using pointer      = element_type*;
    using reference    = element_type&;
    using size_type    = uquantity<std::remove_cv_t<T>>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    cell() = delete;

    /// \brief Copies the cell from \p other
    ///
    /// \param other the other cell to copy
    constexpr cell(const cell& other) noexcept = default;

    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    template <typename U>
    constexpr cell(not_null<U*> p, size_type length) noexcept
      requires(std::convertible_to<U(*)[],T(*)[]>);

    /// \brief Constructs a cell from a cell with a lower alignment
    ///
    /// This overload is explicit if `Align` is less than the known alignment of
    /// \p other, as this forces the user to acknowledge the new alignment
    /// specification.
    ///
    /// \pre The cell `other.data()` is aligned to an `Align` boundary
    /// \tparam UAlign the alignment of the cell being converted
    /// \param other the other cell to copy
    template <typename U, std::size_t UAlign>
    explicit((UAlign < Align))
    constexpr cell(const cell<U[],UAlign>& other) noexcept
      requires(std::convertible_to<U(*)[], T(*)[]>);

    /// \brief Constructs a cell from a cell with a lower alignment
    ///
    /// This overload is explicit if `Align` is less than the known alignment of
    /// \p other, as this forces the user to acknowledge the new alignment
    /// specification.
    ///
    /// \pre The cell `other.data()` is aligned to an `Align` boundary
    /// \tparam UAlign the alignment of the cell being converted
    /// \param other the other cell to copy
    template <typename U, std::size_t N, std::size_t UAlign>
    explicit((UAlign < Align))
    constexpr cell(const cell<U[N],UAlign>& other) noexcept
      requires(std::convertible_to<U(*)[], T(*)[]>);

    //-------------------------------------------------------------------------

    /// \brief Copies the cell from \p other
    ///
    /// \param other the other cell to copy
    /// \return reference to (*this)
    auto operator=(const cell& other) noexcept -> cell& = default;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the pointer from this cell
    ///
    /// \return the underlying pointer
    [[nodiscard]]
    constexpr auto data() const noexcept -> not_null<T*>;

    /// \brief Gets the size of this memory cell, in bytes
    ///
    /// \return the size of this cell in bytes
    [[nodiscard]]
    constexpr auto size_in_bytes() const noexcept -> bytes;

    /// \brief Gets the number of elements addressable in the contiguous
    ///        sequence denoted by this cell
    ///
    /// If the underlying type of the cell is bytes, the length is in the
    /// `bytes` type; otherwise it is a `capacity`
    ///
    /// \return the number of addressable elements
    [[nodiscard]]
    constexpr auto size() const noexcept -> size_type;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Accesses the element at index \p n
    ///
    /// \pre \p n must be less than `size()`
    /// \param n the index
    /// \return a reference to the nth element
    [[nodiscard]]
    constexpr auto operator[](std::size_t n) const noexcept -> T&;

    /// \brief Accesses the element at index \p n
    ///
    /// \throw std::out_of_range if \p n is greater than `size()`
    ///
    /// \param n the index
    /// \return a reference to the nth element
    [[nodiscard]]
    constexpr auto at(std::size_t n) const -> T&;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    not_null<T*> m_data;
    size_type m_length;
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
      std::has_single_bit(Align),
      "Alignment must be a power-of-two."
    );

    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using element_type = T;
    using pointer      = element_type*;
    using reference    = element_type&;
    using size_type    = uquantity<std::remove_cv_t<T>>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    cell() = delete;

    /// \brief Copies the cell from \p other
    ///
    /// \param other the other cell to copy
    constexpr cell(const cell& other) noexcept = default;

    /// \brief Constructs a cell from the underlying pointer
    ///
    /// \param p the pointer that this cell references
    template <typename U>
    constexpr explicit cell(not_null<U*> p) noexcept
      requires(std::convertible_to<U(*)[],T(*)[]>);

    /// \brief Constructs a cell from a cell with a lower alignment
    ///
    /// This overload is explicit if `Align` is less than the known alignment of
    /// \p other, as this forces the user to acknowledge the new alignment
    /// specification.
    ///
    /// \pre The cell `other.data()` is aligned to an `Align` boundary
    /// \tparam UAlign the alignment of the cell being converted
    /// \param other the other cell to copy
    template <typename U, std::size_t UAlign>
    explicit((UAlign < Align))
    constexpr cell(const cell<U[N],UAlign>& other) noexcept
      requires(std::convertible_to<U(*)[], T(*)[]>);

    /// \brief Constructs a cell from a cell without an explicit size
    ///
    /// This overload is always explicit, since the caller must be absolutely
    /// sure that a `cell<T[N]>` is converted from `cell<T[]>`.
    ///
    /// \pre `other.data()` is aligned to an `Align` boundary
    /// \pre `other.size_in_bytes() == N`
    /// \tparam UAlign the alignment of the cell being converted
    /// \param other the other cell to copy
    template <typename U, std::size_t UAlign>
    explicit constexpr cell(const cell<U[],UAlign>& other) noexcept
      requires(std::convertible_to<U(*)[], T(*)[]>);

    //-------------------------------------------------------------------------

    /// \brief Copies the cell from \p other
    ///
    /// \param other the other cell to copy
    /// \return reference to (*this)
    auto operator=(const cell& other) noexcept -> cell& = default;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the pointer from this cell
    ///
    /// \return the underlying pointer
    [[nodiscard]]
    constexpr auto data() const noexcept -> not_null<T*>;

    /// \brief Gets the size of this memory cell, in bytes
    ///
    /// \return the size of this cell in bytes
    [[nodiscard]]
    constexpr auto size_in_bytes() const noexcept -> bytes;

    /// \brief Gets the number of elements addressable in the contiguous
    ///        sequence denoted by this cell
    ///
    /// If the underlying type of the cell is bytes, the length is in the
    /// `bytes` type; otherwise it is a `capacity`
    ///
    /// \return the number of addressable elements
    [[nodiscard]]
    constexpr auto size() const noexcept -> size_type;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Accesses the element at index \p n
    ///
    /// \pre \p n must be less than `size()`
    /// \param n the index
    /// \return a reference to the nth element
    [[nodiscard]]
    constexpr auto operator[](std::size_t n) const noexcept -> T&;

    /// \brief Accesses the element at index \p n
    ///
    /// \throw std::out_of_range if \p n is greater than `size()`
    ///
    /// \param n the index
    /// \return a reference to the nth element
    [[nodiscard]]
    constexpr auto at(std::size_t n) const -> T&;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    not_null<T*> m_data;
  };

  //===========================================================================
  // non-member functions : class : cell
  //===========================================================================

  //---------------------------------------------------------------------------
  // Equality Operators
  //---------------------------------------------------------------------------

  template <typename T, typename U, std::size_t Align>
  constexpr auto operator==(const cell<T, Align>& lhs, const cell<U, Align>& rhs)
    noexcept -> bool
    requires(std::is_scalar_v<T> && std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>);

  //---------------------------------------------------------------------------

  // Allow comparisons between both static and dynamically lengthed cell objects
  template <typename T, typename U, std::size_t Align>
  constexpr auto operator==(const cell<T[], Align>& lhs, const cell<U[], Align>& rhs)
    noexcept -> bool
    requires(std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>);
  template <typename T, typename U, std::size_t N, std::size_t Align>
  constexpr auto operator==(const cell<T[N], Align>& lhs, const cell<U[], Align>& rhs)
    noexcept -> bool
    requires(std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>);
  template <typename T, typename U, std::size_t N, std::size_t Align>
  constexpr auto operator==(const cell<T[], Align>& lhs, const cell<U[N], Align>& rhs)
    noexcept -> bool
    requires(std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>);

  //---------------------------------------------------------------------------

  template <typename T, typename U, std::size_t N, std::size_t M, std::size_t Align>
  constexpr auto operator==(const cell<T[N], Align>& lhs, const cell<U[M], Align>& rhs)
    noexcept -> bool
    requires(std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>);

  //---------------------------------------------------------------------------
  // Utility Functions
  //---------------------------------------------------------------------------

  /// \brief Gets a pointer to the underlying data referenced by the cell
  ///
  /// \param c the cell
  /// \return a pointer to the data
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto data(const cell<T, Align>& c)
    noexcept -> typename cell<T, Align>::element_type*;

  //// \brief Gets the number of elements referenced by this cell
  ///
  /// \param c the cell
  /// \return a number of elements that occupies this cell
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto size(const cell<T, Align>& c)
    noexcept -> typename cell<T, Align>::size_type;

  /// \brief Gets the number of bytes that this cell occupies
  ///
  /// \param c the cell
  /// \return a number of bytes that this cell occupies
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto size_in_bytes(const cell<T, Align>& c)
    noexcept -> bytes;

  //---------------------------------------------------------------------------
  // Conversions
  //---------------------------------------------------------------------------

  /// \{
  /// \brief Converts the specified cell into a cell that ranges over the
  ///        mutable byte sequence
  ///
  /// \param c the cell to convert to a byte cell
  /// \return the byte cell
  template <typename T, std::size_t Align>
  constexpr auto as_mutable_bytes(const cell<T, Align>& c)
    noexcept -> cell<std::byte[sizeof(T)], Align>
    requires(!std::is_const_v<T> && !std::is_volatile_v<T>);
  template <typename T, std::size_t Align>
  constexpr auto as_mutable_bytes(const cell<T[], Align>& c)
    noexcept -> cell<std::byte[], Align>
    requires(!std::is_const_v<T> && !std::is_volatile_v<T>);
  template <typename T, std::size_t N, std::size_t Align>
  constexpr auto as_mutable_bytes(const cell<T[N], Align>& c)
    noexcept -> cell<std::byte[sizeof(T) * N], Align>
    requires(!std::is_const_v<T> && !std::is_volatile_v<T>);
  // \}

  //---------------------------------------------------------------------------

  /// \{
  /// \brief Converts the specified cell into a cell that ranges over the const
  ///        byte sequence
  ///
  /// \param c the cell to convert to a byte cell
  /// \return the byte cell
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto as_const_bytes(const cell<T, Align>& c)
    noexcept -> cell<const std::byte[sizeof(T)], Align>
    requires(!std::is_volatile_v<T>);
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto as_const_bytes(const cell<T[], Align>& c)
    noexcept -> cell<const std::byte[], Align>
    requires(!std::is_volatile_v<T>);
  template <typename T, std::size_t N, std::size_t Align>
  [[nodiscard]]
  constexpr auto as_const_bytes(const cell<T[N], Align>& c)
    noexcept -> cell<const std::byte[sizeof(T) * N], Align>
    requires(!std::is_volatile_v<T>);
  // \}

  // is_volatile_bytes not provided, since it's seldomly beneficial to view
  // a *non-volatile* pointer as a volatile one (at least, for any *legal*
  // reason).

  //---------------------------------------------------------------------------

  /// \{
  /// \brief Converts the specified cell into a cell that ranges over the byte
  ///        sequence
  ///
  /// This will propagate the CV qualifiers of the T type to the underlying byte
  /// sequence
  ///
  /// \param c the cell to convert to a byte cell
  /// \return the byte cell
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto as_bytes(const cell<T, Align>& c)
    noexcept -> cell<detail::match_cv_t<T, std::byte>[sizeof(T)], Align>;
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto as_bytes(const cell<T[], Align>& c)
    noexcept -> cell<detail::match_cv_t<T, std::byte>[], Align>;
  template <typename T, std::size_t N, std::size_t Align>
  [[nodiscard]]
  constexpr auto as_bytes(const cell<T[N], Align>& c)
    noexcept -> cell<detail::match_cv_t<T, std::byte>[sizeof(T) * N], Align>;
  // \}

  //---------------------------------------------------------------------------
  // Iterators
  //---------------------------------------------------------------------------

  /// \brief Gets an iterator to the beginning of the cell's range
  ///
  /// This allows for any cell of data, whether scalar or array, to be iterated
  /// over based on the cell's known reachability.
  ///
  /// \param c the cell to get the iterator for
  /// \return an iterator to the beginning of the cell's range
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto begin(const cell<T, Align>& c)
    noexcept -> typename cell<T, Align>::element_type*;

  /// \brief Gets an iterator to the end of the cell's range
  ///
  /// \param c the cell to get the iterator for
  /// \return an iterator to the end of the cell's range
  template <typename T, std::size_t Align>
  [[nodiscard]]
  constexpr auto end(const cell<T, Align>& c)
    noexcept -> typename cell<T, Align>::element_type*;

} // namespace msl

//=============================================================================
// definitions : class : cell
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
template <typename U>
MSL_FORCE_INLINE constexpr
msl::cell<T, Align>::cell(not_null<U*> p)
  noexcept requires(std::convertible_to<U(*)[],T(*)[]>)
  : m_data{p}
{

}

template <typename T, std::size_t Align>
template <typename U, std::size_t UAlign>
MSL_FORCE_INLINE constexpr
msl::cell<T,Align>::cell(const cell<U,UAlign>& other)
  noexcept requires(std::convertible_to<U(*)[],T(*)[]>)
  : m_data{other.data()}
{

}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T, Align>::data()
  const noexcept -> not_null<T*>
{
  return assume_aligned<Align>(m_data);
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T, Align>::size_in_bytes()
  const noexcept -> bytes
{
  return size_of<T>();
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T, Align>::size()
  const noexcept -> size_type
{
  return size_type{1};
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T, Align>::operator->()
  const noexcept -> not_null<T*>
{
  return m_data;
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T, Align>::operator*()
  const noexcept -> T&
{
  return m_data;
}

//=============================================================================
// definitions : class : cell<T[], Align>
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
template <typename U>
MSL_FORCE_INLINE constexpr
msl::cell<T[], Align>::cell(not_null<U*> p, size_type length)
  noexcept requires(std::convertible_to<U(*)[],T(*)[]>)
  : m_data{p},
    m_length{length}
{

}

template <typename T, std::size_t Align>
template <typename U, std::size_t UAlign>
MSL_FORCE_INLINE constexpr
msl::cell<T[], Align>::cell(const cell<U[],UAlign>& other)
  noexcept requires(std::convertible_to<U(*)[], T(*)[]>)
  : m_data{other.data()},
    m_length{other.size()}
{

}

template <typename T, std::size_t Align>
template <typename U, std::size_t N, std::size_t UAlign>
MSL_FORCE_INLINE constexpr
msl::cell<T[], Align>::cell(const cell<U[N],UAlign>& other)
  noexcept requires(std::convertible_to<U(*)[], T(*)[]>)
  : m_data{other.data()},
    m_length{N}
{

}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T[], Align>::data()
  const noexcept -> not_null<T*>
{
  return assume_aligned<Align>(m_data);
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T[], Align>::size_in_bytes()
  const noexcept -> bytes
{
  return size().count() * size_of<T>();
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T[], Align>::size()
  const noexcept -> size_type
{
  return m_length;
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T[], Align>::operator[](std::size_t n)
  const noexcept -> T&
{
  MSL_ASSERT(n < size(), "n must not exceed the length");

  return traversal_utilities::access_at_offset(data(), n);
}

template <typename T, std::size_t Align>
inline constexpr
auto msl::cell<T[], Align>::at(std::size_t n)
  const -> T&
{
  if (n < size()) MSL_UNLIKELY {
    detail::throw_cell_out_of_range(n, size().count());
  }

  return traversal_utilities::access_at_offset(data(), n);
}

//=============================================================================
// definitions : class : cell<T[N], Align>
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
template <typename U>
MSL_FORCE_INLINE constexpr
msl::cell<T[N], Align>::cell(not_null<U*> p)
  noexcept requires(std::convertible_to<U(*)[],T(*)[]>)
  : m_data{p}
{

}

template <typename T, std::size_t N, std::size_t Align>
template <typename U, std::size_t UAlign>
MSL_FORCE_INLINE constexpr
msl::cell<T[N], Align>::cell(const cell<U[N],UAlign>& other)
  noexcept requires(std::convertible_to<U(*)[], T(*)[]>)
  : m_data{other.data()}
{

}

template <typename T, std::size_t N, std::size_t Align>
template <typename U, std::size_t UAlign>
MSL_FORCE_INLINE constexpr
msl::cell<T[N], Align>::cell(const cell<U[],UAlign>& other)
  noexcept requires(std::convertible_to<U(*)[], T(*)[]>)
  : m_data{other.data()}
{
  MSL_ASSERT(other.size() == N, "Length of 'other' must be N in order to convert");
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T[N], Align>::data()
  const noexcept -> not_null<T*>
{
  return assume_aligned<Align>(m_data);
}

template <typename T, std::size_t N, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T[N], Align>::size_in_bytes()
  const noexcept -> bytes
{
  return N * size_of<T>();
}

template <typename T, std::size_t N, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T[N], Align>::size()
  const noexcept -> size_type
{
  return size_type{N};
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

template <typename T, std::size_t N, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::cell<T[N], Align>::operator[](std::size_t n)
  const noexcept -> T&
{
  MSL_ASSERT(n < size(), "n must not exceed the length");

  return traversal_utilities::access_at_offset(data(), n);
}

template <typename T, std::size_t N, std::size_t Align>
inline constexpr
auto msl::cell<T[N], Align>::at(std::size_t n)
  const -> T&
{
  if (n < size()) MSL_UNLIKELY {
    detail::throw_cell_out_of_range(n, N);
  }

  return traversal_utilities::access_at_offset(data(), n);
}

//=============================================================================
// non-member functions : class : cell
//=============================================================================

//-----------------------------------------------------------------------------
// Equality Operators
//-----------------------------------------------------------------------------

template <typename T, typename U, std::size_t Align>
inline constexpr
auto msl::operator==(const cell<T, Align>& lhs, const cell<U, Align>& rhs)
  noexcept -> bool
  requires(std::is_scalar_v<T> && std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>)
{
  return lhs.data() == rhs.data();
}

//-----------------------------------------------------------------------------

template <typename T, typename U, std::size_t Align>
inline constexpr
auto msl::operator==(const cell<T[], Align>& lhs, const cell<U[], Align>& rhs)
  noexcept -> bool
  requires(std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>)
{
  return lhs.data() == rhs.data() && lhs.size() == rhs.size();
}

template <typename T, typename U, std::size_t N, std::size_t Align>
inline constexpr
auto msl::operator==(const cell<T[], Align>& lhs, const cell<U[N], Align>& rhs)
  noexcept -> bool
  requires(std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>)
{
  return lhs.data() == rhs.data() && lhs.size() == rhs.size();
}

template <typename T, typename U, std::size_t N, std::size_t Align>
inline constexpr
auto msl::operator==(const cell<T[N], Align>& lhs, const cell<U[], Align>& rhs)
  noexcept -> bool
  requires(std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>)
{
  return lhs.data() == rhs.data() && lhs.size() == rhs.size();
}

//-----------------------------------------------------------------------------

template <typename T, typename U, std::size_t N, std::size_t M, std::size_t Align>
inline constexpr
auto msl::operator==(const cell<T[N], Align>& lhs, const cell<U[M], Align>& rhs)
  noexcept -> bool
  requires(std::same_as<std::remove_cv_t<T>,std::remove_cv_t<U>>)
{
  if constexpr (N == M) {
    return lhs.data() == rhs.data();
  } else {
    // Trivially false; two cells can't be the same if they point to different
    // sizes
    intrinsics::suppress_unused(lhs, rhs);
    return false;
  }
}

//-----------------------------------------------------------------------------
// Utility Functions
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::data(const cell<T, Align>& c)
  noexcept -> typename cell<T, Align>::element_type*
{
  return c.data().as_nullable();
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::size(const cell<T, Align>& c)
  noexcept -> typename cell<T, Align>::size_type
{
  return c.size();
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::size_in_bytes(const cell<T, Align>& c)
  noexcept -> bytes
{
  return c.size_in_bytes();
}

//-----------------------------------------------------------------------------
// Conversions
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_mutable_bytes(const cell<T, Align>& c)
  noexcept -> cell<std::byte[sizeof(T)], Align>
  requires(!std::is_const_v<T> && !std::is_volatile_v<T>)
{
  return cell<std::byte[sizeof(T)], Align>{
    reinterpret_pointer_cast<std::byte>(c.data())
  };
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_mutable_bytes(const cell<T[], Align>& c)
  noexcept -> cell<std::byte[], Align>
  requires(!std::is_const_v<T> && !std::is_volatile_v<T>)

{
  return cell<std::byte[], Align>{
    reinterpret_pointer_cast<std::byte>(c.data()),
    c.size_in_bytes()
  };
}

template <typename T, std::size_t N, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_mutable_bytes(const cell<T[N], Align>& c)
  noexcept -> cell<std::byte[sizeof(T) * N], Align>
  requires(!std::is_const_v<T> && !std::is_volatile_v<T>)
{
  return cell<std::byte[sizeof(T) * N], Align>{
    reinterpret_pointer_cast<std::byte>(c.data())
  };
}

//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_const_bytes(const cell<T, Align>& c)
  noexcept -> cell<const std::byte[sizeof(T)], Align>
  requires(!std::is_volatile_v<T>)
{
  return cell<const std::byte[sizeof(T)], Align>{
    reinterpret_pointer_cast<const std::byte>(c.data())
  };
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_const_bytes(const cell<T[], Align>& c)
  noexcept -> cell<const std::byte[], Align>
  requires(!std::is_volatile_v<T>)

{
  return cell<const std::byte[], Align>{
    reinterpret_pointer_cast<const std::byte>(c.data()),
    c.size_in_bytes()
  };
}

template <typename T, std::size_t N, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_const_bytes(const cell<T[N], Align>& c)
  noexcept -> cell<const std::byte[sizeof(T) * N], Align>
  requires(!std::is_volatile_v<T>)
{
  return cell<const std::byte[sizeof(T) * N], Align>{
    reinterpret_pointer_cast<const std::byte>(c.data())
  };
}

//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_bytes(const cell<T, Align>& c)
  noexcept -> cell<detail::match_cv_t<T, std::byte>[sizeof(T)], Align>
{
  using type = detail::match_cv_t<T, std::byte>;

  return cell<type[sizeof(T)], Align>{
    reinterpret_pointer_cast<type>(c.data())
  };
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_bytes(const cell<T[], Align>& c)
  noexcept -> cell<detail::match_cv_t<T, std::byte>[], Align>
{
  using type = detail::match_cv_t<T, std::byte>;

  return cell<type[], Align>{
    reinterpret_pointer_cast<type>(c.data()),
    c.size_in_bytes()
  };
}

template <typename T, std::size_t N, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::as_bytes(const cell<T[N], Align>& c)
  noexcept -> cell<detail::match_cv_t<T, std::byte>[sizeof(T) * N], Align>
{
  using type = detail::match_cv_t<T, std::byte>;

  return cell<type[sizeof(T) * N], Align>{
    reinterpret_pointer_cast<type>(c.data())
  };
}

//-----------------------------------------------------------------------------
// Iterators
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::begin(const cell<T, Align>& c)
  noexcept -> typename cell<T, Align>::element_type*
{
  return c.data().as_nullable();
}

template <typename T, std::size_t Align>
MSL_FORCE_INLINE constexpr
auto msl::end(const cell<T, Align>& c)
  noexcept -> typename cell<T, Align>::element_type*
{
  return begin(c) + c.size();
}

#endif /* MSL_CELLS_CELL_HPP */
