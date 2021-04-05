////////////////////////////////////////////////////////////////////////////////
/// \file pointer_like.hpp
///
/// \brief This header provides the definition for a concept to behave like a
///        pointer
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

#ifndef MSL_CONCEPTS_POINTER_LIKE_HPP
#define MSL_CONCEPTS_POINTER_LIKE_HPP

#include "msl/pointers/raw_pointer.hpp"

#include <concepts> // equality_comparable, equality_comparable_with, etc

namespace msl::detail {

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A concept for determining if a `T` and `U` type are equality
  ///        comparable with one another
  ///
  /// We can't use `equality_comparable_with` here due to an implicit
  /// requirement on copy-construction with the `convertible_to` clause of
  /// the concept -- so this defines a custom
  /// 'weakly_equality_comparable_with' that does the same thing
  ///
  /// \tparam T the left type
  /// \tparam U the right type
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, typename U>
  concept weakly_equality_comparable_with = requires(const T& t, const U& u) {
    { t == u } -> std::convertible_to<bool>;
    { u == t } -> std::convertible_to<bool>;
    { t != u } -> std::convertible_to<bool>;
    { u != t } -> std::convertible_to<bool>;
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A simple trait to check that a given type `T` is dereferencable
  ///
  /// \tparam T the type to check
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  concept dereferenceable = requires(T t) {
    { t.operator->() };
    { *t };
  };
} // namespace msl::detail

namespace msl::inline concepts {

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A concept for things that are "pointer-like", but are not
  ///        necessarily "true" pointers
  ///
  /// The primary requirement for things to be considered "pointer-like" is that
  /// they must be indirectly dereferencable with `operator->` and `operator*`,
  /// equality comparable with itself.
  ///
  /// \tparam T the type to check
  //////////////////////////////////////////////////////////////////////////////
  template <typename T>
  concept pointer_like = raw_pointer<T> || (
    detail::dereferenceable<T> &&
    std::equality_comparable<T> &&
    detail::weakly_equality_comparable_with<T, decltype(nullptr)> &&
    requires(const T t) {
      { static_cast<bool>(t) } -> std::same_as<bool>;
    }
  );
} // namespace msl::inline concepts

#endif /* MSL_CONCEPTS_POINTER_LIKE_HPP */
