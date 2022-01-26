////////////////////////////////////////////////////////////////////////////////
/// \file pointer_utilities.hpp.h
///
/// \brief TODO(Bitwize): Add description
////////////////////////////////////////////////////////////////////////////////

/*
  The MIT License (MIT)

  Copyright (c) 2021 Matthew Rodusek All rights reserved.

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

#ifndef MSL_POINTERS_POINTER_UTILITIES_HPP
#define MSL_POINTERS_POINTER_UTILITIES_HPP

#include "msl/pointers/raw_pointer.hpp"
#include "msl/quantities/alignment.hpp"
#include "msl/utilities/assert.hpp"

#include <cstdint> // std::uintptr_t
#include <bit>     // std::countr_zero

namespace msl {

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A collection of utilities for computing and modeling pointer
  ///        alignment
  ///
  /// \note
  /// Functions that align pointers only work on sequences of bytes, rather than
  /// `void*` or `T*` pointers. Formally, a pointer returned from `uintptr_t` is
  /// not considered "safely-derived" since arithmetic with `uintptr_t` is not
  /// guaranteed to model the same address space (only the round-trip from
  /// `uintptr_t` is considered legal). However, we can safely compute *offsets*
  /// with `uintptr_t`, and then add that offset to a pointer to a byte sequence
  /// in order to traverse the sequence in a legal manner
  ///
  /// This traversal using `std::byte` can be removed by defining
  /// `MSL_DISABLE_STRICT_MODE` at compile-time.
  //////////////////////////////////////////////////////////////////////////////
  class pointer_utilities final
  {
    pointer_utilities() = delete;
    ~pointer_utilities() = delete;

    //--------------------------------------------------------------------------
    // Queries
    //--------------------------------------------------------------------------
  public:

    /// \brief Queries whether a given pointer \p p is aligned to the specified
    ///        boundary \p align
    ///
    /// \param p the pointer to test
    /// \param align the alignment to query
    /// \return `true` if `p` is aligned to `align`
    static auto is_aligned(const void* p, alignment align) noexcept -> bool;

    /// \brief Queries the current alignment of the specified pointer \p p
    ///
    /// \param p the pointer to query
    /// \return the alignment
    static auto alignment_of(const void* p) noexcept -> alignment;

    //--------------------------------------------------------------------------
    // Alignment
    //--------------------------------------------------------------------------
  public:

    /// \brief Aligns the pointer \p p to the next highest address that is a
    ///        multiple of \p align
    ///
    /// \param p the pointer to align
    /// \param align the alignment
    /// \return the newly aligned pointer
    static auto align_high(std::byte* p, alignment align) noexcept -> std::byte*;

    /// \brief Aligns the pointer \p p to the next highest address that is a
    ///        \p offset bytes from a multiple of \p align
    ///
    /// \note
    /// An offset alignment can be useful to prepare additional storage for a
    /// pointer, such as if we wanted to store how many bytes the entire
    /// allocation will take.
    ///
    /// \param p the pointer to align
    /// \param align the alignment
    /// \param offset the number of bytes to offset from the boundary
    /// \return the newly aligned pointer
    static auto offset_align_high(std::byte* p, alignment align, std::size_t offset)
      noexcept -> std::byte*;

    /// \brief Aligns the pointer \p p the previous lowest address that is a
    ///        a multiple of \p align
    ///
    /// \param p the pointer to align
    /// \param align the alignment
    /// \return the newly aligned pointer
    static auto align_low(std::byte* p, alignment align)
      noexcept -> std::byte*;

    /// \brief Aligns the pointer \p p to the previous lowest address that is a
    ///        \p offset bytes from a multiple of \p align
    ///
    /// \note
    /// An offset alignment can be useful to prepare additional storage for a
    /// pointer, such as if we wanted to store how many bytes the entire
    /// allocation will take.
    ///
    /// \param p the pointer to align
    /// \param align the alignment
    /// \return the newly aligned pointer
    static auto offset_align_low(std::byte* p, alignment align, std::size_t offset)
      noexcept -> std::byte*;
  };

} // namespace msl

//-----------------------------------------------------------------------------
// Queries
//-----------------------------------------------------------------------------

inline
auto msl::pointer_utilities::is_aligned(const void* p, alignment align)
  noexcept -> bool
{
  const auto address = reinterpret_cast<std::uintptr_t>(p);
  const auto mask = (align.value().count() - 1u);

  return (address & mask) == 0u;
}

inline
auto msl::pointer_utilities::alignment_of(const void* p)
  noexcept -> alignment
{
  const auto address = reinterpret_cast<std::uintptr_t>(p);

  return alignment::assume_at_boundary(std::countr_zero(address));
}

//-----------------------------------------------------------------------------
// Alignment
//-----------------------------------------------------------------------------

inline
auto msl::pointer_utilities::align_high(std::byte* p, alignment align)
  noexcept -> std::byte*
{
  const auto address = reinterpret_cast<std::uintptr_t>(p);
  const auto mask    = static_cast<std::uintptr_t>(~(align.value().count() - 1u));
  const auto padding = static_cast<std::uintptr_t>(align.value().count() - 1u);

  const auto new_address = (address + padding) & mask;

  MSL_ASSERT(address <= new_address);

#if MSL_DISABLE_STRICT_MODE
  return reinterpret_cast<std::byte*>(new_address);
#else
  return p + (new_address - address);
#endif
}

inline
auto msl::pointer_utilities::offset_align_high(std::byte* p,
                                               alignment align,
                                               std::size_t offset)
  noexcept -> std::byte*
{
  const auto address = reinterpret_cast<std::uintptr_t>(p);
  const auto mask    = static_cast<std::uintptr_t>(~(align.value().count() - 1u));
  const auto padding = static_cast<std::uintptr_t>(align.value().count() - 1u + offset);

  const auto new_address = ((address + padding) & mask) - offset;

  MSL_ASSERT(address <= new_address);

#if MSL_DISABLE_STRICT_MODE
  return reinterpret_cast<std::byte*>(new_address);
#else
  return p + (new_address - address);
#endif
}

inline
auto msl::pointer_utilities::align_low(std::byte* p, alignment align)
  noexcept -> std::byte*
{
  const auto address = reinterpret_cast<std::uintptr_t>(p);
  const auto mask    = static_cast<std::uintptr_t>(~(align.value().count() - 1u));

  const auto new_address = (address & mask);

  MSL_ASSERT(address >= new_address);

#if MSL_DISABLE_STRICT_MODE
  return reinterpret_cast<std::byte*>(new_address);
#else
  return p - (address - new_address);
#endif
}

inline
auto msl::pointer_utilities::offset_align_low(std::byte* p,
                                              alignment align,
                                              std::size_t offset)
  noexcept -> std::byte*
{
  const auto address = reinterpret_cast<std::uintptr_t>(p);
  const auto mask    = static_cast<std::uintptr_t>(~(align.value().count() - 1u));

  const auto new_address = (address & mask) - offset;

  MSL_ASSERT(address >= new_address);

#if MSL_DISABLE_STRICT_MODE
  return reinterpret_cast<std::byte*>(new_address);
#else
  return p - (address - new_address);
#endif
}

#endif /* MSL_POINTERS_POINTER_UTILITIES_HPP */
