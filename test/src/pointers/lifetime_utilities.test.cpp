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

#include "msl/pointers/not_null.hpp"
#include <msl/pointers/lifetime_utilities.hpp>

#include <catch2/catch.hpp>

namespace msl::test {

namespace {

struct test_context {
  bool constructed = false;
  bool destructed = false;
};

template <bool Noexcept = true>
struct test_type {

  explicit test_type(test_context* c)
    noexcept(Noexcept)
    : m_context{c}
  {
    m_context->constructed = true;
  }
  ~test_type()
  {
    m_context->destructed = true;
  }

  test_context* m_context;
};

} // namespace

TEST_CASE("lifetime_utilities::construct_at<T>()", "[construction]") {
  // Arrange
  using sut_type = test_type<>;
  auto context = test_context{};
  auto sut_storage = std::aligned_storage_t<sizeof(sut_type), alignof(sut_type)>{};

  // Act
  [[maybe_unused]]
  auto sut = lifetime_utilities::construct_at<sut_type>(
    assume_not_null(&sut_storage),
    &context
  );

  // Assert
  SECTION("Calls underlying constructor") {
    REQUIRE(context.constructed);
  }
}

TEST_CASE("lifetime_utilities::construct_array_at(void*, std::size_t)", "[construction]") {
  SECTION("T's constructor is non-throwing") {

  }
  SECTION("T's constructor is throwing") {

  }
}

TEST_CASE("lifetime_utilities::construct_array_at(void*, std::size_t, const U&)", "[construction]") {
  SECTION("T's constructor is non-throwing") {

  }
  SECTION("T's constructor is throwing") {

  }
}

TEST_CASE("lifetime_utilities::construct_from_tuple_at(void*,Tuple&&)", "[construction]") {
  SECTION("Constructs array") {

  }
}

//------------------------------------------------------------------------------

TEST_CASE("lifetime_utilities::destroy_at(T*)", "[destroy]") {

}

TEST_CASE("lifetime_utilities::destroy_array_at(T*, std::size_t)", "[destroy]") {

}

TEST_CASE("lifetime_utilities::destroy_range(InputIt, Sentinel)", "[destroy]") {

}

TEST_CASE("lifetime_utilities::destroy_range(Range&&)", "[destroy]") {

}

} // namespace msl::test
