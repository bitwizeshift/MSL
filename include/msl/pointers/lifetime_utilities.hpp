///////////////////////////////////////////////////////////////////////////////
/// \file lifetime_utilities.hpp
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
#ifndef MSL_POINTERS_LIFETIME_UTILITIES_HPP
#define MSL_POINTERS_LIFETIME_UTILITIES_HPP

#include "msl/pointers/not_null.hpp"
#include "msl/pointers/pointer_utilities.hpp"   // pointer_utilities::is_aligned
#include "msl/pointers/traversal_utilities.hpp" // traversal_utilities::advance
#include "msl/quantities/alignment.hpp"         // alignment::of
#include "msl/utilities/intrinsics.hpp"         // MSL_LIKELY
#include "msl/utilities/assert.hpp"             // MSL_ASSERT

#include <memory>      // std::construct_at
#include <new>         // placement new
#include <cstddef>     // std::size_t
#include <iterator>    // std::iterator_traits, std::reverse_iterator
#include <ranges>      // std::ranges::begin
#include <iterator>    // std::input_iterator
#include <type_traits> // std::is_nothrow_constructible_v
#include <utility>     // std::index_sequence, std::forward

namespace msl {

  //============================================================================
  // static class : lifetime_utilities
  //============================================================================

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Static class containing utilities for dealing with lifetimes of
  ///        objects.
  ///
  /// This class offers a variety of utilities for constructng objects at a
  /// given memory location, with proper exception handling safety
  //////////////////////////////////////////////////////////////////////////////
  class lifetime_utilities
  {
    lifetime_utilities() = delete;
    ~lifetime_utilities() = delete;

    //--------------------------------------------------------------------------
    // Construction
    //--------------------------------------------------------------------------
  public:

    /// \brief Constructs an instance of type \p T with the given \p args
    ///        at the memory location specified in \p p
    ///
    /// \tparam T the type to construct
    /// \param p The memory location to construct into
    /// \param args... The arguments to supply to T's constructor
    /// \return Pointer to the initialized memory
    template <typename T, typename...Args>
    [[nodiscard]]
    static constexpr auto construct_at(not_null<void*> p, Args&&...args)
      noexcept(std::is_nothrow_constructible_v<T, Args...>) -> not_null<T*>;

    /// \brief Constructs an array of the specified size at pointer \p p
    ///
    /// \tparam T the type to construct
    /// \param p the pointer to construct the array at
    /// \param n the size of the array
    /// \return the pointer to the constructed array
    template <typename T>
    [[nodiscard]]
    static constexpr auto construct_array_at(not_null<void*> p, uquantity<T> n)
      noexcept(std::is_nothrow_constructible_v<T>) -> not_null<T*>;

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
    static constexpr auto construct_array_at(not_null<void*> p, uquantity<T> n, const U& copy)
      noexcept(std::is_nothrow_constructible_v<T, const U&>) -> not_null<T*>;

    /// \brief Constructs an instance of type \p T from a given \p tuple
    ///
    /// \tparam T the type to construct
    /// \param p the pointer to construct the type at
    /// \param tuple the tuple to unpack as arguments to \p T
    /// \return pointer to the constructed element
    template <typename T, typename Tuple>
    [[nodiscard]]
    static constexpr auto construct_from_tuple_at(not_null<void*> p, Tuple&& tuple)
      -> not_null<T*>;

    //--------------------------------------------------------------------------
    // Destruction
    //--------------------------------------------------------------------------

    /// \brief Destroys the type at the given pointer \p p
    ///
    /// \tparam T the type to destroy
    /// \param p the pointer to the instance to destroy
    template <typename T>
    static constexpr auto destroy_at(not_null<T*> p)
      noexcept(std::is_nothrow_destructible_v<T>) -> void;

    /// \brief Destroys the array of instances at the specified pointer \p p
    ///
    /// \tparam T the type to destroy
    /// \param p the pointer to the array to destroy
    /// \param n the size of the array
    template <typename T>
    static constexpr auto destroy_array_at(not_null<T*> p, uquantity<T> n)
      noexcept(std::is_nothrow_destructible_v<T>) -> void;

    /// \brief Destroys all objects denoted by the range of iterators
    ///
    /// This calls the destructor on each instance in the valid range.
    ///
    /// \param first iterator to the start of the range
    /// \param last iterator to the end of the range
    template <typename InputIt, typename Sentinel>
    static constexpr auto destroy_range(InputIt first, Sentinel last)
      noexcept(std::is_nothrow_destructible_v<std::iter_value_t<InputIt>>) -> void
      requires(std::input_iterator<InputIt> && std::sentinel_for<Sentinel,InputIt>);

#if __cpp_lib_ranges >= 202110

    /// \brief Destroys all objects denoted by a given range
    ///
    /// This calls the destructor on each instance in the valid range.
    ///
    /// \param range the range
    template <typename Range>
    static constexpr auto destroy_range(Range&& range)
      noexcept(std::is_nothrow_destructible_v<std::ranges::range_value_t<std::decay_t<Range>>>) -> void
      requires(std::ranges::range<Range>);

#endif // __cpp_lib_ranges >= 202110

    //--------------------------------------------------------------------------
    // Private Construction
    //--------------------------------------------------------------------------
  private:

    template <typename T, typename Tuple, std::size_t...Idx>
    static constexpr auto construct_from_tuple_at_impl(
      not_null<void*> p,
      Tuple&& tuple, std::index_sequence<Idx...>
    ) -> not_null<T*>;

    template <typename T, typename...Args>
    static constexpr auto construct_array_at_impl(
      not_null<void*> p,
      uquantity<T> n,
      Args&&...args
    ) noexcept(std::is_nothrow_constructible_v<T,Args...>) -> not_null<T*>;
  };

} // namespace msl

//==============================================================================
// definitions : static class : lifetime_utilities
//==============================================================================

//------------------------------------------------------------------------------
// Construction
//------------------------------------------------------------------------------

template <typename T, typename... Args>
inline constexpr
auto msl::lifetime_utilities::construct_at(not_null<void*> p, Args&&...args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>) -> not_null<T*>
{
  MSL_ASSERT(pointer_utilities::is_aligned(p, alignment::of<T>()));

  return assume_not_null(
    std::construct_at<T>(static_cast<T*>(p.get()), std::forward<Args>(args)...)
  );
}

template <typename T>
inline constexpr
auto msl::lifetime_utilities::construct_array_at(not_null<void*> p, uquantity<T> n)
  noexcept(std::is_nothrow_constructible_v<T>) -> not_null<T*>
{
  return construct_array_at_impl<T>(p, n);
}

template <typename T, typename U>
inline constexpr
auto msl::lifetime_utilities::construct_array_at(not_null<void*> p,
                                                 uquantity<T> n,
                                                 const U& copy)
  noexcept(std::is_nothrow_constructible_v<T, const U&>) -> not_null<T*>
{
  // This should ideally be extremely rare. The pointer is still non-null, but
  // invalid since no object can exist in a 0-element array.
  if (n == uquantity<T>::zero()) MSL_UNLIKELY {
    return static_pointer_cast<T>(p);
  }
  return construct_array_at_impl<T>(p, n, copy);
}


template <typename T, typename Tuple>
inline constexpr
auto msl::lifetime_utilities::construct_from_tuple_at(not_null<void*> p,
                                                      Tuple&& tuple)
  -> not_null<T*>
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
inline constexpr
auto msl::lifetime_utilities::destroy_at(not_null<T*> p)
  noexcept(std::is_nothrow_destructible_v<T>) -> void
{
  // The 'is_trivially_destructible' check here is probably unnecessary, since
  // manually invoking a destructor of a trivial type should be a no-op anyway.
  if constexpr (!std::is_trivially_destructible_v<T>) {
    p->~T();
  }
}

template <typename T>
inline constexpr
auto msl::lifetime_utilities::destroy_array_at(not_null<T*> p, uquantity<T> n)
  noexcept(std::is_nothrow_destructible_v<T>) -> void
{
  if constexpr (!std::is_trivially_destructible_v<T>) {
    auto const first = p;
    auto const last  = first + n.count();

    destroy_range(first, last);
  }
}

//------------------------------------------------------------------------------

template <typename InputIt, typename Sentinel>
inline constexpr
auto msl::lifetime_utilities::destroy_range(InputIt first, Sentinel last)
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

#if __cpp_lib_ranges >= 202110

template <typename Range>
inline constexpr
auto msl::lifetime_utilities::destroy_range(Range&& range)
  noexcept(std::is_nothrow_destructible_v<std::ranges::range_value_t<std::decay_t<Range>>>) -> void
  requires(std::ranges::range<Range>)
{
  using value_type = std::ranges::range_value_t<std::decay_t<Range>>>;

  if constexpr (!std::is_trivially_destructible_v<value_type>) {
    destroy_range(
      std::ranges::begin(std::forward<Range>(range)),
      std::ranges::end(std::forward<Range>(range))
    );
  }
}

#endif // __cpp_lib_ranges >= 202110

//------------------------------------------------------------------------------
// Private Construction
//------------------------------------------------------------------------------

template <typename T, typename Tuple, size_t... Idx>
inline constexpr
auto msl::lifetime_utilities::construct_from_tuple_at_impl(
  not_null<void*> p,
  Tuple&& tuple,
  std::index_sequence<Idx...>
) -> not_null<T*>
{
  return construct_at<T>(p, std::get<Idx>(std::forward<Tuple>(tuple))...);
}


template <typename T, typename... Args>
inline constexpr
auto msl::lifetime_utilities::construct_array_at_impl(
  not_null<void*> p,
  uquantity<T> n,
  Args&&...args
) noexcept(std::is_nothrow_constructible_v<T,Args...>) -> not_null<T*>
{
  MSL_ASSERT(n != 0u);

  // 'p' interpreted as a 'T*' cannot legally be incremented with 'it++', so
  // use the 'advance' utility instead.
  constexpr auto increment = [](auto it){
    return traversal_utilities::next(it);
  };

  auto const first = static_pointer_cast<T>(p);
  auto const last  = traversal_utilities::advance(first, n);

  // capture 'result' so we can return a valid constructed object without
  // requiring std::launder
  auto const result = construct_at<T>(first, std::forward<Args>(args)...);

  auto current = increment(result);
  if constexpr (std::is_nothrow_constructible_v<T, Args...>) {
    for (; current != last; current = increment(current)) MSL_LIKELY {
      [[maybe_unused]]
      const auto r = construct_at<T>(
        current,
        std::forward<Args>(args)...
      );
    }
  } else {
    // Attempt to construct n elements
    try {
      for (; current != last; current = increment(current)) MSL_LIKELY {
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

#endif /* MSL_POINTERS_LIFETIME_UTILITIES_HPP */
