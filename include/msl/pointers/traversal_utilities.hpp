///////////////////////////////////////////////////////////////////////////////
/// \file traversal.hpp
///
/// \todo(Bitwize): Document this
///////////////////////////////////////////////////////////////////////////////

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
#ifndef MSL_POINTERS_TRAVERSAL_UTILITIES_HPP
#define MSL_POINTERS_TRAVERSAL_UTILITIES_HPP

#if defined(_MSC_VER)
# pragma once
#endif // defined(_MSC_VER)

#include "msl/pointers/not_null.hpp"
#include "msl/quantities/quantity.hpp"
#include "msl/quantities/digital_quantity.hpp"

#include <new>     // std::launder
#include <cstddef> // std::byte

namespace msl {

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A collection of utilities that deal with general traversal
  ///        requirements across pointers.
  ///
  /// This utility exists to help bypass C++ standard legality requirements
  /// regarding reachability. In particular, it's legal to alias a byte sequence
  /// as a `T*` -- but it's not legal to attempt to index past the first `T*`,
  /// e.g. such as `p + n`, unless that byte sequence refers to a true
  /// `T[N]` (array of `T` objects).
  ///
  /// Although this general requirement is effectively esoteric, it can be
  /// logically resolved. Any `T` type can be aliased as a byte-sequence. When
  /// working with allocators, almost 100% of the time all memory is some form
  /// of byte-sequence. So converting to a byte sequence, computing the
  /// arithmetic manually, and returning back the reference to a safely
  /// derived pointer location that points to a laundered `T*` pointer is
  /// technically valid.
  ///
  /// This abstracts the concept in a simple API, and also conditionally
  /// supports the option to bypass the pedanticness via compile-time
  /// preprocessor statements.
  ///
  /// \note These utilities are a building block for `cell` objects.
  /////////////////////////////////////////////////////////////////////////////
  struct traversal_utilities final
  {
    traversal_utilities() = delete;
    ~traversal_utilities() = delete;

    /// \brief Traverses a contiguous object sequence `p` without violating
    ///        reachability constraints
    ///
    /// In C++, only true array objects are considered "reachable" by indirect
    /// indexing -- otherwise formally doing so is undefined behavior.
    /// This means that, legally, you cannot allocate an object of type `T` that
    /// is not already constructed as an array, and index to its location
    /// without violating reachability.
    ///
    /// Since an uninitialized object type `T` is not yet an established
    /// instance or array, the underlying data object is still considered a
    /// byte-sequence -- meaning that reachability between different objects
    /// may still be achieved by casting to a byte and computing the offset.
    ///
    /// This function does exactly this; it converts the `T` pointer to a
    /// byte pointer, and offsets into that sequence by sizeof(T) -- which is
    /// the only legal way to perform this within an allocator, since otherwise
    /// we are dealing with potential aliasing and reachability violations.
    ///
    /// \param p the pointer to the start of a T object
    template <typename T>
    [[nodiscard]]
    static auto advance(not_null<T*> p, quantity<std::remove_cv_t<T>> n) noexcept -> not_null<T*>;

    /// \brief Proceed to the next \p p entry by advancing once
    ///
    /// \param p the pointer to advance
    /// \return the advanced pointer
    template <typename T>
    [[nodiscard]]
    static constexpr auto next(not_null<T*> p) noexcept -> not_null<T*>;

    /// \brief Proceed to the previous \p p entry by decrementing once
    ///
    /// \param p the pointer to decrement
    /// \return the decremented pointer
    template <typename T>
    [[nodiscard]]
    static constexpr auto previous(not_null<T*> p) noexcept -> not_null<T*>;

    /// \brief Accesses the element at the specified \p offset from the pointer
    ///        \p p
    ///
    /// \param p the pointer
    /// \param offset the offset
    template <typename T>
    [[nodiscard]]
    static auto access_at_offset(not_null<T*> p, quantity<std::remove_cv_t<T>> offset) noexcept -> T&;
  };

} // namespace msl

template <typename T>
inline
auto msl::traversal_utilities::advance(not_null<T*> p, quantity<std::remove_cv_t<T>> n)
  noexcept -> not_null<T*>
{
#if MSL_DISABLE_STRICT_MODE
  return (p + n.count());
#else
  // All 'cell' objects returned from allocators come from contiguous
  // `std::byte` sequences, and any "arrays" of elements returned from an
  // allocator are really just N objects stored in sequence -- so this should be
  // legal as far as the C++ standard is concerned. Even if used on a legitimate
  // array of T objects, array objects are legally aliasable as bytes, and bytes
  // and this computation safely derives a dependent pointer address.
  return launder(
    reinterpret_pointer_cast<T>(
      reinterpret_pointer_cast<std::byte>(p) + (n.count() * sizeof(T))
    )
  );
#endif
}

template <typename T>
inline constexpr
auto msl::traversal_utilities::next(not_null<T*> p)
  noexcept -> not_null<T*>
{
  return advance(p, 1);
}

template <typename T>
inline constexpr
auto msl::traversal_utilities::previous(not_null<T*> p)
  noexcept -> not_null<T*>
{
  return advance(p, -1);
}

template <typename T>
inline
auto msl::traversal_utilities::access_at_offset(not_null<T*> p, quantity<std::remove_cv_t<T>> offset)
  noexcept -> T&
{
  return *advance(p, offset);
}

#endif /* MSL_POINTERS_TRAVERSAL_UTILITIES_HPP */
