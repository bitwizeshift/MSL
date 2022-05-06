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

#include "msl/memory/virtual_memory.hpp"

#include "msl/utilities/intrinsics.hpp"
#include "src/msl/memory/virtual_memory_impl.hpp"
#include <stdexcept>
#include <string>

//-----------------------------------------------------------------------------
// Static Functions
//-----------------------------------------------------------------------------

auto msl::virtual_memory::page_size()
  noexcept -> bytes
{
  return virtual_memory_page_size();
}

auto msl::virtual_memory::reserve(uquantity<page> pages)
  -> virtual_memory
{
  auto p = virtual_memory_reserve(pages.count());

  return virtual_memory{p.get(), pages};
}

//-----------------------------------------------------------------------------
// Constructors / Destructors / Assignment
//-----------------------------------------------------------------------------

msl::virtual_memory::virtual_memory(virtual_memory&& other)
  noexcept
  : m_data{std::exchange(other.m_data, nullptr)},
    m_pages{other.m_pages}
{

}

//-----------------------------------------------------------------------------

msl::virtual_memory::~virtual_memory()
{
  if (m_data != nullptr) {
    virtual_memory_release(assume_not_null(m_data), m_pages.count());
  }
}

//-----------------------------------------------------------------------------

auto msl::virtual_memory::operator=(virtual_memory&& other)
  noexcept -> virtual_memory&
{
  auto copy = std::move(other);
  copy.swap(*this);

  return (*this);
}

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

auto msl::virtual_memory::operator[](std::size_t n)
  -> page
{
  const auto p = m_data + (n * virtual_memory_page_size());

  return page::from_pointer_and_length(assume_not_null(p), page_size());
}

auto msl::virtual_memory::at(std::size_t n)
  -> page
{
  if (n >= m_pages) MSL_UNLIKELY {
    throw std::out_of_range{
      "virtual_memory::at: index out of range. " + std::to_string(n) +
      ">= " + std::to_string(m_pages.count())
    };
  }
  return (*this)[n];
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

auto msl::virtual_memory::commit(std::size_t n)
  -> page
{
  const auto size = page_size();
  const auto p = m_data + (n * size);

  return page::from_pointer_and_length(
    virtual_memory_commit(assume_not_null(p), 1u),
    size
  );
}

auto msl::virtual_memory::decommit(std::size_t n)
  -> void
{
  auto p = m_data + (n * page_size());

  virtual_memory_decommit(assume_not_null(p), 1u);
}
