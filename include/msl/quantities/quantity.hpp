////////////////////////////////////////////////////////////////////////////////
/// \file quantity.hpp
///
/// \brief TODO(Bitwize): Add description
////////////////////////////////////////////////////////////////////////////////

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

#ifndef MSL_QUANTITIES_QUANTITY_HPP
#define MSL_QUANTITIES_QUANTITY_HPP

#if defined(_MSC_VER)
# pragma once
#endif // defined(_MSC_VER)

#include "msl/quantities/digital_quantity.hpp"

#include <type_traits> // std::make_signed_t
#include <cstddef>     // std::size_t
#include <limits>      // std::numeric_limits
#include <compare>     // std::strong_ordering
#include <concepts>    // std::integral, std::same_as

namespace msl::detail {

    template <typename T, typename U>
    struct quantity_base : std::conditional<
      std::is_base_of_v<T,U>,
      T,
      U
    >{};

    template <typename T, typename U>
    using quantity_base_t = typename quantity_base<T,U>::type;

    template <typename T, typename U>
    concept quantity_related_to = (
      std::derived_from<T,U> || std::derived_from<U,T> || std::same_as<T,U>
    );

    template <typename T, typename U, typename Rep, typename URep>
    concept quantity_operable = (
      quantity_related_to<T,U> &&
      std::convertible_to<URep,Rep> &&
      std::convertible_to<Rep,URep> &&
      std::integral<Rep> &&
      std::integral<URep>
    );

} // namespace msl::detail

namespace msl {

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Represents a quantity of T objects
  ///
  /// A `quantity` object is effectively a replacement for the `size_t` type
  /// from C. It represents a quantity of a strongly typed object `T` in
  /// a discrete and countable unit.
  ///
  /// Objects of quantity<T> may operate with quantity<U> objects if T is a
  /// base class of U. This allows for the case of a quantity<Base> to count
  /// quantity<Derived> types, but not the inverse. More concretely, it allows
  /// for an example like:
  ///
  /// \code
  /// auto q = quantity<Fruit>{0};
  /// q += quantity<Apple>{5};
  /// \endcode
  ///
  /// But prevents, at compile time, code like:
  ///
  /// \code
  /// auto q = quantity<Apple>{0};
  /// q += quantity<Fruit>{5};
  /// \endcode
  ///
  /// since a quantity of a base class may abstractly want to count derived
  /// instances, but the inverse is not necessarily true (not all Fruit are
  /// Apples, but all Apples are Fruit).
  ///
  /// Comparisons of quantity objects may be done on any type, irrespective of
  /// relationship -- since it's reasonable to compare quantity<Apple> to a
  /// quantity<Orange>. Quantity objects may also be compared directly to
  /// the underlying representation, so that code in the form of:
  ///
  /// \code
  /// if (quantity<Apple>{5} == 5) { ... }
  /// \endcode
  ///
  /// Is a valid comparison.
  ///
  /// Since quantities of different objects are discretely identifiable in the
  /// type, this also allows quantities of different types to be differentiated
  /// on type-signatures for overload resolution.
  ///
  /// \note Although this type allows interop between base and derived types,
  ///       it is unable to operate on sibling types. For example:
  ///       \code
  ///       quantity<Apple>{5} + quantity<Orange>{2}
  ///       \endcode
  ///       is not able to deduce a result of quantity<Fruit>. This is due to the
  ///       C++ type system having no way of knowing a common base class between
  ///       the two, or which base should be applied.
  ///
  /// \note Unlike std::size_t, a quantity<T> object is by default _signed_.
  ///       This decision is intentional, since in modern machines an unsigned
  ///       value very seldomly is needed to represent the total number of
  ///       objects that are countable. The second template argument may be
  ///       explicitly specified to control this, if needed. Additionally a
  ///       `uquantity` type exists which is always unsigned.
  ///
  /// \note The template argument T does not contribute at all to the type
  ///       instantiation, other than imposing limitations on what it may be
  ///       compared with. In this respect, T is a "tag" type.
  ///
  /// \tparam T the type of object being counted. The underlying type must not
  ///         be a reference, CV-qualified, or void.
  /// \tparam Rep the representation type (signed size_t by default)
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename Rep = std::make_signed_t<std::size_t>>
  class quantity
  {
    static_assert(
      !std::is_void_v<T>,
      "quantity<void> is ill-formed."
    );

    static_assert(
      !std::is_reference_v<T>,
      "quantity<T&> is ill-formed. Use quantity<T> instead."
    );

    static_assert(
      !std::is_const_v<T>,
      "quantity<const T> is ill-formed. Quantities count discrete quantities of "
      "objects, irrespective of their cv-qualifications. Use quantity<T> instead."
    );

    static_assert(
      !std::is_volatile_v<T>,
      "quantity<volatile T> is ill-formed. Quantities count discrete quantities "
      "of objects, irrespective of their cv-qualifications. Use quantity<T> instead."
    );

    //--------------------------------------------------------------------------
    // Public Member Types
    //--------------------------------------------------------------------------
  public:

    using counted_type = T;
    using size_type    = Rep;

    //--------------------------------------------------------------------------
    // Public Static Factories
    //--------------------------------------------------------------------------
  public:

    /// \brief Gets the smallest quantity that this quantity can hold
    ///
    /// \return the smallest quantity
    static constexpr auto min() noexcept -> quantity;

    /// \brief Gets the largest quantity that this quantity can hold
    ///
    /// \return the largest quantity
    static constexpr auto max() noexcept -> quantity;

    /// \brief Gets the zero quantity
    ///
    /// \return the zero quantity
    static constexpr auto zero() noexcept -> quantity;

    //--------------------------------------------------------------------------
    // Constructors / Assignment
    //--------------------------------------------------------------------------
  public:

    /// \brief Default constructs this quantity
    constexpr quantity() noexcept = default;

    /// \brief Copies the quantity from \p other
    ///
    /// \param other the other quantity to copy
    constexpr quantity(const quantity& other) noexcept = default;

    /// \brief Constructs this quantity with the specified count
    ///
    /// \param count the count to construct this with
    constexpr quantity(const Rep& count) noexcept;

    /// \brief Explicitly converts a quantity of U type to a quantity of T
    ///
    /// This constructor only participates in overload resolution if:
    /// * `U` is derived from `T`, and
    /// * `URep` is convertible to `Rep`
    ///
    /// \param other the other quantity being converted
    template <typename U, typename URep>
    constexpr quantity(const quantity<U,URep>& other) noexcept
      requires(msl::detail::quantity_operable<T,U,URep,Rep>);

    //--------------------------------------------------------------------------

    /// \brief Copies the quantity from \p other
    ///
    /// \param other the other quantity to copy
    /// \return reference to (*this)
    auto operator=(const quantity& other) -> quantity& = default;

    /// \brief Copies and converts a quantity of U type to a quantity of T
    ///
    /// This operator only participates in overload resolution if:
    /// * `U` is derived from `T`, and
    /// * `URep` is convertible to `Rep
    ///
    /// \param other the other quantity being converted
    /// \return reference to (*this)
    template <typename U, typename URep>
    constexpr auto operator=(const quantity<U, URep>& other) noexcept -> quantity&
      requires(msl::detail::quantity_operable<T,U,URep,Rep>);

    /// \brief Assigns this quantity with the specified count
    ///
    /// \param count the count to construct this with
    /// \return reference to (*this)
    constexpr auto operator=(const Rep& count) noexcept -> quantity&;

    //--------------------------------------------------------------------------
    // Observers
    //--------------------------------------------------------------------------
  public:

    /// \brief Gets the count from this quantity
    ///
    /// \return the count
    constexpr auto count() const noexcept -> size_type;

    //--------------------------------------------------------------------------
    // Unary Operators
    // -------------------------------------------------------------------------
  public:

    constexpr auto operator+() const noexcept -> const quantity&;
    constexpr auto operator-() const noexcept -> quantity;

    //--------------------------------------------------------------------------
    // Increment / Decrement Operators
    // -------------------------------------------------------------------------
  public:

    constexpr auto operator++() noexcept -> quantity&;
    constexpr auto operator++(int) noexcept -> quantity;
    constexpr auto operator--() noexcept -> quantity&;
    constexpr auto operator--(int) noexcept -> quantity;

    //--------------------------------------------------------------------------
    // Compound Arithmetic Operators
    //--------------------------------------------------------------------------
  public:

    template <typename U, typename URep>
    constexpr auto operator+=(const quantity<U,URep>& rhs) noexcept -> quantity&
      requires(msl::detail::quantity_operable<T,U,URep,Rep>);

    template <typename URep>
    constexpr auto operator+=(const URep& rhs) noexcept -> quantity&
      requires(std::convertible_to<URep,Rep>);

    template <typename U, typename URep>
    constexpr auto operator-=(const quantity<U,URep>& rhs) noexcept -> quantity&
      requires(msl::detail::quantity_operable<T,U,URep,Rep>);

    template <typename URep>
    constexpr auto operator-=(const URep& rhs) noexcept -> quantity&
      requires(std::convertible_to<URep,Rep>);

    template <typename URep>
    constexpr auto operator*=(const URep& rhs) noexcept -> quantity&
      requires(std::convertible_to<URep,Rep>);

    template <typename URep>
    constexpr auto operator/=(const URep& rhs) noexcept -> quantity&
      requires(std::convertible_to<URep,Rep>);

    template <typename U, typename URep>
    constexpr auto operator%=(const quantity<U,URep>& rhs) noexcept -> quantity&
      requires(msl::detail::quantity_operable<T,U,URep,Rep>);

    template <typename URep>
    constexpr auto operator%=(const URep& rhs) noexcept -> quantity&
      requires(std::convertible_to<URep,Rep>);

    //--------------------------------------------------------------------------
    // Comparisons
    //--------------------------------------------------------------------------
  public:

    template <typename U, typename URep>
    constexpr auto operator==(const quantity<U,URep>& other) const noexcept -> bool;

    template <typename URep>
    constexpr auto operator==(const URep& other) const noexcept -> bool
      requires(std::integral<URep>);

    auto operator<=>(const quantity&) const = default;

    template <typename U, typename URep>
    constexpr auto operator<=>(const quantity<U,URep>& other) const noexcept
      -> std::strong_ordering;

    template <typename URep>
    constexpr auto operator<=>(const URep& other) const noexcept -> std::strong_ordering
      requires(std::integral<URep>);

    //--------------------------------------------------------------------------
    // Private Members
    //--------------------------------------------------------------------------
  private:

    size_type m_value;
  };

  template <typename Rep>
  class quantity<std::byte,Rep> : public digital_quantity<Rep>
  {
    using base_type = digital_quantity<Rep>;

  public:

    using base_type::base_type;

    using base_type::operator=;
  };

  //============================================================================
  // aliases : class : quantity
  //============================================================================

  template <typename T>
  using uquantity = quantity<T, std::size_t>;

  //============================================================================
  // non-member functions : class : quantity
  //============================================================================

  //----------------------------------------------------------------------------
  // Arithmetic Operators
  //----------------------------------------------------------------------------

  template <typename T, typename Rep, typename U, typename URep>
  constexpr auto operator+(const quantity<T,Rep>& lhs, const quantity<U,URep>& rhs)
    noexcept -> quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>
    requires(msl::detail::quantity_operable<T,U,URep,Rep>);

  template <typename T, typename Rep, typename U, typename URep>
  constexpr auto operator-(const quantity<T,Rep>& lhs, const quantity<U,URep>& rhs)
    noexcept -> quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>
    requires(msl::detail::quantity_operable<T,U,URep,Rep>);

  template <typename T, typename Rep, typename URep>
  constexpr auto operator*(const quantity<T,Rep>& lhs, const URep& rhs)
    noexcept -> quantity<T,Rep>
    requires(std::convertible_to<URep,Rep>);

  template <typename T, typename Rep, typename URep>
  constexpr auto operator*(const URep& lhs, const quantity<T,Rep>& rhs)
    noexcept -> quantity<T,Rep>
    requires(std::convertible_to<URep,Rep>);

  template <typename T, typename Rep, typename URep>
  constexpr auto operator/(const quantity<T,Rep>& lhs, const URep& rhs)
    noexcept -> quantity<T,Rep>
    requires(std::convertible_to<URep,Rep>);

  template <typename T, typename Rep, typename URep>
  constexpr auto operator%(const quantity<T,Rep>& lhs, const URep& rhs)
    noexcept -> quantity<T,Rep>
    requires(std::convertible_to<URep,Rep>);

  template <typename T, typename Rep, typename U, typename URep>
  constexpr auto operator%(const quantity<T,Rep>& lhs,
                           const quantity<U,URep>& rhs)
    noexcept -> quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>
    requires(msl::detail::quantity_operable<T,U,URep,Rep>);

  //----------------------------------------------------------------------------
  // Pointer Arithmetic Operators
  //----------------------------------------------------------------------------

  template <typename T, typename Rep>
  constexpr
  auto operator+(T* lhs, quantity<T,Rep> rhs) noexcept -> T*;

  template <typename T, typename Rep>
  constexpr
  auto operator+(const T* lhs, quantity<T,Rep> rhs) noexcept -> const T*;

  template <typename T, typename Rep>
  constexpr
  auto operator+(quantity<T,Rep> lhs, T* rhs) noexcept -> T*;

  template <typename T, typename Rep>
  constexpr
  auto operator+(quantity<T,Rep> lhs, const T* rhs) noexcept -> const T*;

  template <typename T, typename Rep>
  constexpr
  auto operator-(T* lhs, quantity<T,Rep> rhs) noexcept -> T*;

  template <typename T, typename Rep>
  constexpr
  auto operator-(const T* lhs, quantity<T,Rep> rhs) noexcept -> const T*;

} // namespace msl

//==============================================================================
// definitions : class : quantity
//==============================================================================

//------------------------------------------------------------------------------
// Static Factories
//------------------------------------------------------------------------------

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::min()
  noexcept -> quantity<T,Rep>
{
  return quantity{std::numeric_limits<T>::min()};
}

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::max()
  noexcept -> quantity<T,Rep>
{
  return quantity{std::numeric_limits<T>::max()};
}

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::zero()
  noexcept -> quantity<T,Rep>
{
  return quantity{std::numeric_limits<T>::zero()};
}

//------------------------------------------------------------------------------
// Constructors / Assignment
//------------------------------------------------------------------------------

template <typename T, typename Rep>
inline constexpr
msl::quantity<T, Rep>::quantity(const Rep& count)
  noexcept
  : m_value{count}
{

}

template <typename T, typename Rep>
template <typename U, typename URep>
inline constexpr
msl::quantity<T, Rep>::quantity(const quantity<U,URep>& other)
  noexcept
  requires(msl::detail::quantity_operable<T,U,URep,Rep>)
  : m_value{other.count()}
{
}

//------------------------------------------------------------------------------

template <typename T, typename Rep>
template <typename U, typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator=(const quantity<U, URep>& other)
  noexcept -> quantity&
  requires(msl::detail::quantity_operable<T,U,URep,Rep>)
{
  m_value = other.count();
  return (*this);
}

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::operator=(const Rep& count)
  noexcept -> quantity&
{
  m_value = count;
  return (*this);
}

//------------------------------------------------------------------------------
// Observers
//------------------------------------------------------------------------------

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::count()
  const noexcept -> size_type
{
  return m_value;
}

//------------------------------------------------------------------------------
// Unary Operators
//------------------------------------------------------------------------------

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::operator+()
  const noexcept -> const quantity<T, Rep>&
{
  return (*this);
}

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::operator-()
  const noexcept -> quantity
{
  return quantity{-count()};
}

//------------------------------------------------------------------------------
// Increment / Decrement Operators
//------------------------------------------------------------------------------

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::operator++()
  noexcept -> quantity&
{
  ++m_value;
  return (*this);
}

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::operator++(int)
  noexcept -> quantity
{
  const auto copy = (*this);
  ++m_value;
  return copy;
}

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::operator--()
  noexcept -> quantity&
{
  --m_value;
  return (*this);
}

template <typename T, typename Rep>
inline constexpr
auto msl::quantity<T, Rep>::operator--(int)
  noexcept -> quantity
{
  const auto copy = (*this);
  --m_value;
  return copy;
}

//------------------------------------------------------------------------------
// Compound Operators
//------------------------------------------------------------------------------

template <typename T, typename Rep>
template <typename U, typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator+=(const quantity<U, URep>& rhs)
  noexcept -> quantity&
  requires(msl::detail::quantity_operable<T,U,URep,Rep>)
{
  m_value += rhs.count();
  return (*this);
}

template <typename T, typename Rep>
template <typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator+=(const URep& rhs)
  noexcept -> quantity&
  requires(std::convertible_to<URep, Rep>)
{
  m_value += rhs;
  return (*this);
}

template <typename T, typename Rep>
template <typename U, typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator-=(const quantity<U, URep>& rhs)
  noexcept -> quantity&
  requires(msl::detail::quantity_operable<T,U,URep,Rep>)
{
  m_value -= rhs.count();
  return (*this);
}

template <typename T, typename Rep>
template <typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator-=(const URep& rhs)
  noexcept -> quantity&
  requires(std::convertible_to<URep, Rep>)
{
  m_value -= rhs;
  return (*this);
}

template <typename T, typename Rep>
template <typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator*=(const URep& rhs)
  noexcept -> quantity&
  requires(std::convertible_to<URep, Rep>)
{
  m_value *= rhs;
  return (*this);
}

template <typename T, typename Rep>
template <typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator/=(const URep& rhs)
  noexcept -> quantity&
  requires(std::convertible_to<URep, Rep>)
{
  m_value /= rhs;
  return (*this);
}

template <typename T, typename Rep>
template <typename U, typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator%=(const quantity<U, URep>& rhs)
  noexcept -> quantity&
  requires(msl::detail::quantity_operable<T,U,URep,Rep>)
{
  m_value %= rhs;
  return (*this);
}

template <typename T, typename Rep>
template <typename URep>
inline constexpr
auto msl::quantity<T, Rep>::operator%=(const URep& rhs)
  noexcept -> quantity&
  requires(std::convertible_to<URep, Rep>)
{
  m_value %= rhs.count();
  return (*this);
}

//==============================================================================
// definitions : non-member functions : class : quantity
//==============================================================================

//------------------------------------------------------------------------------
// Comparison Operators
//------------------------------------------------------------------------------

template <typename T, typename Rep>
template <typename U, typename URep>
inline constexpr
auto msl::quantity<T,Rep>::operator==(const quantity<U,URep>& other)
  const noexcept -> bool
{
  return m_value == other.count();
}

template <typename T, typename Rep>
template <typename URep>
inline constexpr
auto msl::quantity<T,Rep>::operator==(const URep& other)
  const noexcept -> bool
  requires(std::integral<URep>)
{
  return m_value == other;
}

template <typename T, typename Rep>
template <typename U, typename URep>
inline constexpr
auto msl::quantity<T,Rep>::operator<=>(const quantity<U,URep>& other)
  const noexcept -> std::strong_ordering
{
  return m_value <=> other.count();
}

template <typename T, typename Rep>
template <typename URep>
inline constexpr
auto msl::quantity<T,Rep>::operator<=>(const URep& other)
  const noexcept -> std::strong_ordering
  requires(std::integral<URep>)
{
  return m_value <=> other;
}

//------------------------------------------------------------------------------
// Arithmetic Operators
//------------------------------------------------------------------------------

template <typename T, typename Rep, typename U, typename URep>
inline constexpr
auto msl::operator+(const quantity<T,Rep>& lhs, const quantity<U,URep>& rhs)
  noexcept -> quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>
  requires(msl::detail::quantity_operable<T,U,URep,Rep>)
{
  using result_type = quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>;

  return result_type{lhs.count() + rhs.count()};
}

template <typename T, typename Rep, typename U, typename URep>
inline constexpr
auto msl::operator-(const quantity<T,Rep>& lhs, const quantity<U,URep>& rhs)
  noexcept -> quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>
  requires(msl::detail::quantity_operable<T,U,URep,Rep>)
{
  using result_type = quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>;

  return result_type{lhs.count() - rhs.count()};
}

template <typename T, typename Rep, typename URep>
inline constexpr
auto msl::operator*(const quantity<T,Rep>& lhs, const URep& rhs)
  noexcept -> quantity<T,Rep>
  requires(std::convertible_to<URep,Rep>)
{
  return quantity<T, Rep>{lhs.count() * rhs};
}

template <typename T, typename Rep, typename URep>
inline constexpr
auto msl::operator*(const URep& lhs, const quantity<T,Rep>& rhs)
  noexcept -> quantity<T,Rep>
  requires(std::convertible_to<URep,Rep>)
{
  return quantity<T, Rep>{lhs * rhs.count()};
}

template <typename T, typename Rep, typename URep>
inline constexpr
auto msl::operator/(const quantity<T,Rep>& lhs, const URep& rhs)
  noexcept -> quantity<T,Rep>
  requires(std::convertible_to<URep,Rep>)
{
  return quantity<T, Rep>{lhs.count() / rhs};
}

template <typename T, typename Rep, typename URep>
inline constexpr
auto msl::operator%(const quantity<T,Rep>& lhs, const URep& rhs)
  noexcept -> quantity<T,Rep>
  requires(std::convertible_to<URep,Rep>)
{
  return quantity<T, Rep>{lhs.count() % rhs};
}

template <typename T, typename Rep, typename U, typename URep>
inline constexpr
auto msl::operator%(const quantity<T,Rep>& lhs, const quantity<U,URep>& rhs)
  noexcept -> quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>
  requires(msl::detail::quantity_operable<T,U,URep,Rep>)
{
  using result_type = quantity<detail::quantity_base_t<T,U>,std::common_type_t<Rep,URep>>;

  return result_type{lhs.count() % rhs.count()};
}

//------------------------------------------------------------------------------
// Pointer Arithmetic Operators
//------------------------------------------------------------------------------

template <typename T, typename Rep>
inline constexpr
auto msl::operator+(T* lhs, quantity<T,Rep> rhs)
  noexcept -> T*
{
  return lhs + rhs.count();
}

template <typename T, typename Rep>
inline constexpr
auto msl::operator+(const T* lhs, quantity<T,Rep> rhs)
  noexcept -> const T*
{
  return lhs + rhs.count();
}

template <typename T, typename Rep>
inline constexpr
auto msl::operator+(quantity<T,Rep> lhs, T* rhs)
  noexcept -> T*
{
  return lhs.count() + rhs;
}

template <typename T, typename Rep>
inline constexpr
auto msl::operator+(quantity<T,Rep> lhs, const T* rhs)
  noexcept -> const T*
{
  return lhs.count() + rhs;
}

//------------------------------------------------------------------------------

template <typename T, typename Rep>
inline constexpr
auto msl::operator-(T* lhs, quantity<T,Rep> rhs)
  noexcept -> T*
{
  return lhs - rhs.count();
}

template <typename T, typename Rep>
inline constexpr
auto msl::operator-(const T* lhs, quantity<T,Rep> rhs)
  noexcept -> const T*
{
  return lhs - rhs.count();
}

#endif /* MSL_QUANTITIES_QUANTITY_HPP */
