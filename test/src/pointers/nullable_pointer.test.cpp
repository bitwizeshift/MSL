/*
 Any copyright is dedicated to the Public Domain.
 https://creativecommons.org/publicdomain/zero/1.0/
*/

#include <msl/pointers/nullable_pointer.hpp>
#include <msl/pointers/not_null.hpp>

#include <catch2/catch.hpp>

#include <memory>

namespace msl::test {

TEST_CASE("nullable_pointer<T>") {
  SECTION("T is raw pointer") {
    SECTION("Evaluates to true") {
      STATIC_REQUIRE(nullable_pointer<int*>);
    }
  }
  SECTION("T is pointer_like but does not offer nullability") {
    SECTION("Evaluates to false") {
      STATIC_REQUIRE_FALSE(nullable_pointer<not_null<int*>>);
    }
  }
  SECTION("T is not a pointer") {
    SECTION("Evaluates to false") {
      STATIC_REQUIRE_FALSE(traversable_pointer<int>);
    }
  }
}

} // namespace msl::test
