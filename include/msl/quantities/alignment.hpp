///////////////////////////////////////////////////////////////////////////////
/// \file alignment.hpp
///
/// \brief This header defines the semantic type `alignment`, used to represent
///        alignment boundaries.
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
#ifndef MSL_ALIGNMENT_HPP
#define MSL_ALIGNMENT_HPP

#if defined(_MSC_VER)
# pragma once
#endif // defined(_MSC_VER)

#include "msl/quantities/digital_quantity.hpp"
#include "msl/utilities/assert.hpp"
#include "msl/utilities/source_location.hpp"
#include "msl/utilities/intrinsics.hpp"

#include <cstddef> // std::max_align_t
#include <bit>     // std::has_single_bit
#include <compare> // operator<=> types
#include <stdexcept> // std::runtime_error

namespace msl {

  //===========================================================================
  // class : alignment_contract
  //===========================================================================

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The global contract handler for alignment values
  ///
  /// By default this throws a violation exception.
  //////////////////////////////////////////////////////////////////////////////
  struct alignment_contract final
  {
    //--------------------------------------------------------------------------
    // Public Member Types
    //--------------------------------------------------------------------------

    class violation;

    /// \brief The handler function that will be executed on a contract
    ///        violation.
    ///
    /// \note Contract violation handlers must be non-returning. They must
    ///       either throw an exception or terminate the program.
    using violation_handler = auto(*)(bytes, source_location) -> void;

    //--------------------------------------------------------------------------
    // Contract
    //--------------------------------------------------------------------------

    /// \brief Triggers the violation handler for the specified request size
    ///
    /// \param request the bytes requested
    [[noreturn]]
    static auto violate(bytes request, source_location where = source_location::current()) -> void;

    /// \brief Gets the currently assigned violation handler
    ///
    /// \return the currently assigned violation handler
    [[nodiscard]]
    static auto get_violation_handler() noexcept -> violation_handler;

    /// \brief Sets the currently assigned violation handler
    ///
    /// \note If \p handler is `nullptr`, the currently assigned handler is
    ///       returned instead
    ///
    /// \param handler the handler to set
    /// \return the currently assigned violation handler
    static auto set_violation_handler(violation_handler handler) noexcept -> violation_handler;
  };

  //===========================================================================
  // class : alignment_contract::violation
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief An exception for violation of the alignment contract
  ///
  /// This is thrown by the default alignment contact handler.
  /////////////////////////////////////////////////////////////////////////////
  class alignment_contract::violation : public std::logic_error
  {
    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    violation(bytes request, source_location where);
    violation(const violation& other) = default;
    violation(violation&& other) = default;

    //-------------------------------------------------------------------------

    auto operator=(const violation& other) -> violation& = default;
    auto operator=(violation&& other) -> violation& = default;

    //-------------------------------------------------------------------------
    // Accessors
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the source location of the contract violation
    ///
    /// \return the location
    [[nodiscard]]
    auto where() const noexcept -> const source_location&;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    source_location m_where;
  };

  //===========================================================================
  // class : alignment
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A literal type representing alignment boundaries.
  ///
  /// This type may only take on the state of valid alignments, which must be
  /// powers of two.
  ///
  /// ### Example
  ///
  /// Basic Use:
  /// ```cpp
  /// // over-aligned resource request
  /// const auto align = alignment::at_boundary<64>();
  ///
  /// const auto p = resource.request(size_bytes, align);
  /// ```
  /////////////////////////////////////////////////////////////////////////////
  class alignment
  {
    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    /// The underlying representation of the alignment boundary is in byte units
    using element_type = digital_quantity<decltype(alignof(int))>;

    //-------------------------------------------------------------------------
    // Static Factories
    //-------------------------------------------------------------------------
  public:

    /// \brief Returns the minimum alignment of any fundamental type
    ///
    /// \return the minimal valid alignment of any fundamental type
    [[nodiscard]]
    static consteval auto min_default() noexcept -> alignment;

    /// \brief Returns the maximum alignment of any fundamental type
    ///
    /// \return the max alignment of any fundamental type
    [[nodiscard]]
    static consteval auto max_default() noexcept -> alignment;

    /// \brief Returns the alignment of the specified type
    ///
    /// \tparam T the type to check
    /// \return the alignment of the type
    template <typename T>
    [[nodiscard]]
    static consteval auto of() noexcept -> alignment;

    /// \{
    /// \brief Makes an alignment object aligned to the \p n byte boundary
    ///
    /// On failure, this function will call the `alignment_contract` violation
    /// handler
    ///
    /// \pre \p n must be a power-of-two
    /// \param n the byte boundary to align to
    [[nodiscard]]
    static constexpr auto at_boundary(element_type n,
                                      source_location where = source_location::current())
      -> alignment;
    [[nodiscard]]
    static constexpr auto at_boundary(element_type::rep n,
                                      source_location where = source_location::current())
      -> alignment;
    /// \}

    /// \brief Makes an alignment object aligned to the \p N byte boundary
    ///
    /// Rather than calling the contract handler on failure, this will
    /// statically assert on failure.
    ///
    /// \pre \p N must be a power-of-two
    /// \param N the byte boundary to align to
    template <std::size_t N>
    [[nodiscard]]
    static consteval auto at_boundary() noexcept -> alignment;

    [[nodiscard]]
    static constexpr auto assume_at_boundary(element_type n) noexcept -> alignment;
    [[nodiscard]]
    static constexpr auto assume_at_boundary(element_type::rep n) noexcept -> alignment;

    //-------------------------------------------------------------------------
    // Validity
    //-------------------------------------------------------------------------
  public:

    /// \{
    /// \brief Queries whether \p n is a valid alignment boundary
    ///
    /// \return true if valid
    static constexpr auto is_valid(element_type b) noexcept -> bool;
    static constexpr auto is_valid(element_type::rep n) noexcept -> bool;
    /// \}

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs an alignment value with the minimum valid alignment
    ///
    /// \note This does not create a simple '0' alignment, since 0 falls
    ///       outside the range of a valid alignment value.
    consteval alignment() noexcept;

    /// \brief Copies the contents of \p other
    ///
    /// \param other the other alignment to copy
    alignment(const alignment& other) = default;

    //-------------------------------------------------------------------------

    /// \brief Copies the contents of \p other
    ///
    /// \param other the other alignment to copy
    /// \return reference to `(*this)`
    auto operator=(const alignment& other) -> alignment& = default;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the alignment boundary as a raw integral value
    ///
    /// \return the alignment boundary
    constexpr auto value() const noexcept -> element_type;

    //--------------------------------------------------------------------------
    // Comparison Operators
    //--------------------------------------------------------------------------
  public:

    auto operator==(const alignment& other) const noexcept -> bool = default;
    auto operator<=>(const alignment& other) const noexcept = default;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    element_type m_boundary;

    //-------------------------------------------------------------------------
    // Private Constructors
    //-------------------------------------------------------------------------
  private:

    /// \brief Constructs an alignment at the specified \p boundary
    ///
    /// \pre \p boundary is a power-of-two
    /// \param boundary the byte alignment
    constexpr explicit alignment(element_type boundary) noexcept;
  };

  static_assert(std::is_trivially_destructible_v<alignment>);
  static_assert(std::is_trivially_copyable_v<alignment>);

  //===========================================================================
  // non-member functions : class : alignment
  //===========================================================================

  //---------------------------------------------------------------------------
  // Utilities
  //---------------------------------------------------------------------------

  /// \brief Determines the alignment of the specified type T and returns it
  ///        as an `alignment` object
  ///
  /// \tparam T the type to determine the alignment from
  /// \return the `alignment` object
  template <typename T>
  [[nodiscard]]
  consteval auto alignment_of() noexcept -> alignment;

} // namespace msl

//=============================================================================
// class : alignment_contract::violation
//=============================================================================

MSL_FORCE_INLINE
auto msl::alignment_contract::violation::where()
  const noexcept -> const source_location&
{
  return m_where;
}

//=============================================================================
// class : alignment
//=============================================================================

//-----------------------------------------------------------------------------
// Private Constructors
//-----------------------------------------------------------------------------

// This constructor is intentionally defined out-of-order, since some compilers
// complain that the consteval function is not constant evaluated otherwise
inline constexpr
msl::alignment::alignment(element_type boundary)
  noexcept
  : m_boundary{boundary}
{

}

//-----------------------------------------------------------------------------
// Static Factories
//-----------------------------------------------------------------------------

inline consteval
auto msl::alignment::min_default()
  noexcept -> alignment
{
  return alignment{
    element_type{alignof(char)}
  };
}

inline consteval
auto msl::alignment::max_default()
  noexcept -> alignment
{
  return alignment{
    element_type{alignof(std::max_align_t)}
  };
}

template <typename T>
inline consteval
auto msl::alignment::of()
  noexcept -> alignment
{
  return alignment{
    element_type{alignof(T)}
  };
}

inline constexpr
auto msl::alignment::at_boundary(element_type n, source_location where)
  -> alignment
{
  if (!is_valid(n)) MSL_UNLIKELY {
    alignment_contract::violate(n, where);
  }

  return alignment{n};
}

inline constexpr
auto msl::alignment::at_boundary(element_type::rep n, source_location where)
  -> alignment
{
  return at_boundary(element_type{n});
}

template <std::size_t N>
inline consteval
auto msl::alignment::at_boundary()
  noexcept -> alignment
{
  static_assert(is_valid(N), "Alignment must be power-of-two");

  return alignment{
    element_type{N}
  };
}

inline constexpr
auto msl::alignment::assume_at_boundary(element_type n)
  noexcept -> alignment
{
  MSL_ASSERT(is_valid(n), "Alignment must be a power of two");

  return alignment{n};
}

inline constexpr
auto msl::alignment::assume_at_boundary(element_type::rep n)
  noexcept -> alignment
{
  return at_boundary(element_type{n});
}

//-----------------------------------------------------------------------------
// Validity
//-----------------------------------------------------------------------------

inline constexpr
auto msl::alignment::is_valid(element_type b)
  noexcept -> bool
{
  return is_valid(b.count());
}

inline constexpr
auto msl::alignment::is_valid(element_type::rep n)
  noexcept -> bool
{
  return std::has_single_bit(n);
}

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

inline consteval
msl::alignment::alignment()
  noexcept
  : alignment{min_default()}
{

}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

inline constexpr
auto msl::alignment::value()
  const noexcept -> element_type
{
  return m_boundary;
}

//=============================================================================
// non-member functions : class : alignment
//=============================================================================

//---------------------------------------------------------------------------
// Utilities
//---------------------------------------------------------------------------

template <typename T>
inline consteval
auto msl::alignment_of()
  noexcept -> alignment
{
  return alignment::of<T>();
}

#endif /* MSL_ALIGNMENT_HPP */
