////////////////////////////////////////////////////////////////////////////////
/// \file digital_quantity.hpp
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

#ifndef MSL_QUANTITIES_DIGITAL_QUANTITY_HPP
#define MSL_QUANTITIES_DIGITAL_QUANTITY_HPP

#if defined(_MSC_VER)
# pragma once
#endif // defined(_MSC_VER)

#include <ratio>       // std::ratio
#include <cstdint>     // std::uint64_t, etc
#include <climits>     // CHAR_BIT
#include <cstddef>     // std::byte
#include <type_traits> // std::enable_if, std::common_type
#include <limits>      // std::numeric_limits
#include <concepts>    // std::convertible_to, etc
#include <compare>

namespace msl {

  //===========================================================================
  // class : digital_quantity
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief Represents a storage digital_quantity for digital media
  ///
  /// Capacity uses the underlying 'Base' unit for handling
  ///
  /// \tparam Rep the underlying representation of this digital_quantity
  /// \tparam Base The SI base unit value
  /////////////////////////////////////////////////////////////////////////////
  template <typename Rep, typename Base = std::ratio<1>>
  class digital_quantity
  {
    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using rep = Rep;
    using base = typename Base::type;

    static_assert(
      base::den <= CHAR_BIT,
      "It is not possible to have a digital_quantity that represents something "
      "that is smaller than 1 bit"
    );
    static_assert(
      std::is_integral<Rep>::value,
      "Representation must be an integral value; loss of precision is not "
      "supported."
    );

    //-------------------------------------------------------------------------
    // Public Static Factories
    //-------------------------------------------------------------------------
  public:

    /// \brief Returns a zero-size digital_quantity
    ///
    /// \return the digital_quantity
    static constexpr auto zero() noexcept -> digital_quantity;

    /// \brief Returns the minimum digital_quantity this can support
    ///
    /// \return the digital_quantity
    static constexpr auto min() noexcept -> digital_quantity;

    /// \brief Returns the maximum digital_quantity this can support
    ///
    /// \return the digital_quantity
    static constexpr auto max() noexcept -> digital_quantity;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Default-constructs this digital_quantity
    digital_quantity() = default;

    /// \brief Constructs this digital_quantity from an underlying representation
    ///
    /// \param r the value
    template <typename URep>
    constexpr explicit digital_quantity(const URep& r) noexcept
      requires(std::convertible_to<URep,Rep>);

    /// \brief Converts \p other to this digital_quantity
    ///
    /// This constructor is only enabled if there will be no
    /// loss-of-precision. To have a lossy conversion, see digital_quantity_cast
    ///
    /// \param other the other digital_quantity to convert
    template <typename URep, typename UBase>
    constexpr digital_quantity(digital_quantity<URep,UBase> other) noexcept
      requires(std::convertible_to<URep,Rep> && (std::ratio_divide<UBase, Base>::den == 1));

    /// \brief Copy-constructs this \p digital_quantity from \p other
    ///
    /// \param other the other digital_quantity to copy
    digital_quantity(const digital_quantity& other) = default;

    //-------------------------------------------------------------------------

    /// \brief Assigns the contents of \p other to this
    ///
    /// \param other the other digital_quantity to copy
    /// \return reference to \c (*this)
    auto operator=(const digital_quantity& other) -> digital_quantity& = default;

    //-------------------------------------------------------------------------
    // Arithmetic Operators
    //-------------------------------------------------------------------------
  public:

    constexpr auto operator+() const noexcept -> std::common_type_t<digital_quantity>;
    constexpr auto operator-() const noexcept -> std::common_type_t<digital_quantity>;

    constexpr auto operator++() noexcept -> digital_quantity&;
    constexpr auto operator++(int) noexcept -> digital_quantity;
    constexpr auto operator--() noexcept -> digital_quantity&;
    constexpr auto operator--(int) noexcept -> digital_quantity;

    constexpr auto operator+=(const digital_quantity& rhs) noexcept -> digital_quantity&;
    constexpr auto operator-=(const digital_quantity& rhs) noexcept -> digital_quantity&;
    constexpr auto operator*=(const rep& r) noexcept -> digital_quantity&;
    constexpr auto operator/=(const rep& r) noexcept -> digital_quantity&;
    constexpr auto operator%=(const rep& r) noexcept -> digital_quantity&;
    constexpr auto operator%=(const digital_quantity& rhs) noexcept -> digital_quantity&;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the total count of the digital_quantity
    ///
    /// \return the count
    constexpr auto count() const noexcept -> Rep;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

      Rep m_count;
  };

  //===========================================================================
  // aliases : class : std::ratio
  //===========================================================================

  // SI units

  inline namespace si {

    using kilo = std::kilo;
    using mega = std::mega;
    using giga = std::giga;
    using tera = std::tera;
    using peta = std::peta;
    using exa  = std::exa;

    using kibi = std::ratio<1024>;
    using mebi = std::ratio_multiply<std::ratio<1024>,kibi>;
    using gibi = std::ratio_multiply<std::ratio<1024>,mebi>;
    using tebi = std::ratio_multiply<std::ratio<1024>,gibi>;
    using pebi = std::ratio_multiply<std::ratio<1024>,tebi>;
    using exbi = std::ratio_multiply<std::ratio<1024>,pebi>;

  } // namespace si

  //===========================================================================
  // trait : is_digital_quantity
  //===========================================================================

  template <typename T>
  struct is_digital_quantity : std::false_type{};

  template <typename Rep, typename Base>
  struct is_digital_quantity<digital_quantity<Rep,Base>> : std::true_type{};

  template <typename T>
  inline constexpr bool is_digital_quantity_v = is_digital_quantity<T>::value;

  //===========================================================================
  // aliases : class : digital_quantity
  //===========================================================================

  //---------------------------------------------------------------------------
  // Byte Units
  //---------------------------------------------------------------------------

  using bytes     = digital_quantity<std::size_t>;
  using kilobytes = digital_quantity<std::uint64_t, si::kilo>; // at least 54 bits
  using megabytes = digital_quantity<std::uint64_t, si::mega>; // at least 44 bits
  using gigabytes = digital_quantity<std::uint64_t, si::giga>; // at least 34 bits
  using terabytes = digital_quantity<std::uint32_t, si::tera>; // at least 24 bits
  using petabytes = digital_quantity<std::uint16_t, si::peta>; // at least 16 bits

  using kibibytes = digital_quantity<std::uint64_t, si::kibi>; // at least 54 bits
  using mebibytes = digital_quantity<std::uint64_t, si::mebi>; // at least 44 bits
  using gibibytes = digital_quantity<std::uint64_t, si::gibi>; // at least 34 bits
  using tebibytes = digital_quantity<std::uint32_t, si::tebi>; // at least 24 bits
  using pebibytes = digital_quantity<std::uint16_t, si::pebi>; // at least 16 bits

  //---------------------------------------------------------------------------
  // Bit Units
  //---------------------------------------------------------------------------

  /// \brief The smallest / atomic unit
  using bits      = digital_quantity<std::uint64_t, std::ratio<1, CHAR_BIT>>;
  using kilobits  = digital_quantity<std::uint64_t, std::ratio<si::kilo::num, CHAR_BIT>::type>;
  using megabits  = digital_quantity<std::uint64_t, std::ratio<si::mega::num, CHAR_BIT>::type>;
  using gigabits  = digital_quantity<std::uint64_t, std::ratio<si::giga::num, CHAR_BIT>::type>;
  using terabits  = digital_quantity<std::uint32_t, std::ratio<si::tera::num, CHAR_BIT>::type>;
  using petabits  = digital_quantity<std::uint32_t, std::ratio<si::peta::num, CHAR_BIT>::type>;

  //---------------------------------------------------------------------------

  using kibibits  = digital_quantity<std::uint64_t, std::ratio<si::kibi::num, CHAR_BIT>::type>;
  using mebibits  = digital_quantity<std::uint64_t, std::ratio<si::mebi::num, CHAR_BIT>::type>;
  using gibibits  = digital_quantity<std::uint64_t, std::ratio<si::gibi::num, CHAR_BIT>::type>;
  using tebibits  = digital_quantity<std::uint32_t, std::ratio<si::tebi::num, CHAR_BIT>::type>;
  using pebibits  = digital_quantity<std::uint32_t, std::ratio<si::pebi::num, CHAR_BIT>::type>;

  //---------------------------------------------------------------------------
  // Non-standard units
  //---------------------------------------------------------------------------

  // A nibble is defined as half of a byte (irrespective of the number of bits
  // in a byte)
  using nibbles = digital_quantity<std::uint64_t, std::ratio<1,2>>;

  //===========================================================================
  // non-member functions : class : digital_quantity
  //===========================================================================

  //---------------------------------------------------------------------------
  // Arithmetic Operators
  //---------------------------------------------------------------------------

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr auto operator+(const digital_quantity<Rep1,Base1>& lhs,
                           const digital_quantity<Rep2,Base2>& rhs)
     noexcept -> std::common_type_t<digital_quantity<Rep1,Base1>,
                                    digital_quantity<Rep2,Base2>>;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr auto operator-(const digital_quantity<Rep1,Base1>& lhs,
                           const digital_quantity<Rep2,Base2>& rhs)
    noexcept -> std::common_type_t<digital_quantity<Rep1,Base1>,
                                   digital_quantity<Rep2,Base2>>;

  template <typename Rep1, typename Base, typename Rep2>
  constexpr auto operator*(const digital_quantity<Rep1,Base>& d, const Rep2& s)
    noexcept -> digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  template <typename Rep1, typename Rep2, typename Base>
  constexpr auto operator*(const Rep1& s, const digital_quantity<Rep2,Base>& d)
    noexcept -> digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  template <typename Rep1, typename Base, typename Rep2>
  constexpr auto operator/(const digital_quantity<Rep1,Base>& d, const Rep2& s)
    noexcept -> digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr auto operator/(const digital_quantity<Rep1,Base1>& lhs,
                           const digital_quantity<Rep2,Base2>& rhs)
    noexcept -> std::common_type_t<Rep1,Rep2>;

  template <typename Rep1, typename Base, typename Rep2>
  constexpr auto operator%(const digital_quantity<Rep1, Base>& d, const Rep2& s)
    noexcept -> digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr auto operator%(const digital_quantity<Rep1,Base1>& lhs,
                           const digital_quantity<Rep2,Base2>& rhs)
    noexcept -> std::common_type_t<digital_quantity<Rep1,Base1>,
                                   digital_quantity<Rep2,Base2>>;

  //---------------------------------------------------------------------------
  // Comparison Operators
  //---------------------------------------------------------------------------

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr auto operator==(const digital_quantity<Rep1, Base1>& lhs,
                            const digital_quantity<Rep2, Base2>& rhs)
    noexcept -> bool;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr auto operator<=>(const digital_quantity<Rep1, Base1>& lhs,
                             const digital_quantity<Rep2, Base2>& rhs)
    noexcept -> std::strong_ordering;

  //---------------------------------------------------------------------------
  // Utilities
  //---------------------------------------------------------------------------

  /// \brief Converts \p from to a ToQuantity with lossy conversion
  ///
  /// This is the only way to convert types with loss of precision.
  ///
  /// \tparam ToQuantity the digital_quantity to convert to
  /// \param from the digital_quantity to convert
  /// \return the converted digital_quantity
  template <typename ToQuantity, typename FromQuantity,
            typename = std::enable_if_t<is_digital_quantity_v<ToQuantity> &&
                                        is_digital_quantity_v<FromQuantity>>>
  constexpr auto digital_quantity_cast(const FromQuantity& from)
    noexcept -> ToQuantity;

  //---------------------------------------------------------------------------

  /// \brief Computes the absolute difference between \p lhs and \p rhs
  ///
  /// \param lhs the left digital_quantity
  /// \param rhs the right digital_quantity
  /// \return the difference between the two capacities
  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr auto difference(const digital_quantity<Rep1,Base1>& lhs,
                            const digital_quantity<Rep2,Base2>& rhs)
    noexcept -> std::common_type_t<digital_quantity<Rep1,Base1>,
                                   digital_quantity<Rep2,Base2>>;

  /// \brief Rounds the specified digital_quantity down to ToQuantity
  ///
  /// \tparam ToQuantity the digital_quantity to round down to
  /// \param c the digital_quantity
  /// \return the rounded digital_quantity
  template <typename ToQuantity, typename Rep, typename Base,
            typename = std::enable_if_t<is_digital_quantity_v<ToQuantity>>>
  constexpr auto floor(const digital_quantity<Rep, Base>& c) noexcept -> ToQuantity;


  /// \brief Rounds the specified digital_quantity up to ToQuantity
  ///
  /// \tparam ToQuantity the digital_quantity to round up to
  /// \param c the digital_quantity
  /// \return the rounded digital_quantity
  template <typename ToQuantity, typename Rep, typename Base,
            typename = std::enable_if_t<is_digital_quantity_v<ToQuantity>>>
  constexpr auto ceil(const digital_quantity<Rep, Base>& c) noexcept -> ToQuantity;

  /// \brief Rounds the specified digital_quantity \p c to the nearest ToQuantity
  ///
  /// This follows the typical C++ rounding model, and behaves like std::round
  /// by always rounding away from 0.
  ///
  /// \tparam ToQuantity the digital_quantity to round to
  /// \param c the digital_quantity to round
  /// \return the rounded digital_quantity
  template <typename ToQuantity, typename Rep, typename Base,
            typename = std::enable_if_t<is_digital_quantity_v<ToQuantity>>>
  constexpr auto round(const digital_quantity<Rep, Base>& c) noexcept -> ToQuantity;

  /// \brief Gets the absolute value of the specified digital_quantity
  ///
  /// \param c the digital_quantity to get the absolute value of
  /// \return the absolute value of the digital_quantity
  template <typename Rep, typename Base>
  constexpr auto abs(const digital_quantity<Rep, Base>& c) noexcept -> digital_quantity<Rep, Base>;

  //---------------------------------------------------------------------------

  /// \brief Gets the size of the specified type, in bytes
  ///
  /// \tparam T the type to get the size of
  template <typename T>
  constexpr auto size_of() noexcept -> bytes;

  /// \brief Gets the size of the specified value, in bytes
  ///
  /// \param v the value to get the size of
  template <typename T>
  constexpr auto size_of(const T& v) noexcept -> bytes;

  //===========================================================================
  // literals : class : digital_quantity
  //===========================================================================

  inline namespace literals {

    //-------------------------------------------------------------------------
    // Byte Literals
    //-------------------------------------------------------------------------

    inline namespace byte_literals {

      constexpr auto operator""_b(unsigned long long x) noexcept -> bytes;
      constexpr auto operator""_kb(unsigned long long x) noexcept -> kilobytes;
      constexpr auto operator""_mb(unsigned long long x) noexcept -> megabytes;
      constexpr auto operator""_gb(unsigned long long x) noexcept -> gigabytes;
      constexpr auto operator""_tb(unsigned long long x) noexcept -> terabytes;
      constexpr auto operator""_pb(unsigned long long x) noexcept -> petabytes;

      //-----------------------------------------------------------------------

      constexpr auto operator""_kib(unsigned long long x) noexcept -> kibibytes;
      constexpr auto operator""_mib(unsigned long long x) noexcept -> mebibytes;
      constexpr auto operator""_gib(unsigned long long x) noexcept -> gibibytes;
      constexpr auto operator""_tib(unsigned long long x) noexcept -> tebibytes;
      constexpr auto operator""_pib(unsigned long long x) noexcept -> pebibytes;

    } // namespace byte_literals

    //-------------------------------------------------------------------------
    // Bit Literals
    //-------------------------------------------------------------------------

    inline namespace bit_literals {

      constexpr auto operator""_bit(unsigned long long x) noexcept -> bits;
      constexpr auto operator""_kbit(unsigned long long x) noexcept -> kilobits;
      constexpr auto operator""_mbit(unsigned long long x) noexcept -> megabits;
      constexpr auto operator""_gbit(unsigned long long x) noexcept -> gigabits;
      constexpr auto operator""_tbit(unsigned long long x) noexcept -> terabits;
      constexpr auto operator""_pbit(unsigned long long x) noexcept -> petabits;

      //-----------------------------------------------------------------------

      constexpr auto operator""_kibit(unsigned long long x) noexcept -> kibibits;
      constexpr auto operator""_mibit(unsigned long long x) noexcept -> mebibits;
      constexpr auto operator""_gibit(unsigned long long x) noexcept -> gibibits;
      constexpr auto operator""_tibit(unsigned long long x) noexcept -> tebibits;
      constexpr auto operator""_pibit(unsigned long long x) noexcept -> pebibits;

    } // inline namespace byte_literals
  } // inline namespace literals

  //===========================================================================
  // non-member functions : alias : digital_quantity
  //===========================================================================

  //---------------------------------------------------------------------------
  // Arithmetic Operators
  //---------------------------------------------------------------------------

  template <typename Rep, typename Base,
            typename = std::enable_if_t<Base::den == 1>>
  constexpr auto operator+(std::byte* p, digital_quantity<Rep,Base> offset)
    noexcept -> std::byte*;

  template <typename Rep, typename Base,
            typename = std::enable_if_t<Base::den == 1>>
  constexpr auto operator+(const std::byte* p, digital_quantity<Rep,Base> offset)
    noexcept -> const std::byte*;

  template <typename Rep, typename Base,
            typename = std::enable_if_t<Base::den == 1>>
  constexpr auto operator-(std::byte* p, digital_quantity<Rep,Base> offset)
    noexcept -> std::byte*;

  template <typename Rep, typename Base,
            typename = std::enable_if_t<Base::den == 1>>
  constexpr auto operator-(const std::byte* p, digital_quantity<Rep,Base> offset)
    noexcept -> const std::byte*;

} // namespace msl

namespace msl::detail {

  template <std::intmax_t X, std::intmax_t Y>
  struct gcd : gcd<Y, X % Y>{};

  template <std::intmax_t X>
  struct gcd<X,0> : std::integral_constant<std::intmax_t, X>{};

  template <std::intmax_t X, std::intmax_t Y>
	struct lcm : std::integral_constant<std::intmax_t, (X / gcd<X,Y>::value) * Y>{};

} // namespace msl::detail

namespace std {

  // Compute the common type of two capacities
  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  struct common_type<msl::digital_quantity<Rep1, Base1>, msl::digital_quantity<Rep2, Base2>>
  {
    using type = msl::digital_quantity<
      std::common_type_t<Rep1, Rep2>,
      typename std::ratio<
        msl::detail::gcd<Base1::num, Base2::num>::value,
        msl::detail::lcm<Base1::den, Base2::den>::value
      >::type
    >;
  };

} // namespace std

//------------------------------------------------------------------------------
// Public Static Factories
//------------------------------------------------------------------------------

template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::zero()
  noexcept -> digital_quantity
{
  return digital_quantity{0};
}

template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::min()
  noexcept -> digital_quantity
{
  return digital_quantity{std::numeric_limits<Rep>::min()};
}

template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::max()
  noexcept -> digital_quantity
{
  return digital_quantity{std::numeric_limits<Rep>::max()};
}

//------------------------------------------------------------------------------
// Constructors
//------------------------------------------------------------------------------

template <typename Rep, typename Base>
template <typename URep>
inline constexpr
msl::digital_quantity<Rep,Base>::digital_quantity(const URep& r)
  noexcept requires(std::convertible_to<URep,Rep>)
  : m_count{static_cast<Rep>(r)}
{

}

template <typename Rep, typename Base>
template <typename URep, typename UBase>
inline constexpr
msl::digital_quantity<Rep,Base>::digital_quantity(digital_quantity<URep,UBase> other)
  noexcept
  requires(std::convertible_to<URep,Rep> && (std::ratio_divide<UBase, Base>::den == 1))
  : m_count{static_cast<Rep>(other.count() * std::ratio_divide<UBase, Base>::num)}
{

}

//------------------------------------------------------------------------------
// Arithmetic Operators
//------------------------------------------------------------------------------

template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator+()
  const noexcept -> std::common_type_t<digital_quantity>
{
  using type = std::common_type_t<digital_quantity>;

  return type{m_count};
}

template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator-()
  const noexcept -> std::common_type_t<digital_quantity>
{
  using type = std::common_type_t<digital_quantity>;

  return type{-m_count};
}

template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator++()
  noexcept -> digital_quantity<Rep,Base>&
{
  ++m_count;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator++(int)
  noexcept -> digital_quantity<Rep,Base>
{
  const auto copy = (*this);
  ++m_count;
  return copy;
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator--()
  noexcept -> digital_quantity<Rep,Base>&
{
  --m_count;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator--(int)
  noexcept -> digital_quantity<Rep,Base>
{
  const auto copy = (*this);
  --m_count;
  return copy;
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator+=(const digital_quantity& rhs)
  noexcept -> digital_quantity&
{
  m_count += rhs.count();
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator-=(const digital_quantity& rhs)
  noexcept -> digital_quantity&
{
  m_count -= rhs.count();
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator*=(const rep& r)
  noexcept -> digital_quantity&
{
  m_count *= r;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator/=(const rep& r)
  noexcept -> digital_quantity&
{
  m_count /= r;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator%=(const rep& r)
  noexcept -> digital_quantity&
{
  m_count %= r;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::operator%=(const digital_quantity& rhs)
  noexcept -> digital_quantity&
{
  m_count %= rhs.count();
  return (*this);
}

//------------------------------------------------------------------------------
// Observers
//------------------------------------------------------------------------------

template <typename Rep, typename Base>
inline constexpr
auto msl::digital_quantity<Rep,Base>::count()
  const noexcept -> Rep
{
  return m_count;
}


//==============================================================================
// definitions : non-member functions : class : digital_quantity
//==============================================================================

//------------------------------------------------------------------------------
// Arithmetic Operators
//------------------------------------------------------------------------------

template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr
auto msl::operator+(const digital_quantity<Rep1,Base1>& lhs,
                    const digital_quantity<Rep2,Base2>& rhs)
  noexcept -> std::common_type_t<digital_quantity<Rep1,Base1>,
                                 digital_quantity<Rep2,Base2>>
{
  using result_type = std::common_type_t<
    digital_quantity<Rep1,Base1>,
    digital_quantity<Rep2,Base2>
  >;

  return result_type{
    result_type{lhs}.count() + result_type{rhs}.count()
  };
}

template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr
auto msl::operator-(const digital_quantity<Rep1,Base1>& lhs,
                    const digital_quantity<Rep2,Base2>& rhs)
  noexcept -> std::common_type_t<digital_quantity<Rep1,Base1>,
                                 digital_quantity<Rep2,Base2>>
{
  using result_type = std::common_type_t<
    digital_quantity<Rep1,Base1>,
    digital_quantity<Rep2,Base2>
  >;

  return result_type{
    result_type{lhs}.count() - result_type{rhs}.count()
  };
}


template <typename Rep1, typename Base, typename Rep2>
inline constexpr
auto msl::operator*(const digital_quantity<Rep1,Base>& lhs, const Rep2& rhs)
  noexcept -> digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
{
  using result_type = digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  return result_type{
    result_type{lhs}.count() * rhs
  };
}


template <typename Rep1, typename Rep2, typename Base>
inline constexpr
auto msl::operator*(const Rep1& lhs, const digital_quantity<Rep2,Base>& rhs)
  noexcept -> digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
{
  using result_type = digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  return result_type{
    lhs * result_type{rhs}.count()
  };
}


template <typename Rep1, typename Base, typename Rep2>
inline constexpr
auto msl::operator/(const digital_quantity<Rep1,Base>& lhs, const Rep2& rhs)
  noexcept -> digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
{
  using result_type = digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  return result_type{
    result_type{lhs}.count() / rhs
  };
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr
auto msl::operator/(const digital_quantity<Rep1,Base1>& lhs,
                    const digital_quantity<Rep2,Base2>& rhs)
  noexcept -> std::common_type_t<Rep1,Rep2>
{
  using result_type = std::common_type_t<Rep1,Rep2>;
  using common_type = std::common_type_t<
    digital_quantity<Rep1,Base1>,
    digital_quantity<Rep2,Base2>
  >;

  return result_type{
    common_type{lhs}.count() / common_type{rhs}.count()
  };
}


template <typename Rep1, typename Base, typename Rep2>
inline constexpr
auto msl::operator%(const digital_quantity<Rep1, Base>& lhs, const Rep2& rhs)
  noexcept -> digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
{
  using result_type = digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  return result_type{
    result_type{lhs}.count() % rhs
  };
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr
auto msl::operator%(const digital_quantity<Rep1,Base1>& lhs,
                    const digital_quantity<Rep2,Base2>& rhs)
  noexcept -> std::common_type_t<digital_quantity<Rep1,Base1>,
                                 digital_quantity<Rep2,Base2>>
{
  using result_type = std::common_type_t<
    digital_quantity<Rep1,Base1>,
    digital_quantity<Rep2,Base2>
  >;

  return result_type{
    result_type{lhs}.count() % result_type{rhs}.count()
  };
}


//------------------------------------------------------------------------------
// Comparison Operators
//------------------------------------------------------------------------------

template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr
auto msl::operator==(const digital_quantity<Rep1, Base1>& lhs,
                     const digital_quantity<Rep2, Base2>& rhs)
  noexcept -> bool
{
  using common_type = std::common_type_t<
    digital_quantity<Rep1, Base1>,
    digital_quantity<Rep2, Base2>
  >;

  return common_type{lhs}.count() == common_type{rhs}.count();
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr
auto msl::operator<=>(const digital_quantity<Rep1, Base1>& lhs,
                      const digital_quantity<Rep2, Base2>& rhs)
  noexcept -> std::strong_ordering
{
  using common_type = std::common_type_t<
    digital_quantity<Rep1, Base1>,
    digital_quantity<Rep2, Base2>
  >;

  return common_type{lhs}.count() <=> common_type{rhs}.count();
}

//------------------------------------------------------------------------------
// Utilities
//------------------------------------------------------------------------------

template <typename ToQuantity, typename FromQuantity, typename>
inline constexpr
auto msl::digital_quantity_cast(const FromQuantity& from)
  noexcept -> ToQuantity
{
  using from_base = typename FromQuantity::base;
  using to_base = typename ToQuantity::base;
  using to_rep = typename ToQuantity::rep;
  using result = std::ratio_divide<from_base, to_base>;

  const auto num = static_cast<to_rep>(from.count() * result::num);
  const auto den = static_cast<to_rep>(result::den);

  return ToQuantity{num / den};
}

//---------------------------------------------------------------------------

template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr
auto msl::difference(const digital_quantity<Rep1,Base1>& lhs,
                     const digital_quantity<Rep2,Base2>& rhs)
  noexcept -> std::common_type_t<digital_quantity<Rep1,Base1>,
                                 digital_quantity<Rep2,Base2>>
{
  using result_type = std::common_type_t<
    digital_quantity<Rep1,Base1>,
    digital_quantity<Rep2,Base2>
  >;

  const auto l = result_type{lhs};
  const auto r = result_type{rhs};

  if (l < r) {
    return r - l;
  }
  return l - r;
}


template <typename ToQuantity, typename Rep, typename Base, typename>
inline constexpr
auto msl::floor(const digital_quantity<Rep, Base>& c)
  noexcept -> ToQuantity
{
  const auto to = digital_quantity_cast<ToQuantity>(c);
  if (to > c) {
    return to - ToQuantity{1};
  }
  return to;
}


template <typename ToQuantity, typename Rep, typename Base, typename>
inline constexpr
auto msl::ceil(const digital_quantity<Rep, Base>& c)
  noexcept -> ToQuantity
{
  const auto to = digital_quantity_cast<ToQuantity>(c);
  if (to < c) {
    return to + ToQuantity{1};
  }
  return to;
}


template <typename ToQuantity, typename Rep, typename Base, typename>
inline constexpr
auto msl::round(const digital_quantity<Rep, Base>& c)
  noexcept -> ToQuantity
{
  const auto lower = floor<ToQuantity>(c);
  const auto upper = lower + ToQuantity{1};

  const auto lower_diff = difference(c, lower);
  const auto upper_diff = difference(upper, c);

  // Differences are the same -- means either 'c' is already rounded,
  // or that it's dead-center -- which should be rounded up to behave like
  // the standard rounding that C++ offers (0.5 -> 1, -0.5 -> -1)
  if (lower_diff == upper_diff) {
    if (upper > ToQuantity::zero()) {
      return upper;
    }
    return lower;
  }

  // round to the closer difference
  if (lower_diff < upper_diff) {
    return lower;
  }
  return upper;
}


template <typename Rep, typename Base>
inline constexpr
auto msl::abs(const digital_quantity<Rep, Base>& c)
  noexcept -> digital_quantity<Rep, Base>
{
  return c >= digital_quantity<Rep, Base>::zero() ? c : -c;
}

//------------------------------------------------------------------------------

template <typename T>
inline constexpr auto msl::size_of()
  noexcept -> bytes
{
  return bytes{sizeof(T)};
}


template <typename T>
inline constexpr auto msl::size_of(const T& v)
  noexcept -> bytes
{
  return bytes{sizeof(v)};
}

//==============================================================================
// literals : class : digital_quantity
//==============================================================================

//------------------------------------------------------------------------------
// Byte Literals
//------------------------------------------------------------------------------

inline constexpr
auto msl::literals::byte_literals::operator""_b(unsigned long long x)
  noexcept -> bytes
{
  return bytes{static_cast<bytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_kb(unsigned long long x)
  noexcept -> kilobytes
{
  return kilobytes{static_cast<kilobytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_mb(unsigned long long x)
  noexcept -> megabytes
{
  return megabytes{static_cast<megabytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_gb(unsigned long long x)
  noexcept -> gigabytes
{
  return gigabytes{static_cast<gigabytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_tb(unsigned long long x)
  noexcept -> terabytes
{
  return terabytes{static_cast<terabytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_pb(unsigned long long x)
  noexcept -> petabytes
{
  return petabytes{static_cast<petabytes::rep>(x)};
}

//------------------------------------------------------------------------------

inline constexpr
auto msl::literals::byte_literals::operator""_kib(unsigned long long x)
  noexcept -> kibibytes
{
  return kibibytes{static_cast<kibibytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_mib(unsigned long long x)
  noexcept -> mebibytes
{
  return mebibytes{static_cast<mebibytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_gib(unsigned long long x)
  noexcept -> gibibytes
{
  return gibibytes{static_cast<gibibytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_tib(unsigned long long x)
  noexcept -> tebibytes
{
  return tebibytes{static_cast<tebibytes::rep>(x)};
}


inline constexpr
auto msl::literals::byte_literals::operator""_pib(unsigned long long x)
  noexcept -> pebibytes
{
  return pebibytes{static_cast<pebibytes::rep>(x)};
}

//------------------------------------------------------------------------------
// Bit Literals
//------------------------------------------------------------------------------

inline constexpr
auto msl::literals::bit_literals::operator""_bit(unsigned long long x)
  noexcept -> bits
{
  return bits{static_cast<bits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_kbit(unsigned long long x)
  noexcept -> kilobits
{
  return kilobits{static_cast<kilobits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_mbit(unsigned long long x)
  noexcept -> megabits
{
  return megabits{static_cast<megabits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_gbit(unsigned long long x)
  noexcept -> gigabits
{
  return gigabits{static_cast<gigabits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_tbit(unsigned long long x)
  noexcept -> terabits
{
  return terabits{static_cast<terabits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_pbit(unsigned long long x)
  noexcept -> petabits
{
  return petabits{static_cast<petabits::rep>(x)};
}

//------------------------------------------------------------------------------

inline constexpr
auto msl::literals::bit_literals::operator""_kibit(unsigned long long x)
  noexcept -> kibibits
{
  return kibibits{static_cast<kibibits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_mibit(unsigned long long x)
  noexcept -> mebibits
{
  return mebibits{static_cast<mebibits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_gibit(unsigned long long x)
  noexcept -> gibibits
{
  return gibibits{static_cast<gibibits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_tibit(unsigned long long x)
  noexcept -> tebibits
{
  return tebibits{static_cast<tebibits::rep>(x)};
}


inline constexpr
auto msl::literals::bit_literals::operator""_pibit(unsigned long long x)
  noexcept -> pebibits
{
  return pebibits{static_cast<pebibits::rep>(x)};
}

template <typename Rep, typename Base, typename>
inline constexpr
auto msl::operator+(std::byte* p, digital_quantity<Rep,Base> offset)
  noexcept -> std::byte*
{
  return p + bytes{offset}.count();
}

template <typename Rep, typename Base, typename>
inline constexpr
auto msl::operator+(const std::byte* p, digital_quantity<Rep,Base> offset)
  noexcept -> const std::byte*
{
  return p + bytes{offset}.count();
}

template <typename Rep, typename Base, typename>
inline constexpr
auto msl::operator-(std::byte* p, digital_quantity<Rep,Base> offset)
  noexcept -> std::byte*
{
  return p - bytes{offset}.count();
}

template <typename Rep, typename Base, typename>
inline constexpr
auto msl::operator-(const std::byte* p, digital_quantity<Rep,Base> offset)
  noexcept -> const std::byte*
{
  return p - bytes{offset}.count();
}

#endif /* MSL_QUANTITIES_DIGITAL_QUANTITY_HPP */
