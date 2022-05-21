/*
 Any copyright is dedicated to the Public Domain.
 https://creativecommons.org/publicdomain/zero/1.0/
*/

#include <msl/pointers/traversable_pointer.hpp>

#include <catch2/catch.hpp>

#include <memory>

namespace msl::test {

TEST_CASE("traversable_pointer<T>") {
  SECTION("T is raw pointer") {
    SECTION("Evaluates to true") {
      STATIC_REQUIRE(traversable_pointer<int*>);
    }
  }
  SECTION("T is pointer_like and defines traversal operators") {
    SECTION("Evaluates to true") {
      STATIC_REQUIRE(traversable_pointer<int*>);
    }
  }
  SECTION("T is not a pointer") {
    SECTION("Evaluates to false") {
      STATIC_REQUIRE_FALSE(traversable_pointer<int>);
    }
  }
}

} // namespace msl::test
