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

#include <msl/pointers/tagged_ptr.hpp>

#include <catch2/catch.hpp>

#include <memory>

namespace msl::test {
namespace {

  struct base {long long x;};
  struct derived : base {int y;};

} // namespace <anonymous>

//------------------------------------------------------------------------------
// Constructors
//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::tagged_ptr()") {
  const auto sut = tagged_ptr<int,2>{};

  SECTION("Points to null") {
    SECTION("Is equal to null") {
      REQUIRE(sut == nullptr);
    }
    SECTION("Is convertible to `false`") {
      REQUIRE_FALSE(sut);
    }
  }
  SECTION("Has no tag") {
    REQUIRE(sut.tag() == 0u);
  }
}

//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::tagged_ptr(std::nullptr_t)") {
  const auto sut = tagged_ptr<int,2>{nullptr};

  SECTION("Points to null") {
    SECTION("Is equal to null") {
      REQUIRE(sut == nullptr);
    }
    SECTION("Is convertible to `false`") {
      REQUIRE_FALSE(sut);
    }
  }
  SECTION("Has no tag") {
    REQUIRE(sut.tag() == 0u);
  }
}

//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::tagged_ptr(pointer)") {
  SECTION("U is not convertible to T") {
    SECTION("tagged_ptr<T> is not constructible from U") {
      STATIC_REQUIRE_FALSE(std::is_constructible_v<tagged_ptr<int,2>,long*>);
    }
  }
  SECTION("U is convertible to T") {
    SECTION("U is only a CV-qualification different from T") {
      SECTION("tagged_ptr<const T> is constructible from T*") {
        STATIC_REQUIRE(std::is_constructible_v<tagged_ptr<const int,2>, int*>);
      }

      auto value = int{};
      auto sut = tagged_ptr<const int,2>{&value};

      SECTION("Pointer references input") {
        REQUIRE(sut.get() == &value);
      }
      SECTION("tagged_ptr is non-null") {
        REQUIRE(sut);
      }
      SECTION("tagged_ptr has empty tag") {
        REQUIRE(sut.tag() == 0u);
      }
    }
    SECTION("U is base of T") {
      SECTION("tagged_ptr<base> is constructible from derived*") {
        STATIC_REQUIRE(std::is_constructible_v<tagged_ptr<base,2>, derived*>);
      }

      auto value = derived{};
      auto sut = tagged_ptr<base,2>{&value};

      SECTION("Pointer references input") {
        REQUIRE(sut.get() == &value);
      }
      SECTION("tagged_ptr is non-null") {
        REQUIRE(sut);
      }
      SECTION("tagged_ptr has empty tag") {
        REQUIRE(sut.tag() == 0u);
      }
    }
  }
  SECTION("U is T") {
    SECTION("tagged_ptr<T> is constructible from T*") {
      STATIC_REQUIRE(std::is_constructible_v<tagged_ptr<int,2>, int*>);
    }
    auto value = int{};
    auto sut = tagged_ptr<int,2>{&value};

    SECTION("Pointer references input") {
      REQUIRE(sut.get() == &value);
    }
    SECTION("tagged_ptr is non-null") {
      REQUIRE(sut);
    }
    SECTION("tagged_ptr has empty tag") {
      REQUIRE(sut.tag() == 0u);
    }
  }
}

//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::tagged_ptr(pointer, std::uintptr_t)") {
  SECTION("U is not convertible to T") {
    SECTION("tagged_ptr<T> is not constructible from U") {
      STATIC_REQUIRE_FALSE(std::is_constructible_v<tagged_ptr<int,2>,long*, std::uintptr_t>);
    }
  }
  SECTION("U is convertible to T") {
    SECTION("U is only a CV-qualification different from T") {
      SECTION("tagged_ptr<const T> is constructible from T*") {
        STATIC_REQUIRE(std::is_constructible_v<tagged_ptr<const int,2>, int*, std::uintptr_t>);
      }

      const auto tag = std::uintptr_t{0b10};
      auto value = int{};
      auto sut = tagged_ptr<const int,2>{&value, tag};

      SECTION("Pointer references input") {
        REQUIRE(sut.get() == &value);
      }
      SECTION("tagged_ptr is non-null") {
        REQUIRE(sut);
      }
      SECTION("tagged_ptr has specified tag") {
        REQUIRE(sut.tag() == tag);
      }
    }
    SECTION("U is base of T") {
      SECTION("tagged_ptr<base> is constructible from derived*") {
        STATIC_REQUIRE(std::is_constructible_v<tagged_ptr<base,2>, derived*, std::uintptr_t>);
      }

      const auto tag = std::uintptr_t{0b10};
      auto value = derived{};
      auto sut = tagged_ptr<base,2>{&value, tag};

      SECTION("Pointer references input") {
        REQUIRE(sut.get() == &value);
      }
      SECTION("tagged_ptr is non-null") {
        REQUIRE(sut);
      }
      SECTION("tagged_ptr has specified tag") {
        REQUIRE(sut.tag() == tag);
      }
    }
  }
  SECTION("U is T") {
    SECTION("tagged_ptr<T> is constructible from T*") {
      STATIC_REQUIRE(std::is_constructible_v<tagged_ptr<int,2>, int*, std::uintptr_t>);
    }

    const auto tag = std::uintptr_t{0b10};
    auto value = int{};
    auto sut = tagged_ptr<int,2>{&value, tag};

    SECTION("Pointer references input") {
      REQUIRE(sut.get() == &value);
    }
    SECTION("tagged_ptr is non-null") {
      REQUIRE(sut);
    }
    SECTION("tagged_ptr has specified tag") {
      REQUIRE(sut.tag() == tag);
    }
  }
}

//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::tagged_ptr(const tagged_ptr<U,Bits>&)") {
  SECTION("U is not convertible to T") {
    SECTION("tagged_ptr<T> is not constructible from U") {
      STATIC_REQUIRE_FALSE(std::is_constructible_v<tagged_ptr<int,2>, const tagged_ptr<long,2>&>);
    }
  }
  SECTION("U is convertible to T") {
    SECTION("U is only a CV-qualification different from T") {
      SECTION("tagged_ptr<const T> is constructible from T*") {
        STATIC_REQUIRE(std::is_constructible_v<tagged_ptr<const int,2>, const tagged_ptr<int,2>&>);
      }

      const auto tag = std::uintptr_t{0b10};
      auto value = int{};
      auto sut = tagged_ptr<const int,2>{&value, tag};

      SECTION("Pointer references input") {
        REQUIRE(sut.get() == &value);
      }
      SECTION("tagged_ptr is non-null") {
        REQUIRE(sut);
      }
      SECTION("tagged_ptr has specified tag") {
        REQUIRE(sut.tag() == tag);
      }
    }
    SECTION("U is base of T") {
      SECTION("tagged_ptr<base> is constructible from derived*") {
        STATIC_REQUIRE(std::is_constructible_v<tagged_ptr<base,2>, const tagged_ptr<derived,2>&>);
      }

      const auto tag = std::uintptr_t{0b10};
      auto value = derived{};
      auto copy = tagged_ptr<derived,2>{&value, tag};
      auto sut = tagged_ptr<base,2>{copy};

      SECTION("Pointer references input") {
        REQUIRE(sut.get() == copy.get());
      }
      SECTION("tagged_ptr is non-null") {
        REQUIRE(sut);
      }
      SECTION("tagged_ptr has specified tag") {
        REQUIRE(sut.tag() == tag);
      }
    }
  }
}

//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::operator(U*)") {
  SECTION("U is not convertible to T") {
    SECTION("tagged_ptr<T> is not constructible from U") {
      STATIC_REQUIRE_FALSE(std::is_assignable_v<tagged_ptr<int,2>, long*>);
    }
  }
  SECTION("U is convertible to T") {
    SECTION("U is only a CV-qualification different from T") {
      SECTION("tagged_ptr<const T> is constructible from T*") {
        STATIC_REQUIRE(std::is_assignable_v<tagged_ptr<const int,2>, int*>);
      }

      const auto tag = std::uintptr_t{0b10};
      auto old   = int{};
      auto value = int{};
      auto sut = tagged_ptr<const int,2>{&old, tag};

      sut = &value;

      SECTION("Pointer is rebound") {
        REQUIRE(sut.get() == &value);
      }
      SECTION("Tag is not changed") {
        REQUIRE(sut.tag() == tag);
      }
    }
    SECTION("U is base of T") {
      SECTION("tagged_ptr<base> is constructible from derived*") {
        STATIC_REQUIRE(std::is_assignable_v<tagged_ptr<base,2>, derived*>);
      }

      const auto tag = std::uintptr_t{0b10};
      auto old   = derived{};
      auto value = derived{};
      auto sut = tagged_ptr<base,2>{&old, tag};

      sut = &value;

      SECTION("Pointer is rebound") {
        REQUIRE(sut.get() == &value);
      }
      SECTION("Tag is not changed") {
        REQUIRE(sut.tag() == tag);
      }
    }
  }
}

TEST_CASE("tagged_ptr<T, Bits>::operator(std::nullptr_t)") {
  const auto tag = std::uintptr_t{0b10};
  auto old   = int{};
  auto value = int{};
  auto sut = tagged_ptr<const int,2>{&old, tag};

  sut = &value;

  SECTION("Pointer is rebound") {
    REQUIRE(sut.get() == &value);
  }
  SECTION("Tag is not changed") {
    REQUIRE(sut.tag() == tag);
  }
}

//------------------------------------------------------------------------------
// Modifiers
//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::tag(std::uintptr_t)", "[modifiers]") {
  SECTION("Tag exceeds bits") {
    SECTION("Only specified bits get set") {
      // Arrange
      const auto tag = 0b110;
      auto value = int{42};
      auto ptr = tagged_ptr<int,2>{&value};

      // Act
      ptr.tag(tag);

      // Assert
      REQUIRE(ptr.tag() == 0b10);
    }
  }
  SECTION("Tag can be set") {
    // Arrange
    const auto tag = 0b01;
    auto value = int{42};
    auto ptr = tagged_ptr<int,2>{&value};

    // Act
    ptr.tag(tag);

    // Assert
    REQUIRE(ptr.tag() == tag);
  }
}

//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::reset(pointer)", "[modifiers]") {
  // Arrange
  const auto tag = 0b10;
  auto value = int{42};
  auto other = int{0xdead};
  auto ptr = tagged_ptr<const int,2>{&value, tag};

  // Act
  ptr.reset(&other);

  SECTION("Resets the stored pointer") {
    REQUIRE(ptr.get() == &other);
  }
  SECTION("Tag is not reset") {
    REQUIRE(ptr.tag() == tag);
  }
}

TEST_CASE("tagged_ptr<T, Bits>::reset(std::nullptr_t)", "[modifiers]") {
  // Arrange
  const auto tag = 0b10;
  auto value = int{42};
  auto ptr = tagged_ptr<int,2>{&value, tag};

  // Act
  ptr.reset(nullptr);

  SECTION("Resets the stored pointer") {
    REQUIRE(ptr.get() == nullptr);
  }
  SECTION("Tag is not reset") {
    REQUIRE(ptr.tag() == tag);
  }
}


//------------------------------------------------------------------------------

TEST_CASE("tagged_ptr<T, Bits>::reset(pointer, std::uintptr_t)", "[modifiers]") {
  // Arrange
  const auto tag = 0b10;
  const auto other_tag = 0b01;
  auto value = int{42};
  auto other = int{0xdead};
  auto ptr = tagged_ptr<const int,2>{&value, tag};

  // Act
  ptr.reset(&other, other_tag);

  SECTION("Resets the stored pointer") {
    REQUIRE(ptr.get() == &other);
  }
  SECTION("Resets the tag") {
    REQUIRE(ptr.tag() == other_tag);
  }
}

TEST_CASE("tagged_ptr<T, Bits>::reset(std::nullptr_t, std::uintptr_t)", "[modifiers]") {
  // Arrange
  const auto tag = 0b10;
  const auto other_tag = 0b01;
  auto value = int{42};
  auto ptr = tagged_ptr<int,2>{&value, tag};

  // Act
  ptr.reset(nullptr, other_tag);

  SECTION("Resets the stored pointer") {
    REQUIRE(ptr.get() == nullptr);
  }
  SECTION("Resets the tag") {
    REQUIRE(ptr.tag() == other_tag);
  }
}

} // namespace msl::test
