////////////////////////////////////////////////////////////////////////////////
/// \file nullable_pointer.hpp.h
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

#ifndef MSL_POINTERS_NULLABLE_POINTER_HPP
#define MSL_POINTERS_NULLABLE_POINTER_HPP

#include "msl/pointers/pointer_like.hpp" // detail::weakly_equality_comparable_with

#include <concepts>

namespace msl::inline concepts {

  ////////////////////////////////////////////////////////////////////////////
  /// \brief A concept for nullable_pointer
  ///
  /// The nullable pointer concept is identical to the equivalent named
  /// requirement in the C++ standard.
  ///
  /// For a given type `T`, it is considered a `nullable_point` if it
  /// satisfies the following requirements:
  ///
  /// 1. `T` is default-constructible
  /// 2. `T` is constructible from a `nullptr` (`T(nullptr)` is well-formed)
  /// 3. `T` is assignable from a `nullptr` (`t = nullptr` is well-formed)
  /// 4. `T` is equality comparable with other `T` types
  /// 5. `T` is equality comparable with `nullptr`
  ///
  /// \tparam T the type to check
  ////////////////////////////////////////////////////////////////////////////
  template <typename T>
  concept nullable_pointer = (
    std::default_initializable<T> &&
    std::constructible_from<T, decltype(nullptr)> &&
    std::is_assignable_v<T&,decltype(nullptr)> &&
    std::equality_comparable<T> &&
    detail::weakly_equality_comparable_with<T,decltype(nullptr)>
  );

} // namespace msl::inline concepts

#endif /* MSL_POINTERS_NULLABLE_POINTER_HPP */
