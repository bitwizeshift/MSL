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
#ifndef MSL_MEMORY_VIRTUAL_MEMORY_HPP
#define MSL_MEMORY_VIRTUAL_MEMORY_HPP

#include "msl/blocks/memory_block.hpp"
#include "msl/quantities/digital_quantity.hpp"
#include "msl/quantities/quantity.hpp"
#include "msl/utilities/intrinsics.hpp"
#if defined(_MSC_VER)
# pragma once
#endif

namespace msl {

  class virtual_memory
  {
    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using page = memory_block;

    //-------------------------------------------------------------------------
    // Static Functions
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the size of a page in bytes
    ///
    /// \return the size of a page in bytes
    static auto page_size() noexcept -> bytes;

    /// \brief A factory function for producing virtual memory
    ///
    /// \param pages the number of pages to request
    /// \return the virtual memory, on success
    static auto reserve(uquantity<page> pages) -> virtual_memory;

    //-------------------------------------------------------------------------
    // Constructors / Destructor / Assignment
    //-------------------------------------------------------------------------
  public:

    virtual_memory(virtual_memory&& other) noexcept;

    virtual_memory(const virtual_memory&) = delete;

    //-------------------------------------------------------------------------

    ~virtual_memory();

    //-------------------------------------------------------------------------

    auto operator=(virtual_memory&& other) noexcept -> virtual_memory&;

    auto operator=(const virtual_memory& other) -> virtual_memory& = delete;

    //-------------------------------------------------------------------------
    // Element Access
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the underlying data pointer
    ///
    /// \return the pointer
    auto data() const noexcept -> std::byte*;

    auto operator[](std::size_t n) -> page;

    auto at(std::size_t n) -> page;

    //-------------------------------------------------------------------------
    // Capacity
    //-------------------------------------------------------------------------
  public:

    /// \brief Requests the size of this virtual memory in bytes
    ///
    /// \return the size of this in bytes
    auto size_in_bytes() const noexcept -> bytes;

    /// \brief Requests the number of pages made available by this virtual
    ///        memory
    ///
    /// \return the number of pages
    auto pages() const noexcept -> uquantity<page>;

    /// \brief Queries if this memory is empty
    ///
    /// \return `true` does not contain storage for any pages
    auto empty() const noexcept -> bool;

    //-------------------------------------------------------------------------
    // Modifiers
    //-------------------------------------------------------------------------
  public:

    /// \brief Commits the \p n'th page to virtual memory
    ///
    /// \param n the page number to commit
    auto commit(std::size_t n) -> memory_block;

    /// \brief Decommits the \p n'th page
    ///
    /// \param n the page number to decommit
    auto decommit(std::size_t n) -> void;

    /// \brief Releases the virtual memory controlled by this class
    ///
    /// The underlying data is \c nullptr after this call
    ///
    /// \return a pointer to the memory
    auto release() noexcept -> std::byte*;

    /// \brief Swaps this memory object with \p other
    ///
    /// \param other the other memory object to swap with
    auto swap(virtual_memory& other) noexcept -> void;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    std::byte* m_data;
    uquantity<page> m_pages;

    /// \brief Constructs the virtual memory from a data pointer and the number
    ///        of pages
    ///
    /// \param data the reserved pointer
    /// \param pages the number of pages
    virtual_memory(std::byte* data, uquantity<page> pages) noexcept;
  };

} // namespace msl

//-----------------------------------------------------------------------------
// Element Access
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE
auto msl::virtual_memory::data()
  const noexcept -> std::byte*
{
  return m_data;
}

//-----------------------------------------------------------------------------
// Capacity
//-----------------------------------------------------------------------------

inline
auto msl::virtual_memory::size_in_bytes()
  const noexcept -> bytes
{
  return page_size() * pages().count();
}

inline
auto msl::virtual_memory::pages()
  const noexcept -> uquantity<page>
{
  return m_pages;
}

inline
auto msl::virtual_memory::empty()
  const noexcept -> bool
{
  return pages() == 0u;
}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE
auto msl::virtual_memory::release()
  noexcept -> std::byte*
{
  return std::exchange(m_data, nullptr);
}

inline
auto msl::virtual_memory::swap(virtual_memory& other)
  noexcept -> void
{
  using std::swap;

  swap(m_data, other.m_data);
  swap(m_pages, other.m_pages);
}

//-----------------------------------------------------------------------------
// Private Constructors
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE
msl::virtual_memory::virtual_memory(std::byte* data, uquantity<page> pages)
  noexcept
  : m_data{data},
    m_pages{pages}
{

}

#endif /* MSL_MEMORY_VIRTUAL_MEMORY_HPP */