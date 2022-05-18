////////////////////////////////////////////////////////////////////////////////
/// \file unaligned_utilities.hpp
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

#ifndef MSL_POINTERS_UNALIGNED_UTILITIES_HPP
#define MSL_POINTERS_UNALIGNED_UTILITIES_HPP

#include "msl/pointers/not_null.hpp"

#include <cstdint>
#include <type_traits> // std::is_trivially_destructible_v
#include <cstring>     // std::memcpy
#include <concepts>

namespace msl {

  /////////////////////////////////////////////////////////////////////////////
  /// \brief Utilities for working with unaligned data
  ///
  /// These utilities are important when wanting to read from, or write to,
  /// a memory address.
  /////////////////////////////////////////////////////////////////////////////
  class unaligned_utilities
  {
  public:

    unaligned_utilities() = delete;
    ~unaligned_utilities() = delete;

    /// \brief Writes the value \p value to the memory address \p where
    ///
    /// This function ignores any alignment requirements by making use of
    /// `std::memcpy`
    ///
    /// \pre \p where must be a sequence of at least `sizeof(T)` bytes
    /// \param where the address to write the object \p value to
    /// \param value the value to write
    template <typename T>
    static auto write_to(not_null<std::byte*> where, T&& value)
      noexcept -> void
      requires(std::is_trivially_copyable_v<std::decay_t<T>>);

    /// \brief Reads the value \p value from the memory address \p where
    ///
    /// This function ignores any alignment requirements by making use of
    /// `std::memcpy`
    ///
    /// \pre \p where must be a sequence of at least `sizeof(T)` bytes
    /// \param where the address to read the object from
    /// \return the read value
    template <typename T>
    [[nodiscard]]
    static auto read_from(not_null<const std::byte*> where) noexcept -> T
      requires(std::is_trivially_copyable_v<T>);
  };

} // namespace msl

template <typename T>
auto msl::unaligned_utilities::write_to(not_null<std::byte*> where, T&& value)
  noexcept -> void
  requires(std::is_trivially_copyable_v<std::decay_t<T>>)
{
  std::memcpy(where.as_nullable(), &value, sizeof(T));
}

template <typename T>
auto msl::unaligned_utilities::read_from(not_null<const std::byte*> where)
  noexcept -> T
  requires(std::is_trivially_copyable_v<T>)
{
  T result;
  std::memcpy(&result, where.as_nullable(), sizeof(T));

  return result;
}

#endif /* MSL_POINTERS_UNALIGNED_UTILITIES_HPP */
