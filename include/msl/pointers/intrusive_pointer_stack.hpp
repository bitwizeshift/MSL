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
#ifndef MSL_MEMORY_INTRUSIVE_POINTER_STACK_HPP
#define MSL_MEMORY_INTRUSIVE_POINTER_STACK_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include "msl/pointers/not_null.hpp"    // not_null
#include "msl/quantities/quantity.hpp"  // uquantity
#include "msl/utilities/assert.hpp"     // MSL_ASSERT
#include "msl/utilities/intrinsics.hpp" // MSL_FORCE_INLINE

#include <cstddef> // std::byte
#include <cstring> // std::memcpy
#include <utility> // std::exchange

namespace msl {

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A container of pointers, all stored intrusively within the
  ///        memory region in which it points to
  /////////////////////////////////////////////////////////////////////////////
  class intrusive_pointer_stack
  {
    //-------------------------------------------------------------------------
    // Constructors
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs an intrusive pointer stack
    constexpr intrusive_pointer_stack() = default;

    constexpr intrusive_pointer_stack(intrusive_pointer_stack&& other) noexcept;

    intrusive_pointer_stack(const intrusive_pointer_stack&) = delete;

    //-------------------------------------------------------------------------

    auto operator=(const intrusive_pointer_stack&) -> intrusive_pointer_stack& = delete;
    auto operator=(intrusive_pointer_stack&&) -> intrusive_pointer_stack& = delete;

    //-------------------------------------------------------------------------
    // Modifiers
    //-------------------------------------------------------------------------
  public:

    /// \brief Pushes a single pointer \p p into this intrusive pointer stack
    ///
    /// \pre \p p must point to a writeable region of memory
    ///
    /// \post size is increased by 1
    /// \post `peek()` returns `p`
    /// \param p the pointer
    auto push(not_null<std::byte*> p) noexcept -> void;

    /// \brief Pops the top element from the intrusive pointer stack
    ///
    /// \pre `!empty()`
    /// \post size is decreased by 1
    /// \post the top element is changed
    auto pop() noexcept -> void;

    /// \brief Resets the state of this intrusive pointer stack to construction
    ///        state
    constexpr auto reset() noexcept -> void;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Peeks the top element of this intrusive pointer stack
    ///
    /// \return the top element of this pointer stack
    auto peek() const noexcept -> std::byte*;

    /// \brief Queries whether this pointer exists within the stack
    ///
    /// \param p the pointer to query
    /// \return true if this pointer exists within the stack
    auto contains(const std::byte* p) const noexcept -> bool;

    //-------------------------------------------------------------------------
    // Capacity
    //-------------------------------------------------------------------------
  public:

    /// \brief Queries the number of entries in this pointer stack
    ///
    /// \warning This function operates in O(n) time. In general, it's not ideal
    ///          to call this function unless necessary for debugging
    ///
    /// \return the number of elements in this stack
    auto size() const noexcept -> uquantity<std::byte*>;

    /// \brief Queries whether this stack contains any elements
    ///
    /// \true if this
    constexpr auto empty() const noexcept -> bool;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    std::byte* m_head = nullptr;
  };

} // namespace msl

//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE constexpr
msl::intrusive_pointer_stack::intrusive_pointer_stack(intrusive_pointer_stack&& other)
  noexcept
  : m_head{std::exchange(other.m_head, nullptr)}
{

}

//-----------------------------------------------------------------------------
// Modifiers
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE
auto msl::intrusive_pointer_stack::push(not_null<std::byte*> p)
  noexcept -> void
{
  std::memcpy(p.as_nullable(), &m_head, sizeof(std::byte*));
  m_head = p.as_nullable();
}

MSL_FORCE_INLINE
auto msl::intrusive_pointer_stack::pop()
  noexcept -> void
{
  MSL_ASSERT(m_head != nullptr);

  auto result = static_cast<std::byte**>(nullptr);
  std::memcpy(&result, m_head, sizeof(std::byte*));

  m_head = (*result);
}

MSL_FORCE_INLINE constexpr
auto msl::intrusive_pointer_stack::reset()
  noexcept -> void
{
  m_head = nullptr;
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE
auto msl::intrusive_pointer_stack::peek()
  const noexcept -> std::byte*
{
  return m_head;
}

//-----------------------------------------------------------------------------
// Capacity
//-----------------------------------------------------------------------------

MSL_FORCE_INLINE constexpr
auto msl::intrusive_pointer_stack::empty()
  const noexcept -> bool
{
  return m_head == nullptr;
}

#endif /* MSL_MEMORY_INTRUSIVE_POINTER_STACK_HPP */