/*
 Any copyright is dedicated to the Public Domain.
 https://creativecommons.org/publicdomain/zero/1.0/
*/

#include <msl/pointers/raw_pointer.hpp>

#include <catch2/catch.hpp>

#include <memory>

namespace msl::test {

TEST_CASE("raw_pointer<T>") {
  SECTION("T is raw pointer") {
    SECTION("Evaluates to true") {
      STATIC_REQUIRE(raw_pointer<int*>);
    }
  }
  SECTION("T is smart pointer") {
    SECTION("Evaluates to false") {
      STATIC_REQUIRE_FALSE(raw_pointer<std::unique_ptr<int>>);
      STATIC_REQUIRE_FALSE(raw_pointer<std::shared_ptr<int>>);
    }
  }
  SECTION("T is not a pointer") {
    SECTION("Evaluates to false") {
      STATIC_REQUIRE_FALSE(raw_pointer<int>);
    }
  }
}

} // namespace msl::test
