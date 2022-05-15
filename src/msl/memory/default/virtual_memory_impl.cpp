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
#include "src/msl/memory/virtual_memory_impl.hpp"
#include "msl/utilities/intrinsics.hpp"

#include <stdexcept>

namespace msl {
  class not_implemented : public std::runtime_error
  {
  public:
    using runtime_error::runtime_error;
  };
} // namespace msl

//--------------------------------------------------------------------------
// Free Functions
//--------------------------------------------------------------------------

auto msl::virtual_memory_page_size()
  noexcept -> bytes
{
  return bytes::zero();
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_reserve(std::size_t n)
  -> not_null<std::byte*>
{
  intrinsics::suppress_unused(n);

  throw not_implemented{"virtual_memory_reserve not implemented for target system"};
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_commit(not_null<std::byte*> memory, std::size_t n)
  -> not_null<std::byte*>
{
  intrinsics::suppress_unused(memory, n);

  throw not_implemented{"virtual_memory_commit not implemented for target system"};
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_decommit(not_null<std::byte*> memory, std::size_t n)
  -> void
{
  intrinsics::suppress_unused(memory, n);

  throw not_implemented{"virtual_memory_decommit not implemented for target system"};
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_release(not_null<std::byte*> memory, std::size_t n)
  -> void
{
  intrinsics::suppress_unused(memory, n);

  throw not_implemented{"virtual_memory_release not implemented for target system"};
}
