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
#ifndef SRC_MSL_MEMORY_VIRTUAL_MEMORY_IMPL_HPP
#define SRC_MSL_MEMORY_VIRTUAL_MEMORY_IMPL_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include "msl/quantities/digital_quantity.hpp"
#include "msl/pointers/not_null.hpp"

#include <cstddef>

namespace msl {

  /// \brief Retrieves the page size of the virtual memory
  ///
  /// \return the page size of the virtual memory
  auto virtual_memory_page_size() noexcept -> bytes;

  //--------------------------------------------------------------------------

  /// \brief Reserves \p n pages of virtual memory.
  ///
  /// Each page is \ref virtual_memory_page_size in size. The memory may not
  /// be used until after it has first been commited with a call to
  /// \ref virtual_memory_commit
  ///
  /// \param n the number of pages to reserve
  /// \return pointer to the reserved memory
  auto virtual_memory_reserve(std::size_t n) -> not_null<std::byte*>;

  /// \brief Commits \p n pages of memory to virtual memory
  ///
  /// \param memory Memory pointing to a page to commit
  /// \param n The number of pages to commit
  /// \return pointer to the committed memory
  auto virtual_memory_commit(not_null<std::byte*> memory, std::size_t n) -> not_null<std::byte*>;

  /// \brief Decommits \p n pages of memory to virtual memory
  ///
  /// \param memory Memory pointing to a page to decommit
  /// \param n The number of pages to decommit
  auto virtual_memory_decommit(not_null<std::byte*> memory, std::size_t n) -> void;

  /// \brief Releases \p n pages of virtual memory
  ///
  /// \param memory The memory originally returned from virtual_memory_reserve
  /// \param n The number of pages to release
  auto virtual_memory_release(not_null<std::byte*> memory, std::size_t n) -> void;

} // namespace msl

#endif /* SRC_MSL_MEMORY_VIRTUAL_MEMORY_IMPL_HPP */