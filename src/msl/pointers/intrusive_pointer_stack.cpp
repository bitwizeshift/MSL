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
#include "msl/pointers/intrusive_pointer_stack.hpp"

//-----------------------------------------------------------------------------
// Lookup
//-----------------------------------------------------------------------------

auto msl::intrusive_pointer_stack::contains(const std::byte* p)
  const noexcept -> bool
{
  auto q = m_head;

  while (q != nullptr) {
    auto next = static_cast<std::byte**>(nullptr);
    std::memcpy(&next, q, sizeof(std::byte**));
    if (p == q) {
      return true;
    }
    q = *next;
  }
  return false;
}

//-----------------------------------------------------------------------------
// Capacity
//-----------------------------------------------------------------------------

auto msl::intrusive_pointer_stack::size()
  const noexcept -> uquantity<std::byte*>
{
  auto result = uquantity<std::byte*>{0u};

  auto p = m_head;

  // Count each non-null pointer
  while (p != nullptr) {
    auto next = static_cast<std::byte**>(nullptr);
    std::memcpy(&next, p, sizeof(std::byte**));
    p = *next;
    ++result;
  }
  return result;
}