/*
 Any copyright is dedicated to the Public Domain.
 https://creativecommons.org/publicdomain/zero/1.0/
*/

#include <msl/pointers/pointer_like.hpp>

#include <catch2/catch.hpp>

#include <memory>

namespace msl::test {

TEST_CASE("pointer_like<T>") {
  SECTION("T is raw pointer") {
    SECTION("Evaluates to true") {
      STATIC_REQUIRE(pointer_like<int*>);
    }
  }
  SECTION("T is smart pointer") {
    SECTION("Evaluates to true") {
      STATIC_REQUIRE(pointer_like<std::unique_ptr<int>>);
      STATIC_REQUIRE(pointer_like<std::shared_ptr<int>>);
    }
  }
  SECTION("T is not a pointer") {
    SECTION("Evaluates to false") {
      STATIC_REQUIRE_FALSE(pointer_like<int>);
    }
  }
}

} // namespace msl::test
