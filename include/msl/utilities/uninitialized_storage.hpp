///////////////////////////////////////////////////////////////////////////////
/// \file uninitialized_storage.hpp
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
#ifndef MSL_UTILITIES_UNINITIALIZED_STORAGE_HPP
#define MSL_UTILITIES_UNINITIALIZED_STORAGE_HPP

#include "not_null.hpp"

#include <new>         // placement new
#include <cstddef>     // std::size_t
#include <iterator>    // std::iterator_traits, std::reverse_iterator
#include <type_traits> // std::is_nothrow_constructible_v
#include <utility>     // std::index_sequence, std::forward

namespace msl {

  //===========================================================================
  // static class : uninitialized_storage
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief Static class containing utilities for dealing with uninitialized
  ///        storage.
  /////////////////////////////////////////////////////////////////////////////
  class uninitialized_storage
  {
    uninitialized_storage() = delete;
    ~uninitialized_storage() = delete;

    //-------------------------------------------------------------------------
    // Construction
    //-------------------------------------------------------------------------
  public:

    /// \brief Constructs an instance of type \p T with the given \p args
    ///        at the memory location specified in \p ptr
    ///
    /// \tparam T the type to construct
    /// \param p The memory location to construct into
    /// \param args... The arguments to supply to T's constructor
    /// \return Pointer to the initialized memory (cast of \p ptr)
    template <typename T, typename...Args>
    static T* construct_at(void* p, Args&&...args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>);

    /// \brief Constructs an array of the specified size at pointer \p p
    ///
    /// \tparam T the type to construct
    /// \param p the pointer to construct the array at
    /// \param n the size of the array
    /// \return the pointer to the constructed array
    template <typename T>
    static T* construct_array_at(void* p, std::size_t n)
      noexcept(std::is_nothrow_constructible_v<T>);

    /// \brief Constructs an array of the specified size at pointer \p p by
    ///        copying \p copy to each array member
    ///
    /// \tparam T the type to construct
    /// \tparam p the pointer to construct the array at
    /// \param n the size of the array
    /// \param copy the instance to copy to each array member
    /// \return the pointer to the constructed array
    template<typename T>
    static T* construct_array_at(void* p, std::size_t n, const T& copy)
      noexcept(std::is_nothrow_constructible_v<T, const T&>);

    /// \brief Constructs an instance of type \p T from a given \p tuple
    ///
    /// \tparam T the type to construct
    /// \param p the pointer to construct the type at
    /// \param tuple the tuple to unpack as arguments to \p T
    /// \return pointer to the constructed element
    template<typename T, typename Tuple>
    static T* construct_from_tuple_at(void* p, Tuple&& tuple);

    //-------------------------------------------------------------------------
    // Destruction
    //-------------------------------------------------------------------------

    /// \brief Destroys the type at the given pointer \p p
    ///
    /// \tparam T the type to destroy
    /// \param p the pointer to the instance to destroy
    template <typename T>
    static void destroy_at(not_null<T*> p);

    /// \brief Destroys the array of instances at the specified pointer \p p
    ///
    /// \tparam T the type to destroy
    /// \param p the pointer to the array to destroy
    /// \param n the size of the array
    template <typename T>
    static void destroy_array_at(not_null<T*> p, std::size_t n);

    /// \brief Destroys all objects denoted by the range of iterators
    ///
    /// This calls the destructor on each instance in the valid range.
    ///
    /// \param first iterator to the start of the range
    /// \param last iterator to the end of the range
    template <typename InputIt>
    static void destroy_range(InputIt first, InputIt last);

    //-------------------------------------------------------------------------
    // Private Construction
    //-------------------------------------------------------------------------
  private:

    template <typename T, typename Tuple, std::size_t...Idx>
    static T* construct_from_tuple_at_impl(void* p, Tuple&& tuple, std::index_sequence<Idx...>);

    template <typename T, typename...Args>
    static T* construct_array_at_impl(void* p, std::size_t n, Args&&...args)
      noexcept(std::is_nothrow_constructible_v<T,Args...>);
  };

} // namespace msl

//=============================================================================
// definitions : static class : uninitialized_storage
//=============================================================================

//-----------------------------------------------------------------------------
// Construction
//-----------------------------------------------------------------------------

template <typename T, typename... Args>
inline T* msl::uninitialized_storage::construct_at(void* p, Args&&...args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>)
{
  return ::operator new(p) T(std::forward<Args>(args)...);
}

template <typename T>
inline T* msl::uninitialized_storage::construct_array_at(void* p, std::size_t n)
  noexcept(std::is_nothrow_constructible_v<T>)
{
  return construct_array_at_impl<T>(p, n);
}

template <typename T>
inline T* msl::uninitialized_storage::construct_array_at(void* p, std::size_t n, const T& copy)
  noexcept(std::is_nothrow_constructible_v<T, const T&>)
{
  return construct_array_at_impl<T>(p, n, copy);
}


template <typename T, typename Tuple>
inline T* msl::uninitialized_storage::construct_from_tuple_at(void* p, Tuple&& tuple)
{
  return construct_from_tuple_at_impl<T>(
    p,
    std::forward<Tuple>(tuple),
    std::make_index_sequence<std::tuple_size<Tuple>::value>{}
  );
}

//-----------------------------------------------------------------------------
// Destruction
//-----------------------------------------------------------------------------

template <typename T>
inline void msl::uninitialized_storage::destroy_at(not_null<T*> p)
{
  p->~T();
}

template <typename T>
inline void msl::uninitialized_storage::destroy_array_at(not_null<T*> p, std::size_t n)
{
  auto* const first = p;
  auto* const last = p + n;

  destroy_range(first, last);
}

template <typename InputIt>
inline void msl::uninitialized_storage::destroy_range(InputIt first, InputIt last)
{
  using value_type = typename std::iterator_traits<InputIt>::value_type;

  for (auto it = first; it != last; ++it) {
    it->~value_type();
  }
}

//-----------------------------------------------------------------------------
// Private Construction
//-----------------------------------------------------------------------------

template <typename T, typename Tuple, size_t... Idx>
inline T* msl::uninitialized_storage::construct_from_tuple_at_impl(void* p,
                                                                   Tuple&& tuple,
                                                                   std::index_sequence<Idx...>)
{
  return construct_at<T>(p, std::get<Idx>(std::forward<Tuple>(tuple))...);
}


template <typename T, typename... Args>
inline T* msl::uninitialized_storage::construct_array_at_impl(void* p,
                                                              std::size_t n,
                                                              Args&&...args)
  noexcept(std::is_nothrow_constructible_v<T,Args...>)
{
  auto* const first = static_cast<T*>(p);
  auto* const last  = result + n;

  // capture 'result' so we can return a valid constructed object without
  // requiring std::launder
  auto* const result = construct_at<T>(first, std::forward<Args>(args)...);

  auto* current = result + 1;
  if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
    for (; current != last; ++current) {
      construct_at<T>(current, std::forward<Args>(args)...);
    }
  } else {
    try {
      for (; current != last; ++current) {
        construct_at<T>(current, std::forward<Args>(args)...);
      }
    } catch (...) {
      // If an exception happens, call destructors in reverse order starting
      // with the last successfully constructed object.
      destroy_range(
        std::make_reverse_iterator(current),
        std::make_reverse_iterator(first)
      );
      throw;
    }
  }

  return result;
}

#endif /* MSL_UTILITIES_UNINITIALIZED_STORAGE_HPP */
