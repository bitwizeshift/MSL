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
#include "msl/utilities/assert.hpp"
#include "msl/utilities/intrinsics.hpp"

#include <sys/errno.h>
#include <sys/mman.h> // ::mmap
#include <system_error>
#include <unistd.h>   // ::sysconf

//--------------------------------------------------------------------------
// Global Constants
//--------------------------------------------------------------------------

#ifndef _SC_PAGESIZE
# error _SC_PAGESIZE must be defined to determine virtual page size
#endif

//--------------------------------------------------------------------------
// Forward Declarations
//--------------------------------------------------------------------------

namespace msl {
  namespace {
    /// \brief Determines the virtual page size on posix
    ///
    /// \return the virtual page size
    auto get_virtual_page_size()
      noexcept -> bytes
    {
      return bytes{static_cast<std::size_t>(::sysconf(_SC_PAGESIZE))};
    }
  }

  [[noreturn]]
  auto throw_system_error() -> void {
    // std::system_category is written in terms of POSIX categoriy, so this
    // should always be correct.
    const auto code = std::error_code{errno, std::system_category()};
    throw std::system_error{code};
  }
}

//--------------------------------------------------------------------------
// Free Functions
//--------------------------------------------------------------------------

auto msl::virtual_memory_page_size()
  noexcept -> bytes
{
  // Cache on first-entry
  static const auto s_page_size = get_virtual_page_size();

  return s_page_size;
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_reserve(std::size_t n)
  -> not_null<std::byte*>
{
  const auto size = n * virtual_memory_page_size();
  const auto protection = MAP_PRIVATE | MAP_ANONYMOUS;
  const auto p = ::mmap(nullptr, size.count(), PROT_NONE, protection, -1, 0);

  if (p == MAP_FAILED) MSL_UNLIKELY {
    throw_system_error();
  }

  return assume_not_null(static_cast<std::byte*>(p));
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_commit(not_null<std::byte*> memory, std::size_t n)
  -> not_null<std::byte*>
{
  const auto size = n * virtual_memory_page_size();
  const auto protection = PROT_WRITE | PROT_READ;
  const auto result = ::mprotect(memory.get(), size.count(), protection);

  if (result != 0) MSL_UNLIKELY {
    throw_system_error();
  }

// advise that the memory will be needed
#if defined(MADV_WILLNEED)
  ::madvise(memory.get(), size.count(), MADV_WILLNEED);
#elif defined(POSIX_MADV_WILLNEED)
  ::posix_madvise(memory.get(), size.count(), POSIX_MADV_WILLNEED);
#endif

  return memory;
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_decommit(not_null<std::byte*> memory, std::size_t n)
  -> void
{
  const auto size = n * virtual_memory_page_size();

#if defined(MADV_FREE)
  ::madvise(memory.get(), size.count(), MADV_FREE);
#elif defined(MADV_DONTNEED)
  ::madvise(memory.get(), size.count(), MADV_DONTNEED);
#elif defined(POSIX_MADV_DONTNEED)
  ::posix_madvise(memory.get(), size.count(), POSIX_MADV_DONTNEED);
#endif

  const auto result = ::mprotect(memory.get(), size.count(), PROT_NONE);

  if (result == 0) MSL_UNLIKELY {
    throw_system_error();
  }
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_release(not_null<std::byte*> memory, std::size_t n)
  -> void
{
  const auto size = n * virtual_memory_page_size();

  errno = 0;
  const auto result = ::munmap(memory.get(), size.count());

  if (result == 0) MSL_UNLIKELY {
    throw_system_error();
  }
}
