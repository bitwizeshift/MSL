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
#ifndef MSL_BLOCKS_MEMORY_BLOCK_HPP
#define MSL_BLOCKS_MEMORY_BLOCK_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include "msl/quantities/digital_quantity.hpp"
#include "msl/pointers/not_null.hpp"
#include "msl/utilities/assert.hpp"
#include "msl/utilities/intrinsics.hpp"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t, std::byte
#include <ranges>     // std::ranges::begin, etc
#include <functional> // std::less_equal

namespace msl {
  struct nullblock_t{};

  inline constexpr auto nullblock = nullblock_t{};

  /////////////////////////////////////////////////////////////////////////////
  /// \brief Wrapper around a block of memory, containing both the size
  ///        and the address of the memory block.
  ///
  /// This is effectively a "raw" region of memory without any semantic markup,
  /// unlike cells.
  ///
  /// A null memory block does not necessarily contain null pointers; but
  /// rather will contain no valid addresses
  /////////////////////////////////////////////////////////////////////////////
  class memory_block
  {
    //-------------------------------------------------------------------------
    // Static Factories
    //-------------------------------------------------------------------------
  public:

    /// \{
    /// \brief Constructs this memory block from a non-null pointer range
    ///
    /// \note If `start == end`, the block is considered a "null block" by
    ///       having no accessible data.
    ///
    /// \param start the start of the range
    /// \param end one past the end of the range
    /// \return a memory block
    template <typename Iterator, typename Sentinel>
    static constexpr auto from_range(Iterator start, Sentinel end)
      noexcept -> memory_block
      requires(std::contiguous_iterator<Iterator> && std::sentinel_for<Sentinel,Iterator>);
    template <typename Iterator>
    static constexpr auto from_range(Iterator start, Iterator end)
      noexcept -> memory_block
      requires(std::contiguous_iterator<Iterator>);
    /// \}

    /// \brief Constructs this memory block from a contiguous range of bytes
    ///
    /// \note If `begin(range) == end(range)`, the block is considered a
    ///       "null block" by having no accessible data.
    ///
    /// \param range the range to convert to a block
    /// \return a memory block
    template <typename Range>
    static constexpr auto from_range(Range& range)
      noexcept -> memory_block
      requires(std::ranges::contiguous_range<Range>);

    /// \brief Constructs this memory block from a pointer and length
    ///
    /// \note A zero-length block is considered a null block
    ///
    /// \param p the pointer
    /// \param length the length, in bytes
    /// \return the memory block
    static constexpr auto from_pointer_and_length(
      not_null<std::byte*> p,
      bytes length
    ) noexcept -> memory_block;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    /// \{
    /// \brief Constructs a null memory block
    consteval memory_block() noexcept;
    consteval memory_block(nullblock_t) noexcept;
    /// \}

    memory_block(const memory_block&) = default;

    //-------------------------------------------------------------------------

    auto operator=(const memory_block& other) -> memory_block& = default;

    //-------------------------------------------------------------------------
    // Capacity
    //-------------------------------------------------------------------------
  public:

    /// \brief Queries whether this block has an empty size
    ///
    /// \return `true` if this block is empty
    constexpr auto empty() const noexcept -> bool;

    /// \brief Queries the size of this block in bytes
    ///
    /// \return the size in bytes
    constexpr auto size() const noexcept -> bytes;

    //-------------------------------------------------------------------------
    // Queries
    //-------------------------------------------------------------------------
  public:

    /// \brief Queries whether a given pointer exists in this memory block
    ///
    /// \todo Verify the validity of this implementation
    ///
    /// \param p the pointer to check
    /// \return `true` if it exists
    constexpr auto contains(const std::byte* p) const noexcept -> bool;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Retrieves a pointer to the start of this data block
    ///
    /// \return the pointer to the data block
    constexpr auto data() const noexcept -> std::byte*;

    // Note: this class defines `start_address` and `end_address` rather than
    //       typical begin/end for iterators to prevent this class from being
    //       used as a generic container/buffer of bytes. There should be very
    //       few reasons to iterate memory blocks.

    /// \brief Retrieves the start address of this memory block
    ///
    /// This is the same as `data()`
    ///
    /// \return the pointer to the data block
    constexpr auto start_address() const noexcept -> std::byte*;

    /// \brief Retrieves the end address of this memory block
    ///
    /// \note That the end address is 1 past the end of the memory block
    ///
    /// \return the pointer to the end of the data block
    constexpr auto end_address() const noexcept -> std::byte*;

    //-------------------------------------------------------------------------
    // Modifiers
    //-------------------------------------------------------------------------
  public:

    /// \brief Fills the contents of this memory region with the specified byte
    ///
    /// This is used to effectively "memset" internal storage
    ///
    /// \param value the byte to fill the region with
    constexpr auto fill(std::byte value) noexcept -> void;

    /// \brief Fills the contents of this memory region by repeating the bytes
    ///        specified in the range `[first, last)`
    ///
    /// \param first the start of the range (must be at least a forward iterator)
    /// \param last the end of the range
    template <typename ForwardIt, typename Sentinel>
    constexpr auto fill(ForwardIt first, Sentinel last) noexcept -> void
      requires(std::forward_iterator<ForwardIt> && std::sentinel_for<Sentinel,ForwardIt>);

    /// \brief Fills the contents of this memroy region by repeating the bytes
    ///        specified in the \p range
    ///
    /// \param range the sequence of values to repeat
    template <typename Range>
    constexpr auto fill(const Range& range) noexcept -> void
      requires(std::ranges::range<Range>);

    //-------------------------------------------------------------------------
    // Comparisons
    //-------------------------------------------------------------------------
  public:

    auto operator==(const memory_block& other) const -> bool = default;
    constexpr auto operator==(nullblock_t) const -> bool;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    std::byte* m_begin;
    std::byte* m_end;

    //-------------------------------------------------------------------------
    // Private Constructors
    //-------------------------------------------------------------------------
  private:

    constexpr memory_block(std::byte* begin, std::byte* end) noexcept;
  };

} // namespace msl

//-----------------------------------------------------------------------------
// Static Factories
//-----------------------------------------------------------------------------

template <typename Iterator, typename Sentinel>
MSL_FORCE_INLINE constexpr
auto msl::memory_block::from_range(Iterator start, Sentinel end)
  noexcept -> memory_block
  requires(std::contiguous_iterator<Iterator> && std::sentinel_for<Sentinel,Iterator>)
{
  // 'std::to_address' is only guaranteed to work on iterators, not sentinels.
  //
  // It's theoretically possible to define a sentinel that does not provide an
  // explicitly retrievable address, and so the only way to properly determine
  // the end pointer is to compute the distance and add it to the start pointer
  //
  // Whether this is a realistic thing to encounter is uncertain
  const auto distance = std::ranges::distance(start, end);
  const auto p = std::to_address(start);

  return memory_block{p, p + distance};
}

template <typename Iterator>
MSL_FORCE_INLINE constexpr
auto msl::memory_block::from_range(Iterator start, Iterator end)
  noexcept -> memory_block
  requires(std::contiguous_iterator<Iterator>)
{
  return memory_block{std::to_address(start), std::to_address(end)};
}

template <typename Range>
MSL_FORCE_INLINE constexpr
auto msl::memory_block::from_range(Range& range)
  noexcept -> memory_block
  requires(std::ranges::contiguous_range<Range>)
{
  return from_range(std::ranges::begin(range), std::ranges::end(range));
}


MSL_FORCE_INLINE constexpr
auto msl::memory_block::from_pointer_and_length(
  not_null<std::byte*> p,
  bytes length
) noexcept -> memory_block
{
  return from_range(p.as_nullable(), p.as_nullable() + length.count());
}

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

inline consteval
msl::memory_block::memory_block()
  noexcept
  : memory_block{nullblock}
{

}

inline consteval
msl::memory_block::memory_block(nullblock_t)
  noexcept
  : m_begin{nullptr},
    m_end{nullptr}
{

}

//-----------------------------------------------------------------------------
// Capacity
//-----------------------------------------------------------------------------

inline constexpr
auto msl::memory_block::empty()
  const noexcept -> bool
{
  return m_begin == m_end;
}

inline constexpr
auto msl::memory_block::size()
  const noexcept -> bytes
{
  return bytes{static_cast<std::size_t>(m_end - m_begin)};
}

//-----------------------------------------------------------------------------
// Queries
//-----------------------------------------------------------------------------

inline constexpr
auto msl::memory_block::contains(const std::byte* p)
  const noexcept -> bool
{
  // Comparing pointers from possibly different sub-objects is UB; however,
  // the various functional comparators define a total-ordering which can at
  // least allow for a simple containment check to effectively operate.
  //
  // TODO(bitwizeshift): determine whether this is guaranteed to be correct.
  constexpr auto compare = std::less_equal<const std::byte*>{};

  return compare(m_begin, p) && compare(p, m_end);
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

inline constexpr
auto msl::memory_block::data()
  const noexcept -> std::byte*
{
  return m_begin;
}

inline constexpr
auto msl::memory_block::start_address()
  const noexcept -> std::byte*
{
  return m_begin;
}

inline constexpr
auto msl::memory_block::end_address()
  const noexcept -> std::byte*
{
  return m_end;
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

inline constexpr
auto msl::memory_block::fill(std::byte value)
  noexcept -> void
{
  std::fill(m_begin, m_end, value);
}

template <typename ForwardIt, typename Sentinel>
inline constexpr
auto msl::memory_block::fill(ForwardIt first, Sentinel last)
  noexcept -> void
  requires(std::forward_iterator<ForwardIt> && std::sentinel_for<Sentinel,ForwardIt>)
{
  auto it = first;
  const auto end = end_address();

  for (auto jt = start_address(); jt != end_address(); ++jt) {
    *jt = *it;

    // wrap-around logic
    ++it;
    if (it == last) {
      it = first;
    }
  }
}

template <typename Range>
inline constexpr
auto msl::memory_block::fill(const Range& range)
  noexcept -> void
  requires(std::ranges::range<Range>)
{
  fill(std::ranges::begin(range), std::ranges::end(range));
}

MSL_FORCE_INLINE constexpr
auto msl::memory_block::operator==(nullblock_t)
  const -> bool
{
  return empty();
}

//-----------------------------------------------------------------------------
// Private Constructors
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE constexpr
msl::memory_block::memory_block(std::byte* begin, std::byte* end)
  noexcept
  : m_begin{begin},
    m_end{end}
{

}

#endif /* MSL_BLOCKS_MEMORY_BLOCK_HPP */