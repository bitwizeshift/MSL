////////////////////////////////////////////////////////////////////////////////
/// \file assert.hpp
///
/// \brief This header provides a constexpr `assert` macro
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

#ifndef MSL_UTILITIES_ASSERT_HPP
#define MSL_UTILITIES_ASSERT_HPP

#if defined(_MSC_VER)
# pragma once
#endif // defined(_MSC_VER)

#include <cassert>

#define MSL_IMPL_COUNT_VA_ARGS(...)                                            \
  MSL_IMPL_COUNT_VA_ARGS_H(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define MSL_IMPL_COUNT_VA_ARGS_H(_1, _2, _3, _4, _5, _6, _7, _8, _9, N,...)    \
  N

#define MSL_IMPL_JOIN(lhs,rhs)          MSL_IMPL_JOIN_H1(lhs, rhs)
#define MSL_IMPL_JOIN_H1(lhs,rhs)       lhs ## rhs


#define MSL_IMPL_ASSERT_1(condition)                                           \
  (                                                                            \
    (condition)                                                                \
    ? (static_cast<void>(0))                                                   \
    : [&]() { assert(condition); }()                                           \
  )

#define MSL_IMPL_ASSERT_2(condition, message)                                  \
  (                                                                            \
    (condition)                                                                \
    ? ((void)0)                                                                \
    : [&]() { assert(condition && message); }()                                \
  )

//! \def MSL_ASSERT(condition, message)
//!
//! \brief Validates an assertion of the specified condition, with an optional
//!        message
//!
//! \param condition the condition to assert
//! \param message the optional message to print
#if !defined(NDEBUG)
# define MSL_ASSERT(...)                                                       \
  MSL_IMPL_JOIN(MSL_IMPL_ASSERT_, MSL_IMPL_COUNT_VA_ARGS(__VA_ARGS__))(__VA_ARGS__)
#else
# define MSL_ASSERT(...) static_cast<void>(__VA_ARGS__)
#endif

#endif /* MSL_UTILITIES_ASSERT_HPP */
