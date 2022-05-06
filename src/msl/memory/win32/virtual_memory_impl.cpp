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

#include "src/msl/memory/win32/windows.hpp"
#include "msl/utilities/intrinsics.hpp"

#include <string>
#include <system_error>

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
      ::SYSTEM_INFO system_info;
      ::GetSystemInfo(&system_info);

      return bytes{static_cast<std::size_t>(system_info.dwPageSize)};
    }

    class windows_error_category : public std::error_category
    {
    public:

      auto name() const noexcept -> const char* override
      {
        return "windows error code";
      }

      auto message(int error) const -> std::string override
      {
        auto buffer = static_cast<char*>(nullptr);
        auto result = std::string{};

        const auto flags = (
          FORMAT_MESSAGE_ALLOCATE_BUFFER |
          FORMAT_MESSAGE_FROM_SYSTEM |
          FORMAT_MESSAGE_IGNORE_INSERTS
        );

        if (::FormatMessage(flags, nullptr, error, 0, buffer, 0, nullptr)) {
          result = std::string{buffer};
        }
        ::LocalFree(buffer);
        return result;
      }
    };

    constinit auto g_error_category = windows_error_category{};

    [[noreturn]]
    auto throw_system_error() -> void {
      const auto code = std::error_code{::GetLastError(), g_error_category};
      throw std::system_error{code};
    }

  } // namespace <anonymous>
} // namespace msl

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
  const auto p = ::VirtualAlloc(
    nullptr,
    size.count(),
    MEM_RESERVE,
    PAGE_NOACCESS
  );

  if (p == nullptr) MSL_UNLIKELY {
    throw_system_error();
  }

  return assume_not_null(static_cast<std::byte*>(p));
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_commit(not_null<std::byte*> memory, std::size_t n)
  -> not_null<std::byte*>
{
  const auto size = n * virtual_memory_page_size();
  const auto region = ::VirtualAlloc(
    memory.get(),
    size.count(),
    MEM_COMMIT,
    PAGE_READWRITE
  );

  if (region == nullptr) MSL_UNLIKELY {
    throw_system_error();
  }

  return assume_not_null(region);
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_decommit(not_null<std::byte*> memory, std::size_t n)
  -> void
{
  const auto size = n * virtual_memory_page_size();
  const auto result = ::VirtualFree(memory.get(), size.count(), MEM_DECOMMIT);

  if (result == 0) MSL_UNLIKELY {
    throw_system_error();
  }
}

//--------------------------------------------------------------------------

auto msl::virtual_memory_release(not_null<std::byte*> memory, std::size_t n)
  -> void
{
  const auto result = ::VirtualFree(memory.get(), 0u, MEM_RELEASE);

  if (result == 0) MSL_UNLIKELY {
    throw_system_error();
  }
}
