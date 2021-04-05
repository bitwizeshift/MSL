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
