///////////////////////////////////////////////////////////////////////////////
/// \file digital_quantity.hpp
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
#ifndef MSL_UTILITIES_DIGITAL_QUANTITY_HPP
#define MSL_UTILITIES_DIGITAL_QUANTITY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <ratio>   // std::ratio
#include <cstdint> // std::uint64_t, etc
#include <climits> // CHAR_BIT
#include <cstddef> // std::byte
#include <type_traits> // std::enable_if, std::common_type
#include <limits> // std::numeric_limits

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
    static constexpr digital_quantity zero() noexcept;

    /// \brief Returns the minimum digital_quantity this can support
    ///
    /// \return the digital_quantity
    static constexpr digital_quantity min() noexcept;

    /// \brief Returns the maximum digital_quantity this can support
    ///
    /// \return the digital_quantity
    static constexpr digital_quantity max() noexcept;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Default-constructs this digital_quantity
    constexpr digital_quantity() noexcept = default;

    /// \brief Constructs this digital_quantity from an underlying representation
    ///
    /// \param r the value
    template <typename URep,
              typename = std::enable_if_t<std::is_convertible<URep,Rep>::value>>
    constexpr explicit digital_quantity(const URep& r) noexcept;

    /// \brief Converts \p other to this digital_quantity
    ///
    /// This constructor is only enabled if there will be no
    /// loss-of-precision. To have a lossy conversion, see digital_quantity_cast
    ///
    /// \param other the other digital_quantity to convert
    template <typename URep, typename UBase,
              typename = std::enable_if_t<std::ratio_divide<UBase, Base>::den == 1 &&
                                          std::is_convertible<URep,Rep>::value>>
    constexpr digital_quantity(digital_quantity<URep,UBase> other) noexcept;

    /// \brief Copy-constructs this \p digital_quantity from \p other
    ///
    /// \param other the other digital_quantity to copy
    constexpr digital_quantity(const digital_quantity& other) noexcept = default;

    //-------------------------------------------------------------------------

    /// \brief Assigns the contents of \p other to this
    ///
    /// \param other the other digital_quantity to copy
    /// \return reference to \c (*this)
    digital_quantity& operator=(const digital_quantity& other) noexcept = default;

    //-------------------------------------------------------------------------
    // Arithmetic Operators
    //-------------------------------------------------------------------------
  public:
    constexpr std::common_type_t<digital_quantity> operator+() const noexcept;
    constexpr std::common_type_t<digital_quantity> operator-() const noexcept;

    constexpr digital_quantity& operator++() noexcept;
    constexpr digital_quantity operator++(int) noexcept;
    constexpr digital_quantity& operator--() noexcept;
    constexpr digital_quantity operator--(int) noexcept;

    constexpr digital_quantity& operator+=(const digital_quantity& rhs) noexcept;
    constexpr digital_quantity& operator-=(const digital_quantity& rhs) noexcept;
    constexpr digital_quantity& operator*=(const rep& r) noexcept;
    constexpr digital_quantity& operator/=(const rep& r) noexcept;
    constexpr digital_quantity& operator%=(const rep& r) noexcept;
    constexpr digital_quantity& operator%=(const digital_quantity& rhs) noexcept;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the total count of the digital_quantity
    ///
    /// \return the count
    constexpr Rep count() const noexcept;

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

  // should 'bytes' be 'std::size_t'?
  using bytes     = digital_quantity<std::uint64_t>;           // at least 64 bits
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
  constexpr std::common_type_t<digital_quantity<Rep1,Base1>, digital_quantity<Rep2,Base2>>
    operator+(const digital_quantity<Rep1,Base1>& lhs,
              const digital_quantity<Rep2,Base2>& rhs) noexcept;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr std::common_type_t<digital_quantity<Rep1,Base1>, digital_quantity<Rep2,Base2>>
    operator-(const digital_quantity<Rep1,Base1>& lhs,
              const digital_quantity<Rep2,Base2>& rhs) noexcept;

  template <typename Rep1, typename Base, typename Rep2>
  constexpr digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
    operator*(const digital_quantity<Rep1,Base>& d,
              const Rep2& s) noexcept;

  template <typename Rep1, typename Rep2, typename Base>
  constexpr digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
    operator*(const Rep1& s,
              const digital_quantity<Rep2,Base>& d) noexcept;

  template <typename Rep1, typename Base, typename Rep2>
  constexpr digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
    operator/(const digital_quantity<Rep1,Base>& d,
              const Rep2& s) noexcept;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr std::common_type_t<Rep1,Rep2>
    operator/(const digital_quantity<Rep1,Base1>& lhs,
              const digital_quantity<Rep2,Base2>& rhs) noexcept;

  template <typename Rep1, typename Base, typename Rep2>
  constexpr digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
    operator%(const digital_quantity<Rep1, Base>& d,
              const Rep2& s) noexcept;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr std::common_type_t<digital_quantity<Rep1,Base1>, digital_quantity<Rep2,Base2>>
    operator%(const digital_quantity<Rep1,Base1>& lhs,
              const digital_quantity<Rep2,Base2>& rhs) noexcept;

  //---------------------------------------------------------------------------
  // Comparison Operators
  //---------------------------------------------------------------------------

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr bool operator==(const digital_quantity<Rep1, Base1>& lhs,
                            const digital_quantity<Rep2, Base2>& rhs) noexcept;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr bool operator!=(const digital_quantity<Rep1, Base1>& lhs,
                            const digital_quantity<Rep2, Base2>& rhs) noexcept;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr bool operator<(const digital_quantity<Rep1, Base1>& lhs,
                          const digital_quantity<Rep2, Base2>& rhs) noexcept;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr bool operator<=(const digital_quantity<Rep1, Base1>& lhs,
                            const digital_quantity<Rep2, Base2>& rhs) noexcept;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr bool operator>(const digital_quantity<Rep1, Base1>& lhs,
                          const digital_quantity<Rep2, Base2>& rhs) noexcept;

  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr bool operator>=(const digital_quantity<Rep1, Base1>& lhs,
                            const digital_quantity<Rep2, Base2>& rhs) noexcept;

  //---------------------------------------------------------------------------
  // Utilities
  //---------------------------------------------------------------------------

  /// \brief Converts \p from to a ToCapacity with lossy conversion
  ///
  /// This is the only way to convert types with loss of precision.
  ///
  /// \tparam ToCapacity the digital_quantity to convert to
  /// \param from the digital_quantity to convert
  /// \return the converted digital_quantity
  template <typename ToCapacity, typename FromCapacity,
            typename = std::enable_if_t<is_digital_quantity_v<ToCapacity> &&
                                        is_digital_quantity_v<FromCapacity>>>
  constexpr ToCapacity digital_quantity_cast(const FromCapacity& from);

  //---------------------------------------------------------------------------

  /// \brief Computes the absolute difference between \p lhs and \p rhs
  ///
  /// \param lhs the left digital_quantity
  /// \param rhs the right digital_quantity
  /// \return the difference between the two capacities
  template <typename Rep1, typename Base1, typename Rep2, typename Base2>
  constexpr std::common_type_t<digital_quantity<Rep1,Base1>, digital_quantity<Rep2,Base2>>
    difference(const digital_quantity<Rep1,Base1>& lhs, const digital_quantity<Rep2,Base2>& rhs)
    noexcept;

  /// \brief Rounds the specified digital_quantity down to ToCapacity
  ///
  /// \tparam ToCapacity the digital_quantity to round down to
  /// \param c the digital_quantity
  /// \return the rounded digital_quantity
  template <typename ToCapacity, typename Rep, typename Base,
            typename = std::enable_if_t<is_digital_quantity_v<ToCapacity>>>
  constexpr ToCapacity floor(const digital_quantity<Rep, Base>& c) noexcept;


  /// \brief Rounds the specified digital_quantity up to ToCapacity
  ///
  /// \tparam ToCapacity the digital_quantity to round up to
  /// \param c the digital_quantity
  /// \return the rounded digital_quantity
  template <typename ToCapacity, typename Rep, typename Base,
            typename = std::enable_if_t<is_digital_quantity_v<ToCapacity>>>
  constexpr ToCapacity ceil(const digital_quantity<Rep, Base>& c) noexcept;

  /// \brief Rounds the specified digital_quantity \p c to the nearest ToCapacity
  ///
  /// This follows the typical C++ rounding model, and behaves like std::round
  /// by always rounding away from 0.
  ///
  /// \tparam ToCapacity the digital_quantity to round to
  /// \param c the digital_quantity to round
  /// \return the rounded digital_quantity
  template <typename ToCapacity, typename Rep, typename Base,
            typename = std::enable_if_t<is_digital_quantity_v<ToCapacity>>>
  constexpr ToCapacity round(const digital_quantity<Rep, Base>& c) noexcept;

  /// \brief Gets the absolute value of the specified digital_quantity
  ///
  /// \param c the digital_quantity to get the absolute value of
  /// \return the absolute value of the digital_quantity
  template <typename Rep, typename Base>
  constexpr digital_quantity<Rep, Base> abs(const digital_quantity<Rep, Base>& c) noexcept;

  //---------------------------------------------------------------------------

  /// \brief Gets the size of the specified type, in bytes
  ///
  /// \tparam T the type to get the size of
  template <typename T>
  constexpr bytes size_of() noexcept;

  /// \brief Gets the size of the specified value, in bytes
  ///
  /// \param v the value to get the size of
  template <typename T>
  constexpr bytes size_of(const T& v) noexcept;

  //===========================================================================
  // literals : class : digital_quantity
  //===========================================================================

  inline namespace literals {
    //-------------------------------------------------------------------------
    // Byte Literals
    //-------------------------------------------------------------------------

    inline namespace byte_literals {

      constexpr bytes operator""_b(unsigned long long x) noexcept;
      constexpr kilobytes operator""_kb(unsigned long long x) noexcept;
      constexpr megabytes operator""_mb(unsigned long long x) noexcept;
      constexpr gigabytes operator""_gb(unsigned long long x) noexcept;
      constexpr terabytes operator""_tb(unsigned long long x) noexcept;
      constexpr petabytes operator""_pb(unsigned long long x) noexcept;

      //-----------------------------------------------------------------------

      constexpr kibibytes operator""_kib(unsigned long long x) noexcept;
      constexpr mebibytes operator""_mib(unsigned long long x) noexcept;
      constexpr gibibytes operator""_gib(unsigned long long x) noexcept;
      constexpr tebibytes operator""_tib(unsigned long long x) noexcept;
      constexpr pebibytes operator""_pib(unsigned long long x) noexcept;

    } // namespace byte_literals

    //-------------------------------------------------------------------------
    // Bit Literals
    //-------------------------------------------------------------------------

    inline namespace bit_literals {

      constexpr bits operator""_bit(unsigned long long x) noexcept;
      constexpr kilobits operator""_kbit(unsigned long long x) noexcept;
      constexpr megabits operator""_mbit(unsigned long long x) noexcept;
      constexpr gigabits operator""_gbit(unsigned long long x) noexcept;
      constexpr terabits operator""_tbit(unsigned long long x) noexcept;
      constexpr petabits operator""_pbit(unsigned long long x) noexcept;

      //-----------------------------------------------------------------------

      constexpr kibibits operator""_kibit(unsigned long long x) noexcept;
      constexpr mebibits operator""_mibit(unsigned long long x) noexcept;
      constexpr gibibits operator""_gibit(unsigned long long x) noexcept;
      constexpr tebibits operator""_tibit(unsigned long long x) noexcept;
      constexpr pebibits operator""_pibit(unsigned long long x) noexcept;

    } // inline namespace digital_quantity_literals
  } // inline namespace literals

  //===========================================================================
  // non-member functions : alias : digital_quantity
  //===========================================================================

  //---------------------------------------------------------------------------
  // Arithmetic Operators
  //---------------------------------------------------------------------------

  template <typename Rep, typename Base,
            typename = std::enable_if_t<Base::den == 1>>
  constexpr std::byte* operator+(std::byte* p, digital_quantity<Rep,Base> offset)
    noexcept;

  template <typename Rep, typename Base,
            typename = std::enable_if_t<Base::den == 1>>
  constexpr const std::byte* operator+(const std::byte* p, digital_quantity<Rep,Base> offset)
    noexcept;

  template <typename Rep, typename Base,
            typename = std::enable_if_t<Base::den == 1>>
  constexpr std::byte* operator-(std::byte* p, digital_quantity<Rep,Base> offset)
    noexcept;

  template <typename Rep, typename Base,
            typename = std::enable_if_t<Base::den == 1>>
  constexpr const std::byte* operator-(const std::byte* p, digital_quantity<Rep,Base> offset)
    noexcept;

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
      std::ratio<
        msl::detail::gcd<Base1::num, Base2::num>::value,
        msl::detail::lcm<Base1::den, Base2::den>::value
      >
    >;
  };

} // namespace std

//-----------------------------------------------------------------------------
// Public Static Factories
//-----------------------------------------------------------------------------

template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>
  msl::digital_quantity<Rep,Base>::zero()
  noexcept
{
  return digital_quantity{0};
}

template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>
  msl::digital_quantity<Rep,Base>::min()
  noexcept
{
  return digital_quantity{std::numeric_limits<Rep>::min()};
}

template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>
  msl::digital_quantity<Rep,Base>::max()
  noexcept
{
  return digital_quantity{std::numeric_limits<Rep>::max()};
}

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

template <typename Rep, typename Base>
template <typename URep, typename>
inline constexpr msl::digital_quantity<Rep,Base>::digital_quantity(const URep& r)
  noexcept
  : m_count{static_cast<Rep>(r)}
{

}

template <typename Rep, typename Base>
template <typename URep, typename UBase, typename>
inline constexpr msl::digital_quantity<Rep,Base>
  ::digital_quantity(digital_quantity<URep,UBase> other)
  noexcept
  : m_count{static_cast<Rep>(other.count() * std::ratio_divide<UBase, Base>::num)}
{

}

//-----------------------------------------------------------------------------
// Arithmetic Operators
//-----------------------------------------------------------------------------

template <typename Rep, typename Base>
inline constexpr std::common_type_t<msl::digital_quantity<Rep,Base>>
  msl::digital_quantity<Rep,Base>::operator+()
  const noexcept
{
  using type = std::common_type_t<digital_quantity>;

  return type{m_count};
}

template <typename Rep, typename Base>
inline constexpr std::common_type_t<msl::digital_quantity<Rep,Base>>
  msl::digital_quantity<Rep,Base>::operator-()
  const noexcept
{
  using type = std::common_type_t<digital_quantity>;

  return type{-m_count};
}

template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>&
  msl::digital_quantity<Rep,Base>::operator++()
  noexcept
{
  ++m_count;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>
  msl::digital_quantity<Rep,Base>::operator++(int)
  noexcept
{
  const auto copy = (*this);
  ++m_count;
  return copy;
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>&
  msl::digital_quantity<Rep,Base>::operator--()
  noexcept
{
  --m_count;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>
  msl::digital_quantity<Rep,Base>::operator--(int)
  noexcept
{
  const auto copy = (*this);
  --m_count;
  return copy;
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>&
  msl::digital_quantity<Rep,Base>::operator+=(const digital_quantity& rhs)
  noexcept
{
  m_count += rhs.count();
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>&
  msl::digital_quantity<Rep,Base>::operator-=(const digital_quantity& rhs)
  noexcept
{
  m_count -= rhs.count();
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>&
  msl::digital_quantity<Rep,Base>::operator*=(const rep& r)
  noexcept
{
  m_count *= r;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>&
  msl::digital_quantity<Rep,Base>::operator/=(const rep& r)
  noexcept
{
  m_count /= r;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>&
  msl::digital_quantity<Rep,Base>::operator%=(const rep& r)
  noexcept
{
  m_count %= r;
  return (*this);
}


template <typename Rep, typename Base>
inline constexpr msl::digital_quantity<Rep,Base>&
  msl::digital_quantity<Rep,Base>::operator%=(const digital_quantity& rhs)
  noexcept
{
  m_count %= rhs.count();
  return (*this);
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template <typename Rep, typename Base>
inline constexpr Rep msl::digital_quantity<Rep,Base>::count()
  const noexcept
{
  return m_count;
}


//=============================================================================
// definitions : non-member functions : class : digital_quantity
//=============================================================================

//-----------------------------------------------------------------------------
// Arithmetic Operators
//-----------------------------------------------------------------------------

template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr std::common_type_t<msl::digital_quantity<Rep1,Base1>,
                                    msl::digital_quantity<Rep2,Base2>>
  msl::operator+(const digital_quantity<Rep1,Base1>& lhs,
                 const digital_quantity<Rep2,Base2>& rhs)
  noexcept
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
inline constexpr std::common_type_t<msl::digital_quantity<Rep1,Base1>,
                                    msl::digital_quantity<Rep2,Base2>>
  msl::operator-(const digital_quantity<Rep1,Base1>& lhs,
                 const digital_quantity<Rep2,Base2>& rhs)
  noexcept
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
inline constexpr msl::digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
  msl::operator*(const digital_quantity<Rep1,Base>& lhs,
                 const Rep2& rhs)
  noexcept
{
  using result_type = digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  return result_type{
    result_type{lhs}.count() * rhs
  };
}


template <typename Rep1, typename Rep2, typename Base>
inline constexpr msl::digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
  msl::operator*(const Rep1& lhs,
                 const digital_quantity<Rep2,Base>& rhs)
  noexcept
{
  using result_type = digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  return result_type{
    lhs * result_type{rhs}.count()
  };
}


template <typename Rep1, typename Base, typename Rep2>
inline constexpr msl::digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
  msl::operator/(const digital_quantity<Rep1,Base>& lhs,
                 const Rep2& rhs)
  noexcept
{
  using result_type = digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  return result_type{
    result_type{lhs}.count() / rhs
  };
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr std::common_type_t<Rep1,Rep2>
  msl::operator/(const digital_quantity<Rep1,Base1>& lhs,
                 const digital_quantity<Rep2,Base2>& rhs)
  noexcept
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
inline constexpr msl::digital_quantity<std::common_type_t<Rep1,Rep2>, Base>
  msl::operator%(const digital_quantity<Rep1, Base>& lhs,
                 const Rep2& rhs)
  noexcept
{
  using result_type = digital_quantity<std::common_type_t<Rep1,Rep2>, Base>;

  return result_type{
    result_type{lhs}.count() % rhs
  };
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr std::common_type_t<msl::digital_quantity<Rep1,Base1>,
                                    msl::digital_quantity<Rep2,Base2>>
  msl::operator%(const digital_quantity<Rep1,Base1>& lhs,
                 const digital_quantity<Rep2,Base2>& rhs)
  noexcept
{
  using result_type = std::common_type_t<
    digital_quantity<Rep1,Base1>,
    digital_quantity<Rep2,Base2>
  >;

  return result_type{
    result_type{lhs}.count() % result_type{rhs}.count()
  };
}


//-----------------------------------------------------------------------------
// Comparison Operators
//-----------------------------------------------------------------------------

template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr bool msl::operator==(const digital_quantity<Rep1, Base1>& lhs,
                                      const digital_quantity<Rep2, Base2>& rhs)
  noexcept
{
  using common_type = std::common_type_t<
    digital_quantity<Rep1, Base1>,
    digital_quantity<Rep2, Base2>
  >;

  return common_type{lhs}.count() == common_type{rhs}.count();
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr bool msl::operator!=(const digital_quantity<Rep1, Base1>& lhs,
                                      const digital_quantity<Rep2, Base2>& rhs)
  noexcept
{
  using common_type = std::common_type_t<
    digital_quantity<Rep1, Base1>,
    digital_quantity<Rep2, Base2>
  >;

  return common_type{lhs}.count() != common_type{rhs}.count();
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr bool msl::operator<(const digital_quantity<Rep1, Base1>& lhs,
                                     const digital_quantity<Rep2, Base2>& rhs)
  noexcept
{
  using common_type = std::common_type_t<
    digital_quantity<Rep1, Base1>,
    digital_quantity<Rep2, Base2>
  >;

  return common_type{lhs}.count() < common_type{rhs}.count();
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr bool msl::operator<=(const digital_quantity<Rep1, Base1>& lhs,
                                      const digital_quantity<Rep2, Base2>& rhs)
  noexcept
{
  using common_type = std::common_type_t<
    digital_quantity<Rep1, Base1>,
    digital_quantity<Rep2, Base2>
  >;

  return common_type{lhs}.count() <= common_type{rhs}.count();
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr bool msl::operator>(const digital_quantity<Rep1, Base1>& lhs,
                                     const digital_quantity<Rep2, Base2>& rhs)
  noexcept
{
  using common_type = std::common_type_t<
    digital_quantity<Rep1, Base1>,
    digital_quantity<Rep2, Base2>
  >;

  return common_type{lhs}.count() > common_type{rhs}.count();
}


template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr bool msl::operator>=(const digital_quantity<Rep1, Base1>& lhs,
                                      const digital_quantity<Rep2, Base2>& rhs)
  noexcept
{
  using common_type = std::common_type_t<
    digital_quantity<Rep1, Base1>,
    digital_quantity<Rep2, Base2>
  >;

  return common_type{lhs}.count() >= common_type{rhs}.count();
}

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

template <typename ToCapacity, typename FromCapacity, typename>
inline constexpr ToCapacity msl::digital_quantity_cast(const FromCapacity& from)
{
  using from_base = typename FromCapacity::base;
  using to_base = typename ToCapacity::base;
  using to_rep = typename ToCapacity::rep;
  using result = std::ratio_divide<from_base, to_base>;

  const auto num = static_cast<to_rep>(from.count() * result::num);
  const auto den = static_cast<to_rep>(result::den);

  return ToCapacity{num / den};
}

//---------------------------------------------------------------------------

template <typename Rep1, typename Base1, typename Rep2, typename Base2>
inline constexpr std::common_type_t<msl::digital_quantity<Rep1,Base1>,
                                    msl::digital_quantity<Rep2,Base2>>
  msl::difference(const digital_quantity<Rep1,Base1>& lhs,
                          const digital_quantity<Rep2,Base2>& rhs)
  noexcept
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


template <typename ToCapacity, typename Rep, typename Base, typename>
inline constexpr ToCapacity msl::floor(const digital_quantity<Rep, Base>& c)
  noexcept
{
  const auto to = digital_quantity_cast<ToCapacity>(c);
  if (to > c) {
    return to - ToCapacity{1};
  }
  return to;
}


template <typename ToCapacity, typename Rep, typename Base, typename>
inline constexpr ToCapacity msl::ceil(const digital_quantity<Rep, Base>& c)
  noexcept
{
  const auto to = digital_quantity_cast<ToCapacity>(c);
  if (to < c) {
    return to + ToCapacity{1};
  }
  return to;
}


template <typename ToCapacity, typename Rep, typename Base, typename>
inline constexpr ToCapacity msl::round(const digital_quantity<Rep, Base>& c)
  noexcept
{
  const auto lower = floor<ToCapacity>(c);
  const auto upper = lower + ToCapacity{1};

  const auto lower_diff = difference(c, lower);
  const auto upper_diff = difference(upper, c);

  // Differences are the same -- means either 'c' is already rounded,
  // or that it's dead-center -- which should be rounded up to behave like
  // the standard rounding that C++ offers (0.5 -> 1, -0.5 -> -1)
  if (lower_diff == upper_diff) {
    if (upper > ToCapacity::zero()) {
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
inline constexpr msl::digital_quantity<Rep, Base>
  msl::abs(const digital_quantity<Rep, Base>& c)
  noexcept
{
  return c >= digital_quantity<Rep, Base>::zero() ? c : -c;
}

//-----------------------------------------------------------------------------

template <typename T>
inline constexpr msl::bytes msl::size_of()
  noexcept
{
  return bytes{sizeof(T)};
}


template <typename T>
inline constexpr msl::bytes msl::size_of(const T& v)
  noexcept
{
  return bytes{sizeof(v)};
}

//=============================================================================
// literals : class : digital_quantity
//=============================================================================

//-----------------------------------------------------------------------------
// Byte Literals
//-----------------------------------------------------------------------------

inline constexpr msl::bytes
  msl::literals::byte_literals::operator""_b(unsigned long long x)
  noexcept
{
  return bytes{static_cast<bytes::rep>(x)};
}


inline constexpr msl::kilobytes
  msl::literals::byte_literals::operator""_kb(unsigned long long x)
  noexcept
{
  return kilobytes{static_cast<kilobytes::rep>(x)};
}


inline constexpr msl::megabytes
  msl::literals::byte_literals::operator""_mb(unsigned long long x)
  noexcept
{
  return megabytes{static_cast<megabytes::rep>(x)};
}


inline constexpr msl::gigabytes
  msl::literals::byte_literals::operator""_gb(unsigned long long x)
  noexcept
{
  return gigabytes{static_cast<gigabytes::rep>(x)};
}


inline constexpr msl::terabytes
  msl::literals::byte_literals::operator""_tb(unsigned long long x)
  noexcept
{
  return terabytes{static_cast<terabytes::rep>(x)};
}


inline constexpr msl::petabytes
  msl::literals::byte_literals::operator""_pb(unsigned long long x)
  noexcept
{
  return petabytes{static_cast<petabytes::rep>(x)};
}

//-----------------------------------------------------------------------------

inline constexpr msl::kibibytes
  msl::literals::byte_literals::operator""_kib(unsigned long long x)
  noexcept
{
  return kibibytes{static_cast<kibibytes::rep>(x)};
}


inline constexpr msl::mebibytes
  msl::literals::byte_literals::operator""_mib(unsigned long long x)
  noexcept
{
  return mebibytes{static_cast<mebibytes::rep>(x)};
}


inline constexpr msl::gibibytes
  msl::literals::byte_literals::operator""_gib(unsigned long long x)
  noexcept
{
  return gibibytes{static_cast<gibibytes::rep>(x)};
}


inline constexpr msl::tebibytes
  msl::literals::byte_literals::operator""_tib(unsigned long long x)
  noexcept
{
  return tebibytes{static_cast<tebibytes::rep>(x)};
}


inline constexpr msl::pebibytes
  msl::literals::byte_literals::operator""_pib(unsigned long long x)
  noexcept
{
  return pebibytes{static_cast<pebibytes::rep>(x)};
}

//-----------------------------------------------------------------------------
// Bit Literals
//-----------------------------------------------------------------------------

inline constexpr msl::bits
  msl::literals::bit_literals::operator""_bit(unsigned long long x)
  noexcept
{
  return bits{static_cast<bits::rep>(x)};
}


inline constexpr msl::kilobits
  msl::literals::bit_literals::operator""_kbit(unsigned long long x)
  noexcept
{
  return kilobits{static_cast<kilobits::rep>(x)};
}


inline constexpr msl::megabits
  msl::literals::bit_literals::operator""_mbit(unsigned long long x)
  noexcept
{
  return megabits{static_cast<megabits::rep>(x)};
}


inline constexpr msl::gigabits
  msl::literals::bit_literals::operator""_gbit(unsigned long long x)
  noexcept
{
  return gigabits{static_cast<gigabits::rep>(x)};
}


inline constexpr msl::terabits
  msl::literals::bit_literals::operator""_tbit(unsigned long long x)
  noexcept
{
  return terabits{static_cast<terabits::rep>(x)};
}


inline constexpr msl::petabits
  msl::literals::bit_literals::operator""_pbit(unsigned long long x)
  noexcept
{
  return petabits{static_cast<petabits::rep>(x)};
}

//-----------------------------------------------------------------------------

inline constexpr msl::kibibits
  msl::literals::bit_literals::operator""_kibit(unsigned long long x)
  noexcept
{
  return kibibits{static_cast<kibibits::rep>(x)};
}


inline constexpr msl::mebibits
  msl::literals::bit_literals::operator""_mibit(unsigned long long x)
  noexcept
{
  return mebibits{static_cast<mebibits::rep>(x)};
}


inline constexpr msl::gibibits
  msl::literals::bit_literals::operator""_gibit(unsigned long long x)
  noexcept
{
  return gibibits{static_cast<gibibits::rep>(x)};
}


inline constexpr msl::tebibits
  msl::literals::bit_literals::operator""_tibit(unsigned long long x)
  noexcept
{
  return tebibits{static_cast<tebibits::rep>(x)};
}


inline constexpr msl::pebibits
  msl::literals::bit_literals::operator""_pibit(unsigned long long x)
  noexcept
{
  return pebibits{static_cast<pebibits::rep>(x)};
}

template <typename Rep, typename Base, typename>
inline constexpr std::byte*
  msl::operator+(std::byte* p, digital_quantity<Rep,Base> offset)
  noexcept
{
  const auto b = bytes{offset};
  return p + b.count();
}

template <typename Rep, typename Base, typename>
inline constexpr const std::byte*
  msl::operator+(const std::byte* p, digital_quantity<Rep,Base> offset)
  noexcept
{
  const auto b = bytes{offset};
  return p + b.count();
}


template <typename Rep, typename Base, typename>
inline constexpr std::byte*
  msl::operator-(std::byte* p, digital_quantity<Rep,Base> offset)
  noexcept
{
  const auto b = bytes{offset};
  return p - b.count();
}

template <typename Rep, typename Base, typename>
inline constexpr const std::byte*
  msl::operator-(const std::byte* p, digital_quantity<Rep,Base> offset)
  noexcept
{
  const auto b = bytes{offset};
  return p - b.count();
}

#endif /* MSL_UTILITIES_DIGITAL_QUANTITY_HPP */
