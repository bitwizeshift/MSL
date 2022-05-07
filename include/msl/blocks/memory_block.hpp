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
#include "msl/quantities/quantity.hpp"
#include "msl/pointers/not_null.hpp"
#include "msl/utilities/assert.hpp"
#include "msl/utilities/intrinsics.hpp"

#include <algorithm>  // std::fill
#include <cstddef>    // std::size_t, std::byte
#include <ranges>     // std::ranges::begin, etc
#include <functional> // std::less_equal
#include <initializer_list> // std::initializer_list

namespace msl {
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
      -> memory_block
      requires(std::contiguous_iterator<Iterator> && std::sentinel_for<Sentinel,Iterator>);
    template <typename Iterator>
    static constexpr auto from_range(Iterator start, Iterator end)
      -> memory_block
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
      -> memory_block;

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
    constexpr auto contains(not_null<const std::byte*> p) const noexcept -> bool;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Retrieves a pointer to the start of this data block
    ///
    /// \return the pointer to the data block
    constexpr auto data() const noexcept -> not_null<std::byte*>;

    // Note: this class defines `start_address` and `end_address` rather than
    //       typical begin/end for iterators to prevent this class from being
    //       used as a generic container/buffer of bytes. There should be very
    //       few reasons to iterate memory blocks.

    /// \brief Retrieves the start address of this memory block
    ///
    /// This is the same as `data()`
    ///
    /// \return the pointer to the data block
    constexpr auto start_address() const noexcept -> not_null<std::byte*>;

    /// \brief Retrieves the end address of this memory block
    ///
    /// \note That the end address is 1 past the end of the memory block
    ///
    /// \return the pointer to the end of the data block
    constexpr auto end_address() const noexcept -> not_null<std::byte*>;

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

    /// \brief Fills the content of this memory region by repeating the bytes
    ///        in the initializer list of entries
    ///
    /// \param ilist the initializer list of entries
    constexpr auto fill(std::initializer_list<std::byte> ilist) noexcept -> void;

    //-------------------------------------------------------------------------
    // Comparisons
    //-------------------------------------------------------------------------
  public:

    auto operator==(const memory_block& other) const -> bool = default;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    not_null<std::byte*> m_begin;
    not_null<std::byte*> m_end;

    //-------------------------------------------------------------------------
    // Private Constructors
    //-------------------------------------------------------------------------
  private:

    constexpr memory_block(not_null<std::byte*> begin, not_null<std::byte*> end) noexcept;
  };

  //---------------------------------------------------------------------------
  // Traversal
  //---------------------------------------------------------------------------

  /// \brief Computes what the next contiguously adjacent block is
  ///
  /// \note This function is only legal if used on memory blocks that are
  ///       contiguous in memory. If there is no validly addressable block after
  ///       \p b , then this function will result in undefined behavior
  ///
  /// \param b the block
  /// \param n the number of blocks after
  /// \return the memory block
  constexpr auto next_block(memory_block b, uquantity<memory_block> n = 1)
    noexcept -> memory_block;

  /// \brief Computes what the previous contiguously adjacent block is
  ///
  /// \note This function is only legal if used on memory blocks that are
  ///       contiguous in memory. If there is no validly addressable block after
  ///       \p b , then this function will result in undefined behavior
  ///
  /// \param b the block
  /// \param n the number of blocks after
  /// \return the memory block
  constexpr auto previous_block(memory_block b, uquantity<memory_block> n = 1)
    noexcept -> memory_block;

  //===========================================================================
  // class : memory_block_order
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A comparator type to provide a total ordering of memory blocks
  ///
  /// This is necessary for if a memory block were to be used as the key in a
  /// map, for example.
  /////////////////////////////////////////////////////////////////////////////
  struct memory_block_order
  {
    constexpr auto operator()(const memory_block& lhs, const memory_block& rhs)
      const noexcept -> bool;
  };

} // namespace msl

//-----------------------------------------------------------------------------
// Static Factories
//-----------------------------------------------------------------------------

template <typename Iterator, typename Sentinel>
MSL_FORCE_INLINE constexpr
auto msl::memory_block::from_range(Iterator start, Sentinel end)
  -> memory_block
  requires(std::contiguous_iterator<Iterator> && std::sentinel_for<Sentinel,Iterator>)
{
  // 'std::to_address' is only guaranteed to work on iterators, not sentinels.
  //
  // It's theoretically possible to define a sentinel that does not provide an
  // explicitly retrievable address, and so the only way to properly determine
  // the end pointer is to compute the distance and add it to the start pointer
  //
  // Such a situation could occur for a null-terminator sentinel for C-strings,
  // for example -- since the terminator has no knowledge of distance, but the
  // range is logically contiguous. Whether this would actually happen in
  // practice is a different matter.
  const auto distance = std::distance(start, end);
  MSL_ASSERT(distance >= 0);
  const auto p = check_not_null(std::to_address(start));

  return from_pointer_and_length(p, bytes{static_cast<std::size_t>(distance)});
}

template <typename Iterator>
MSL_FORCE_INLINE constexpr
auto msl::memory_block::from_range(Iterator start, Iterator end)
  -> memory_block
  requires(std::contiguous_iterator<Iterator>)
{
  return memory_block{
    check_not_null(std::to_address(start)),
    check_not_null(std::to_address(end))
  };
}

template <typename Range>
MSL_FORCE_INLINE constexpr
auto msl::memory_block::from_range(Range& range)
  -> memory_block
{
  return from_range(std::ranges::begin(range), std::ranges::end(range));
}

MSL_FORCE_INLINE constexpr
auto msl::memory_block::from_pointer_and_length(
  not_null<std::byte*> p,
  bytes length
) noexcept -> memory_block
{
  return memory_block{p, p + length.count()};
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
auto msl::memory_block::contains(not_null<const std::byte*> p)
  const noexcept -> bool
{
  // TODO(bitwizeshift): consider renaming this 'maybe_contains'?

  // Comparing pointers from possibly different sub-objects is UB; however,
  // the various functional comparators define a total-ordering which can at
  // least allow for a simple containment check to effectively operate.
  //
  // TODO(bitwizeshift):
  //   Determine whether this is guaranteed to always be correct.
  constexpr auto compare = std::less_equal<const std::byte*>{};

  return compare(m_begin.get(), p.get()) && compare(p.get(), m_end.get());
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

inline constexpr
auto msl::memory_block::data()
  const noexcept -> not_null<std::byte*>
{
  return m_begin;
}

inline constexpr
auto msl::memory_block::start_address()
  const noexcept -> not_null<std::byte*>
{
  return m_begin;
}

inline constexpr
auto msl::memory_block::end_address()
  const noexcept -> not_null<std::byte*>
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
  // If we were given a null-range, bail out early. We can't read from an empty
  // range.
  //
  // TODO(bitwizeshift): should this be an assert or contract violation?
  if (first == last) MSL_UNLIKELY {
    return;
  }

  auto it = first;
  const auto end = end_address();

  for (auto jt = start_address(); jt != end; ++jt) {
    *jt = *it;

    // wrap-around logic
    ++it;
    if (it == last) {
      it = first;
    }
  }
}

inline constexpr
auto msl::memory_block::fill(std::initializer_list<std::byte> ilist)
  noexcept -> void
{
  fill(ilist.begin(), ilist.end());
}

//-----------------------------------------------------------------------------
// Private Constructors
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE constexpr
msl::memory_block::memory_block(not_null<std::byte*> begin, not_null<std::byte*> end)
  noexcept
  : m_begin{begin},
    m_end{end}
{

}

//-----------------------------------------------------------------------------
// Traversal
//-----------------------------------------------------------------------------

inline constexpr
auto msl::next_block(memory_block b, uquantity<memory_block> n)
  noexcept -> memory_block
{
  const auto size = b.size();
  const auto offset = size * n.count();

  const auto p = b.start_address() + offset.count();
  return memory_block::from_pointer_and_length(p, size);
}

inline constexpr
auto msl::previous_block(memory_block b, uquantity<memory_block> n)
  noexcept -> memory_block
{
  const auto size = b.size();
  const auto offset = size * n.count();

  const auto p = b.start_address() - offset.count();
  return memory_block::from_pointer_and_length(p, size);
}

//=============================================================================
// class : memory_block_order
//=============================================================================

MSL_FORCE_INLINE constexpr
auto msl::memory_block_order::operator()(const memory_block& lhs, const memory_block& rhs)
  const noexcept -> bool
{
  constexpr auto compare = std::less<const std::byte*>{};

  if (lhs.start_address() == rhs.start_address()) {
    return compare(lhs.end_address().get(), rhs.end_address().get());
  }
  return compare(lhs.start_address().get(), rhs.start_address().get());
}

#endif /* MSL_BLOCKS_MEMORY_BLOCK_HPP */