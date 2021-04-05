////////////////////////////////////////////////////////////////////////////////
/// \file traversable_pointer.hpp.h
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

#ifndef MSL_POINTERS_TRAVERSABLE_POINTER_HPP
#define MSL_POINTERS_TRAVERSABLE_POINTER_HPP

#include "msl/pointers/pointer_like.hpp"

namespace msl::inline concepts {

  ////////////////////////////////////////////////////////////////////////////
  /// \brief A concept to model any pointer-like type that has the traveral
  ///        functionalities made available to it
  ///
  /// Traversal is considered only in terms of the incrementing/decrementing
  /// functionalities, along with the member and non-member
  /// addition/subtraction -- but does not consider array-subscripting.
  ////////////////////////////////////////////////////////////////////////////
  template <typename T>
  concept traversable_pointer = requires(T t, const T ct, std::ptrdiff_t n)
  {
    // incrementable
    { ++t } -> std::convertible_to<T&>;
    { t++ } -> std::convertible_to<T>;
    // decrementable
    { --t } -> std::convertible_to<T&>;
    { t-- } -> std::convertible_to<T>;
    // compound addable/subtractable
    { t += n } -> std::convertible_to<T&>;
    { t -= n } -> std::convertible_to<T&>;
    // addable/subtractable
    { ct + n } -> std::convertible_to<T>;
    { n + ct } -> std::convertible_to<T>;
    { ct - n } -> std::convertible_to<T>;
    // distance
    { ct - ct } -> std::convertible_to<std::ptrdiff_t>;
  } && pointer_like<T>;

} // namespace msl::inline concepts

#endif /* INCLUDE_MSL_POINTERS_TRAVERSABLE_POINTER_HPP */
