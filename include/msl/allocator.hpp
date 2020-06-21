///////////////////////////////////////////////////////////////////////////////
/// \file allocator.hpp
///
/// \todo(Bitwize): Document this
///////////////////////////////////////////////////////////////////////////////

/*
  The MIT License (MIT)

  Copyright (c) 2020 Matthew Rodusek All rights reserved.

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
#ifndef MSL_ALLOCATOR_HPP
#define MSL_ALLOCATOR_HPP

#include "msl/cells/cell.hpp"
#include "msl/cells/active_cell.hpp"
#include "msl/cells/cell_activator.hpp"

#include "msl/utilities/digital_quantity.hpp"
#include "msl/utilities/quantity.hpp"

#include <cstddef> // std::byte

namespace msl {

  class allocator
  {
    //-------------------------------------------------------------------------
    // Allocation
    //-------------------------------------------------------------------------
  public:

    template <std::size_t Align = 1u>
    [[nodiscard]]
    cell<std::byte, Align> allocate_byte();

    template <std::size_t Align = 1u>
    [[nodiscard]]
    cell<std::byte[], Align> allocate_bytes(bytes length);

    template <std::size_t Align = 1u>
    [[nodiscard]]
    cell<std::byte[], Align> allocate_bytes(std::size_t length);

    template <std::size_t N, std::size_t Align = 1u>
    [[nodiscard]]
    cell<std::byte[N], Align> allocate_fixed_bytes();

    template <typename T, std::size_t Align = alignof(T)>
    [[nodiscard]]
    cell<T, Align> allocate_object();

    template <typename T, std::size_t Align = alignof(T)>
    [[nodiscard]]
    cell<T[], Align> allocate_objects(quantity<T> length);

    template <typename T, std::size_t Align = alignof(T)>
    [[nodiscard]]
    cell<T[], Align> allocate_objects(std::size_t length);

    template <typename T, std::size_t N, std::size_t Align = alignof(T)>
    [[nodiscard]]
    cell<T[N], Align> allocate_fixed_objects();

    //-------------------------------------------------------------------------
    // Deallocation
    //-------------------------------------------------------------------------
  public:

    template <typename T, std::size_t Align>
    void deallocate(cell<T, Align> c);

    template <typename T, std::size_t Align>
    void deallocate(cell<T[], Align> c);

    template <typename T, std::size_t N, std::size_t Align>
    void deallocate(cell<T[N], Align> c);

    //-------------------------------------------------------------------------
    // Allocation Resizing
    //-------------------------------------------------------------------------
  public:

    template <std::size_t Align>
    bool try_resize_bytes(cell<std::byte[], Align>& c, bytes new_length);
    template <std::size_t Align>
    bool try_resize_bytes(cell<std::byte[], Align>& c, std::size_t new_length);

    template <typename T, std::size_t Align>
    bool try_resize_objects(cell<T[], Align>& c,
                            typename cell<T[], Align>::length_type new_length);
    template <typename T, std::size_t Align>
    bool try_resize_objects(cell<T[], Align>& c, std::size_t new_length);

    //-------------------------------------------------------------------------
    // Reallocation
    //-------------------------------------------------------------------------
  public:

    template <typename T, std::size_t Align>
    cell<T[], Align> reallocate_objects(cell<T[], Align>&& c, std::size_t new_length);
    template <typename T, std::size_t Align>
    cell<T[], Align> reallocate_objects(cell<T[], Align>&& c, typename cell<T[], Align>::length_type new_length);

    template <std::size_t N, typename T, std::size_t OldN, std::size_t Align>
    cell<T[], Align> reallocate_fixed_objects(cell<T[OldN], Align>&& c, std::size_t new_length);
    template <std::size_t N, typename T, std::size_t OldN, std::size_t Align>
    cell<T[], Align> reallocate_fixed_objects(cell<T[OldN], Align>&& c, typename cell<T[OldN], Align>::length_type new_length);

    //-------------------------------------------------------------------------
    // Fused Allocation / Creation
    //-------------------------------------------------------------------------
  public:

    template <typename T, std::size_t Align=alignof(T), typename...Args>
    [[nodiscard]]
    active_cell<T, Align> make(Args&&...args);

    template <typename T, std::size_t Align=alignof(T)>
    [[nodiscard]]
    active_cell<T[], Align> make_array(std::size_t n);

    template <typename T, std::size_t Align=alignof(T)>
    [[nodiscard]]
    active_cell<T[], Align> make_array(quantity<T> n);

    template <typename T, std::size_t N, std::size_t Align=alignof(T)>
    [[nodiscard]]
    active_cell<T[N], Align> make_fixed_array();

    template <typename T, std::size_t Align>
    void dispose(active_cell<T, Align> c);

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    template <std::size_t Align>
    void deallocate_impl(const cell<std::byte[], Align>& c);

  };

} // namespace msl

//=============================================================================
// class : allocator
//=============================================================================

//-----------------------------------------------------------------------------
// Allocation
//-----------------------------------------------------------------------------

template <std::size_t Align>
inline msl::cell<std::byte, Align> msl::allocator::allocate_byte()
{
  const auto c = allocate_bytes<Align>(1u);

  // discard the length, since it's only 1 object
  return cell<std::byte, Align>{c.data()};
}

template <std::size_t Align>
inline msl::cell<std::byte[], Align> msl::allocator::allocate_bytes(bytes length)
{
  // TODO(bitwize): Implement in terms of memory_resource
  return {};
}

template <std::size_t Align>
inline msl::cell<std::byte[], Align> msl::allocator::allocate_bytes(std::size_t length)
{
  return allocate_bytes<T, Align>(bytes{length});
}

template <std::size_t N, std::size_t Align>
msl::cell<std::byte[N], Align> msl::allocator::allocate_fixed_bytes()
{
  const auto c = allocate_bytes<Align>(N);

  // discard the length, since we know the fixed size
  return cell<std::byte[N], Align>{c.data()};
}

template <typename T, std::size_t Align>
msl::cell<T, Align> msl::allocator::allocate_object()
{
  const auto c = allocate_bytes<Align>(size_of<T>());
  
  return msl::cell<T, Align>{reinterpret_cast<T*>(c.data())};
}

template <typename T, std::size_t Align>
msl::cell<T[], Align> msl::allocator::allocate_objects(std::size_t length) 
{
  return allocate_objects<T, Align>(quantity<T>(length));
}

template <typename T, std::size_t Align>
msl::cell<T[], Align> msl::allocator::allocate_objects(quantity<T> length)
{
  MSL_ASSERT(length >= 0);
  const auto c = allocate_bytes<Align>(size_of<T>() * length.count());

  return cell<T[], Align>{
    reinterpret_cast<T*>(c.data()), length
  };
}

template <typename T, std::size_t N, std::size_t Align>
msl::cell<T[N], Align> msl::allocator::allocate_fixed_objects()
{
  const auto c = allocate_objects<T, Align>(N);

  return cell<T[N], Align>{c.data()};
}

//-----------------------------------------------------------------------------
// Deallocation
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
void msl::allocator::deallocate(msl::cell<T, Align> c)
{
  const auto as_bytes = cell<std::byte[], Align>{
    reinterpret_cast<std::byte*>(c),
    c.size(),
  };

  deallocate(as_bytes);
}

template <typename T, std::size_t Align>
void msl::allocator::deallocate(msl::cell<T[], Align> c)
{
  const auto as_bytes = cell<std::byte[], Align>{
    reinterpret_cast<std::byte*>(c.data()),
    c.size(),
  };

  deallocate_impl(as_bytes);
}

template <typename T, std::size_t N, std::size_t Align>
void msl::allocator::deallocate(msl::cell<T[N], Align> c)
{
  const auto as_bytes = cell<std::byte[], Align>{
    reinterpret_cast<std::byte*>(c.data()),
    c.size(),
  };

  deallocate_impl(as_bytes);
}


template <std::size_t Align>
void msl::allocator::deallocate_impl(const msl::cell<std::byte[], Align>& c)
{
    // TODO(bitwize): Implement in terms of memory_resource
}

template <typename T, std::size_t Align, typename... Args>
inline msl::active_cell<T, Align> msl::allocator::make(Args&&...args)
{
  auto c = allocate_object<T, Align>();

  if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
    return cell_activator::activate(std::move(c), std::forward<Args>(args)...);
  } else {
    try {
      return cell_activator::activate(std::move(c), std::forward<Args>(args)...);
    } catch (...) {
      deallocate_object(std::move(c));
      throw;
    }
  }
}

template <typename T, std::size_t Align>
msl::active_cell<T[], Align> msl::allocator::make_array(std::size_t n)
{
  return make_array(quantity<T>(n));
}

template <typename T, std::size_t Align>
msl::active_cell<T[], Align> msl::allocator::make_array(quantity<T> n)
{
  MSL_ASSERT(length >= 0);

  auto c = allocate_objects<T, Align>(n);

  if constexpr (std::is_nothrow_default_constructible_v<T>) {
    return c.activate();
  } else {
    try {
      return c.activate();
    } catch (...) {
      deallocate(std::move(c));
      throw;
    }
  }
}

template <typename T, std::size_t N, std::size_t Align>
msl::active_cell<T[N], Align> msl::allocator::make_fixed_array()
{
  auto c = make_array<T, Align>(N);

  return active_cell<T[N], Align>{c.data()};
}

template <typename T, std::size_t Align>
void msl::allocator::dispose(msl::active_cell<T, Align> c)
{
  deallocate(cell_activator::deactivate(std::move(c)));
}

template <std::size_t Align>
bool msl::allocator::try_resize_bytes(msl::cell<std::byte[], Align>& c, bytes new_length)
{
  MSL_ASSERT(new_length.count() >= 0);
  // TODO(bitwizeshift): Implement in terms of memory_resource
  return false;
}

template <std::size_t Align>
bool msl::allocator::try_resize_bytes(cell<std::byte[], Align>& c, std::size_t new_length)
{
  return try_resize_bytes(c, bytes(new_length));
}

template <typename T, std::size_t Align>
bool msl::allocator::try_resize_objects(cell<T[], Align>& c, typename cell<T[], Align>::length_type new_length)
{
  MSL_ASSERT(new_length.count() >= 0);
  auto bytes_cell = cell<std::byte[], Align>{reinterpret_cast<std::byte*>(c.data()), c.size()};

  const auto success = try_resize_bytes(bytes_cell, new_length.count());

  if (success) {
    c = cell<T[], Align>{c.data(), new_length};
  }
  return success;
}

template <typename T, std::size_t Align>
bool msl::allocator::try_resize_objects(msl::cell<T[], Align>& c, std::size_t new_length)
{
  return try_resize_objects(c, bytes(new_length));
}

#endif /* MSL_ALLOCATOR_HPP */
