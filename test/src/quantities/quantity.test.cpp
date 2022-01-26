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

#include "msl/quantities/quantity.hpp"

#include <catch2/catch.hpp>

namespace msl::test {
namespace {

struct Fruit {};
struct Orange : Fruit {};
struct Apple : Fruit {};

template <typename T, typename U>
concept weakly_equality_comparable_with = requires(const T& l, const U& r) {
  { l == r } -> std::convertible_to<bool>;
  { r == l } -> std::convertible_to<bool>;
  { l != r } -> std::convertible_to<bool>;
  { r != l } -> std::convertible_to<bool>;
};

} // namespace anonymous

//==============================================================================
// class : quantity
//==============================================================================

//------------------------------------------------------------------------------
// Concepts
//------------------------------------------------------------------------------

TEST_CASE("quantity<T,Rep>", "[concepts]") {
  SECTION("Objects are equality comparable") {
    STATIC_REQUIRE(std::equality_comparable<quantity<Fruit>>);
  }
  SECTION("Objects are equality comparable with integer values") {
    STATIC_REQUIRE(std::equality_comparable_with<quantity<Fruit>,std::size_t>);
  }
  SECTION("Objects are equality comparable with other quantities") {
    STATIC_REQUIRE(weakly_equality_comparable_with<quantity<Apple>,quantity<Orange>>);
  }
}

//------------------------------------------------------------------------------
// Constructors
//------------------------------------------------------------------------------

TEST_CASE("quantity<T,Rep>::quantity()", "[ctor]") {
  const auto sut = quantity<Apple>{};

  SECTION("Default-initializes the underlying value") {
    REQUIRE(sut == 0);
  }
}

TEST_CASE("quantity<T,Rep>::quantity(const Rep&)", "[ctor]") {
  const auto value = 42;
  const auto sut = quantity<Apple>{value};

  SECTION("Constructs the quantity with the underlying value") {
    REQUIRE(sut == value);
  }
}


TEST_CASE("quantity<T,Rep>::quantity(const quantity<U,URep>&)", "[ctor]") {
  SECTION("Constructs the quantity with the value of the other") {
    const auto value = quantity<Orange>{42};
    const auto sut = quantity<Fruit>{value};

    REQUIRE(sut == value);
  }
  SECTION("Is not constructible from non-derived values") {
    STATIC_REQUIRE_FALSE(std::constructible_from<quantity<Apple>,quantity<Orange>>);
  }
}

//------------------------------------------------------------------------------
// Assignment
//------------------------------------------------------------------------------

TEST_CASE("quantity<T,Rep>::operator=(const quantity<U,URep>&)", "[assign]") {

}

TEST_CASE("quantity<T,Rep>::operator=(const Rep&)", "[assign]") {

}

TEST_CASE("quantity<T,Rep>::operator==", "[comparisons]") {
  SECTION("Quantities of different types are equality comparable") {
    const auto lhs = quantity<Orange>{42};
    const auto rhs = quantity<Apple>{42};

    REQUIRE(lhs == rhs);
  }
}

} // namespace msl::test
