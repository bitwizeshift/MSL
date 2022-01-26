///////////////////////////////////////////////////////////////////////////////
/// \file uninitialized_storage.hpp
///
/// \todo(Bitwize): Document this
///////////////////////////////////////////////////////////////////////////////

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
#ifndef MSL_UTILITIES_UNINITIALIZED_STORAGE_HPP
#define MSL_UTILITIES_UNINITIALIZED_STORAGE_HPP

#include "msl/utilities/intrinsics.hpp" // MSL_LIKELY

#include <new>         // placement new
#include <cstddef>     // std::size_t
#include <iterator>    // std::iterator_traits, std::reverse_iterator
#include <ranges>      // std::ranges::begin
#include <iterator>    // std::input_iterator
#include <type_traits> // std::is_nothrow_constructible_v
#include <utility>     // std::index_sequence, std::forward

namespace msl {

  //============================================================================
  // static class : uninitialized_storage
  //============================================================================

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Static class containing utilities for dealing with uninitialized
  ///        storage.
  ///
  /// This class offers a variety of utilities for constructng objects at a
  /// given memory location, with proper exception handling safety
  //////////////////////////////////////////////////////////////////////////////
  class uninitialized_storage
  {
    uninitialized_storage() = delete;
    ~uninitialized_storage() = delete;

    //--------------------------------------------------------------------------
    // Construction
    //--------------------------------------------------------------------------
  public:

    /// \brief Constructs an instance of type \p T with the given \p args
    ///        at the memory location specified in \p ptr
    ///
    /// \tparam T the type to construct
    /// \param p The memory location to construct into
    /// \param args... The arguments to supply to T's constructor
    /// \return Pointer to the initialized memory (cast of \p ptr)
    template <typename T, typename...Args>
    [[nodiscard]]
    static auto construct_at(void* p, Args&&...args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>) -> T*;

    /// \brief Constructs an array of the specified size at pointer \p p
    ///
    /// \tparam T the type to construct
    /// \param p the pointer to construct the array at
    /// \param n the size of the array
    /// \return the pointer to the constructed array
    template <typename T>
    [[nodiscard]]
    static auto construct_array_at(void* p, std::size_t n)
      noexcept(std::is_nothrow_constructible_v<T>) -> T*;

    /// \brief Constructs an array of the specified size at pointer \p p by
    ///        copying \p copy to each array member
    ///
    /// \tparam T the type to construct
    /// \tparam p the pointer to construct the array at
    /// \param n the size of the array
    /// \param copy the instance to copy to each array member
    /// \return the pointer to the constructed array
    template <typename T, typename U>
    [[nodiscard]]
    static auto construct_array_at(void* p, std::size_t n, const U& copy)
      noexcept(std::is_nothrow_constructible_v<T, const U&>) -> T*;

    /// \brief Constructs an instance of type \p T from a given \p tuple
    ///
    /// \tparam T the type to construct
    /// \param p the pointer to construct the type at
    /// \param tuple the tuple to unpack as arguments to \p T
    /// \return pointer to the constructed element
    template <typename T, typename Tuple>
    [[nodiscard]]
    static auto construct_from_tuple_at(void* p, Tuple&& tuple) -> T*;

    //--------------------------------------------------------------------------
    // Destruction
    //--------------------------------------------------------------------------

    /// \brief Destroys the type at the given pointer \p p
    ///
    /// \tparam T the type to destroy
    /// \param p the pointer to the instance to destroy
    template <typename T>
    static auto destroy_at(T* p)
      noexcept(std::is_nothrow_destructible_v<T>) -> void;

    /// \brief Destroys the array of instances at the specified pointer \p p
    ///
    /// \tparam T the type to destroy
    /// \param p the pointer to the array to destroy
    /// \param n the size of the array
    template <typename T>
    static auto destroy_array_at(T* p, std::size_t n)
      noexcept(std::is_nothrow_destructible_v<T>) -> void;

    /// \brief Destroys all objects denoted by the range of iterators
    ///
    /// This calls the destructor on each instance in the valid range.
    ///
    /// \param first iterator to the start of the range
    /// \param last iterator to the end of the range
    template <typename InputIt, typename Sentinel>
    static auto destroy_range(InputIt first, Sentinel last)
      noexcept(std::is_nothrow_destructible_v<std::iter_value_t<InputIt>>) -> void
      requires(std::input_iterator<InputIt> && std::sentinel_for<Sentinel,InputIt>);

    /// \brief Destroys all objects denoted by a given range
    ///
    /// This calls the destructor on each instance in the valid range.
    ///
    /// \param range the range
    template <typename Range>
    static auto destroy_range(Range&& range)
      noexcept(std::is_nothrow_destructible_v<std::ranges::range_value_t<std::decay_t<Range>>>) -> void
      requires(std::ranges::range<Range>);

    //--------------------------------------------------------------------------
    // Private Construction
    //--------------------------------------------------------------------------
  private:

    template <typename T, typename Tuple, std::size_t...Idx>
    static auto construct_from_tuple_at_impl(
      void* p,
      Tuple&& tuple, std::index_sequence<Idx...>
    ) -> T*;

    template <typename T, typename...Args>
    static auto construct_array_at_impl(
      void* p,
      std::size_t n,
      Args&&...args
    ) noexcept(std::is_nothrow_constructible_v<T,Args...>) -> T*;
  };

} // namespace msl

//==============================================================================
// definitions : static class : uninitialized_storage
//==============================================================================

//------------------------------------------------------------------------------
// Construction
//------------------------------------------------------------------------------

template <typename T, typename... Args>
inline
auto msl::uninitialized_storage::construct_at(void* p, Args&&...args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>) -> T*
{
  return new (p) T(std::forward<Args>(args)...);
}

template <typename T>
inline
auto msl::uninitialized_storage::construct_array_at(void* p, std::size_t n)
  noexcept(std::is_nothrow_constructible_v<T>) -> T*
{
  return construct_array_at_impl<T>(p, n);
}

template <typename T, typename U>
inline
auto msl::uninitialized_storage::construct_array_at(void* p,
                                                    std::size_t n,
                                                    const U& copy)
  noexcept(std::is_nothrow_constructible_v<T, const U&>) -> T*
{
  return construct_array_at_impl<T>(p, n, copy);
}


template <typename T, typename Tuple>
inline
auto msl::uninitialized_storage::construct_from_tuple_at(void* p,
                                                         Tuple&& tuple)
  -> T*
{
  return construct_from_tuple_at_impl<T>(
    p,
    std::forward<Tuple>(tuple),
    std::make_index_sequence<std::tuple_size<Tuple>::value>{}
  );
}

//------------------------------------------------------------------------------
// Destruction
//------------------------------------------------------------------------------

template <typename T>
inline
auto msl::uninitialized_storage::destroy_at(T* p)
  noexcept(std::is_nothrow_destructible_v<T>) -> void
{
  // The 'is_trivially_destructible' check here is probably unnecessary, since
  // manually invoking a destructor of a trivial type should be a no-op anyway.
  if constexpr (!std::is_trivially_destructible_v<T>) {
    p->~T();
  }
}

template <typename T>
inline
auto msl::uninitialized_storage::destroy_array_at(T* p, std::size_t n)
  noexcept(std::is_nothrow_destructible_v<T>) -> void
{
  auto const first = p;
  auto const last  = first + n;

  destroy_range(first, last);
}

//------------------------------------------------------------------------------

template <typename InputIt, typename Sentinel>
inline
auto msl::uninitialized_storage::destroy_range(InputIt first, Sentinel last)
  noexcept(std::is_nothrow_destructible_v<std::iter_value_t<InputIt>>) -> void
  requires(std::input_iterator<InputIt> && std::sentinel_for<Sentinel,InputIt>)
{
  using value_type = std::iter_value_t<InputIt>;

  if constexpr (!std::is_trivially_destructible_v<value_type>) {
    for (auto it = first; it != last; ++it) {
      it->~value_type();
    }
  }
}

template <typename Range>
auto msl::uninitialized_storage::destroy_range(Range&& range)
  noexcept(std::is_nothrow_destructible_v<std::ranges::range_value_t<std::decay_t<Range>>>) -> void
  requires(std::ranges::range<Range>)
{
  destroy_range(
    std::ranges::begin(std::forward<Range>(range)),
    std::ranges::end(std::forward<Range>(range))
  );
}

//------------------------------------------------------------------------------
// Private Construction
//------------------------------------------------------------------------------

template <typename T, typename Tuple, size_t... Idx>
inline
auto msl::uninitialized_storage::construct_from_tuple_at_impl(
  void* p,
  Tuple&& tuple,
  std::index_sequence<Idx...>
) -> T*
{
  return construct_at<T>(p, std::get<Idx>(std::forward<Tuple>(tuple))...);
}


template <typename T, typename... Args>
inline
auto msl::uninitialized_storage::construct_array_at_impl(
  void* p,
  std::size_t n,
  Args&&...args
) noexcept(std::is_nothrow_constructible_v<T,Args...>) -> T*
{
  auto const first = static_cast<T*>(p);
  auto const last  = first + n;

  // capture 'result' so we can return a valid constructed object without
  // requiring std::launder
  auto const result = construct_at<T>(first, std::forward<Args>(args)...);

  auto current = result + 1;
  if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
    for (; current != last; ++current) MSL_LIKELY {
      [[maybe_unused]]
      const auto r = construct_at<T>(
        current,
        std::forward<Args>(args)...
      );
    }
  } else {
    // Attempt to construct n elements
    try {
      for (; current != last; ++current) MSL_LIKELY {
        [[maybe_unused]]
        const auto r = construct_at<T>(
          current,
          std::forward<Args>(args)...
        );
      }
    } catch (...) {
      // If an exception happens, call destructors in reverse order starting
      // with the last successfully constructed object.
      MSL_UNLIKELY
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
