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

#include "msl/cells/cell.hpp"
#include "msl/pointers/not_null.hpp"

#include <algorithm>
#include <catch2/catch.hpp>

namespace msl::test {

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

TEMPLATE_TEST_CASE("cell<T>::cell(U*)", "[ctor]", const int, int) {
  // Arrange
  auto value = TestType{};

  // Act
  const auto sut = cell<const int>{assume_not_null(&value)};

  // Assert
  SECTION("Data points to underlying data") {
    REQUIRE(sut.data() == assume_not_null(&value));
  }
  SECTION("Size is 1") {
    REQUIRE(sut.size() == 1u);
  }
  SECTION("Size in bytes is sizeof(T)") {
    REQUIRE(sut.size_in_bytes() == size_of<int>());
  }
}

TEMPLATE_TEST_CASE("cell<T[]>::cell(U*, uquantity<T>)", "[ctor]", const int, int) {
  // Arrange
  constexpr auto size = 5u;
  TestType value[size]{};

  // Act
  const auto sut = cell<const int[]>{assume_not_null(value), size};

  // Assert
  SECTION("Data points to underlying data") {
    REQUIRE(sut.data() == assume_not_null(value));
  }
  SECTION("Size is size of array") {
    REQUIRE(sut.size() == size);
  }
  SECTION("Size in bytes is sizeof(T) * size") {
    REQUIRE(sut.size_in_bytes() == size_of<int>() * size);
  }
}

TEMPLATE_TEST_CASE("cell<T[N]>::cell(U*)", "[ctor]", const int, int) {
  // Arrange
  constexpr auto size = 5u;
  TestType value[size]{};

  // Act
  const auto sut = cell<const int[size]>{assume_not_null(value)};

  // Assert
  SECTION("Data points to underlying data") {
    REQUIRE(sut.data() == assume_not_null(value));
  }
  SECTION("Size is size of array") {
    REQUIRE(sut.size() == size);
  }
  SECTION("Size in bytes is sizeof(T) * size") {
    REQUIRE(sut.size_in_bytes() == size_of<int>() * size);
  }
}

//-----------------------------------------------------------------------------

TEMPLATE_TEST_CASE("cell<T>::cell(cell<U,Align>)", "[ctor]", const int, int) {
  // Arrange
  constexpr auto align = alignof(TestType) * 2u;
  alignas(align) auto value = TestType{};

  SECTION("Align of other is greater than new cell") {
    // Arrange
    const auto other = cell<TestType,align>{assume_not_null(&value)};

    // Act
    const auto sut = cell<const int>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is implicit") {
      STATIC_REQUIRE(std::is_convertible_v<cell<TestType,align>, cell<const int>>);
    }
  }
  SECTION("Align of other is less than new cell") {
    // Arrange
    const auto other = cell<TestType>{assume_not_null(&value)};

    // Act
    const auto sut = cell<const int,align>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is explicit") {
      STATIC_REQUIRE(std::is_constructible_v<cell<const int, align>, cell<TestType>>);
    }
  }
}

TEMPLATE_TEST_CASE("cell<T[]>::cell(cell<U[],Align>)", "[ctor]", const int, int) {
  // Arrange
  constexpr auto size = 5u;
  constexpr auto align = alignof(TestType) * 2u;
  alignas(align) TestType value[size]{};

  SECTION("Align of other is greater than new cell") {
    // Arrange
    const auto other = cell<TestType[],align>{assume_not_null(value), size};

    // Act
    const auto sut = cell<const int[]>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is implicit") {
      STATIC_REQUIRE(std::is_convertible_v<cell<TestType[],align>, cell<const int[]>>);
    }
  }
  SECTION("Align of other is less than new cell") {
    // Arrange
    const auto other = cell<TestType[]>{assume_not_null(value), size};

    // Act
    const auto sut = cell<const int[],align>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is explicit") {
      STATIC_REQUIRE(std::is_constructible_v<cell<const int[], align>, cell<TestType[]>>);
    }
  }
}

TEMPLATE_TEST_CASE("cell<T[N]>::cell(cell<U[N],Align>)", "[ctor]", const int, int) {
  // Arrange
  constexpr auto size = 5u;
  constexpr auto align = alignof(TestType) * 2u;
  alignas(align) TestType value[size]{};

  SECTION("Align of other is greater than new cell") {
    // Arrange
    const auto other = cell<TestType[size],align>{assume_not_null(value)};

    // Act
    const auto sut = cell<const int[size]>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is implicit") {
      STATIC_REQUIRE(std::is_convertible_v<cell<TestType[size],align>, cell<const int[size]>>);
    }
  }
  SECTION("Align of other is less than new cell") {
    // Arrange
    const auto other = cell<TestType[size]>{assume_not_null(value)};

    // Act
    const auto sut = cell<const int[size],align>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is explicit") {
      STATIC_REQUIRE(std::is_constructible_v<cell<const int[size], align>, cell<TestType[size]>>);
    }
  }
}

//-----------------------------------------------------------------------------

TEMPLATE_TEST_CASE("cell<T[]>::cell(cell<U[N],Align>)", "[ctor]", const int, int) {
  // Arrange
  constexpr auto size = 5u;
  constexpr auto align = alignof(TestType) * 2u;
  alignas(align) TestType value[size]{};

  SECTION("Align of other is greater than new cell") {
    // Arrange
    const auto other = cell<TestType[size],align>{assume_not_null(value)};

    // Act
    const auto sut = cell<const int[]>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is implicit") {
      STATIC_REQUIRE(std::is_convertible_v<cell<TestType[size],align>, cell<const int[]>>);
    }
  }
  SECTION("Align of other is less than new cell") {
    // Arrange
    const auto other = cell<TestType[size]>{assume_not_null(value)};

    // Act
    const auto sut = cell<const int[],align>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is explicit") {
      STATIC_REQUIRE(std::is_constructible_v<cell<const int[], align>, cell<TestType[size]>>);
    }
  }
}


TEMPLATE_TEST_CASE("cell<T[N]>::cell(cell<U[],Align>)", "[ctor]", const int, int) {
  // Arrange
  constexpr auto size = 5u;
  constexpr auto align = alignof(TestType) * 2u;
  alignas(align) TestType value[size]{};

  SECTION("Align of other is greater than new cell") {
    // Arrange
    const auto other = cell<TestType[],align>{assume_not_null(value), size};

    // Act
    const auto sut = cell<const int[size]>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is implicit") {
      STATIC_REQUIRE(std::is_constructible_v<cell<const int[size]>, cell<TestType[],align>>);
    }
  }
  SECTION("Align of other is less than new cell") {
    // Arrange
    const auto other = cell<TestType[]>{assume_not_null(value), size};

    // Act
    const auto sut = cell<const int[size],align>{other};

    // Assert
    SECTION("Data points to other cell") {
      REQUIRE(sut.data() == other.data());
    }
    SECTION("Size is same as other cell") {
      REQUIRE(sut.size() == other.size());
    }
    SECTION("Size in bytes is same as other cell") {
      REQUIRE(sut.size_in_bytes() == other.size_in_bytes());
    }
    SECTION("Constructor is explicit") {
      STATIC_REQUIRE(std::is_constructible_v<cell<const int[size], align>, cell<TestType[]>>);
    }
  }
}

//-----------------------------------------------------------------------------
// Equality
//-----------------------------------------------------------------------------

TEMPLATE_TEST_CASE("operator==(const cell<T>&, const cell<U>&)", "[equality]", int, const int) {
  // Arrange
  auto value = 5;
  const auto lhs = cell<const int>{assume_not_null(&value)};

  SECTION("Pointers compare equal") {
    // Arrange
    const auto rhs = cell<TestType>{assume_not_null(&value)};

    // Act
    REQUIRE(lhs == rhs);
  }
  SECTION("Pointers compare inequal") {
    // Arrange
    auto other = 5;
    const auto rhs = cell<TestType>{assume_not_null(&other)};

    // Act
    REQUIRE_FALSE(lhs == rhs);
  }
}

TEMPLATE_TEST_CASE("operator==(const cell<T[]>&, const cell<U[]>&)", "[equality]", int, const int) {
  // Arrange
  constexpr auto size = 5u;
  int value[size]{};
  const auto lhs = cell<const int[]>{assume_not_null(value), size};

  SECTION("Pointers compare equal") {
    // Arrange
    const auto rhs = cell<TestType[]>{assume_not_null(value), size};

    // Act
    REQUIRE(lhs == rhs);
  }
  SECTION("Pointers compare inequal") {
    SECTION("Pointer is different object") {
      // Arrange
      int other[size]{};
      const auto rhs = cell<TestType[]>{assume_not_null(other), size};

      // Act
      REQUIRE_FALSE(lhs == rhs);
    }
    SECTION("Pointer is same subobject, different size") {
      // Arrange
      const auto rhs = cell<TestType[]>{assume_not_null(value), size - 1u};

      // Act
      REQUIRE_FALSE(lhs == rhs);
    }
  }
}

TEMPLATE_TEST_CASE("operator==(const cell<T[]>&, const cell<U[N]>&)", "[equality]", int, const int) {
  // Arrange
  constexpr auto size = 5u;
  int value[size]{};
  const auto lhs = cell<const int[]>{assume_not_null(value), size};

  SECTION("Pointers compare equal") {
    // Arrange
    const auto rhs = cell<TestType[size]>{assume_not_null(value)};

    // Act
    REQUIRE(lhs == rhs);
    REQUIRE(rhs == lhs);
  }
  SECTION("Pointers compare inequal") {
    SECTION("Pointer is different object") {
      // Arrange
      int other[size]{};
      const auto rhs = cell<TestType[size]>{assume_not_null(other)};

      // Act
      REQUIRE_FALSE(lhs == rhs);
      REQUIRE_FALSE(rhs == lhs);
    }
    SECTION("Pointer is same subobject, different size") {
      // Arrange
      const auto rhs = cell<TestType[size-1]>{assume_not_null(value)};

      // Act
      REQUIRE_FALSE(lhs == rhs);
      REQUIRE_FALSE(rhs == lhs);
    }
  }
}

TEMPLATE_TEST_CASE("operator==(const cell<T[N]>&, const cell<U[M]>&)", "[equality]", int, const int) {
  // Arrange
  constexpr auto size = 5u;
  int value[size]{};
  const auto lhs = cell<const int[size]>{assume_not_null(value)};

  SECTION("Pointers compare equal") {
    // Arrange
    const auto rhs = cell<TestType[size]>{assume_not_null(value)};

    // Act
    REQUIRE(lhs == rhs);
  }
  SECTION("Pointers compare inequal") {
    SECTION("Pointer is different object") {
      // Arrange
      int other[size]{};
      const auto rhs = cell<TestType[size]>{assume_not_null(other)};

      // Act
      REQUIRE_FALSE(lhs == rhs);
    }
    SECTION("Pointer is same subobject, different size") {
      // Arrange
      const auto rhs = cell<TestType[size - 1]>{assume_not_null(value)};

      // Act
      REQUIRE_FALSE(lhs == rhs);
    }
  }
}

//-----------------------------------------------------------------------------
// Iterators
//-----------------------------------------------------------------------------

TEST_CASE("cell<T>::begin/end", "[iterators]") {
  // Arrange
  const auto value = int{};
  const auto sut = cell<const int>{assume_not_null(&value)};

  // Act / Assert
  SECTION("Range is one element") {
    REQUIRE(std::distance(begin(sut), end(sut)) == 1u);
  }
  SECTION("Iterates the one element") {
    REQUIRE(std::equal(begin(sut), end(sut), &value, &value + 1));
  }
}

TEST_CASE("cell<T[]>::begin/end", "[iterators]") {
  // Arrange
  const auto size = 5u;
  const int value[size]{};
  const auto sut = cell<const int[]>{assume_not_null(value), size};

  // Act / Assert
  SECTION("Range is 'size' element") {
    REQUIRE(std::distance(begin(sut), end(sut)) == size);
  }
  SECTION("Iterates the elements") {
    REQUIRE(std::equal(begin(sut), end(sut), &value[0], &value[0] + size));
  }
}

TEST_CASE("cell<T[N]>::begin/end", "[iterators]") {
  // Arrange
  const auto size = 5u;
  const int value[size]{};
  const auto sut = cell<const int[size]>{assume_not_null(value)};

  // Act / Assert
  SECTION("Range is 'size' element") {
    REQUIRE(std::distance(begin(sut), end(sut)) == size);
  }
  SECTION("Iterates the elements") {
    REQUIRE(std::equal(begin(sut), end(sut), &value[0], &value[0] + size));
  }
}

} // namespace msl::test