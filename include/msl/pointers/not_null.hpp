////////////////////////////////////////////////////////////////////////////////
/// \file not_null.hpp
///
/// \brief TODO(Bitwize): Add description
////////////////////////////////////////////////////////////////////////////////

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

#ifndef MSL_POINTERS_NOT_NULL_HPP
#define MSL_POINTERS_NOT_NULL_HPP

#if defined(_MSC_VER)
# pragma once
#endif // defined(_MSC_VER)

#include "msl/utilities/intrinsics.hpp" // MSL_FORCE_INLINE
#include "msl/utilities/source_location.hpp"
#include "msl/pointers/nullable_pointer.hpp"
#include "msl/pointers/traversable_pointer.hpp"

#include <cstddef>     // std::nullptr_t
#include <utility>     // std::forward, std::move
#include <type_traits> // std::decay_t
#include <memory>      // std::pointer_traits
#include <stdexcept>   // std::logic_error

namespace msl {

  template <typename T>
  class not_null;

  //===========================================================================
  // trait : is_not_null
  //===========================================================================

  template <typename T>
  struct is_not_null : std::false_type{};
  template <typename T>
  struct is_not_null<not_null<T>> : std::true_type{};

  //===========================================================================
  // class : null_pointer_error
  //===========================================================================

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A static class for managing the contract violation logic
  //////////////////////////////////////////////////////////////////////////////
  struct not_null_contract final
  {
    not_null_contract() = delete;
    ~not_null_contract() = delete;

    //--------------------------------------------------------------------------
    // Public Member Types
    //--------------------------------------------------------------------------

    class violation;

    /// \brief The handler function that will be executed on a contract
    ///        violation.
    ///
    /// \note Contract violation handlers must be non-returning. They must
    ///       either throw an exception or terminate the program.
    using violation_handler = auto(*)(source_location) -> void;

    //--------------------------------------------------------------------------
    // Contract
    //--------------------------------------------------------------------------

    /// \brief Invokes the underlying violation handler assigned to this
    ///        contract
    ///
    /// This function will not return
    ///
    /// \param where the location the violation is taking place
    [[noreturn]]
    static auto violate(source_location where = source_location::current()) -> void;

    /// \brief Gets the currently assigned violation handler
    ///
    /// \return the currently assigned violation handler
    static auto get_violation_handler() noexcept -> violation_handler;

    /// \brief Sets the currently assigned violation handler
    ///
    /// \note If \p handler is `nullptr`, the currently assigned handler is
    ///       returned instead
    ///
    /// \param handler the handler to set
    /// \return the currently assigned violation handler
    static auto set_violation_handler(violation_handler handler) noexcept -> violation_handler;
  };


  /////////////////////////////////////////////////////////////////////////////
  /// \brief An exception thrown on null contract violations as part of
  ///        check_not_null
  /////////////////////////////////////////////////////////////////////////////
  class not_null_contract::violation : public std::logic_error
  {
    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    explicit violation(source_location where);
    violation(const violation& other) = default;
    violation(violation&& other) = default;

    //-------------------------------------------------------------------------

    auto operator=(const violation& other) -> violation& = default;
    auto operator=(violation&& other) -> violation& = default;

    //-------------------------------------------------------------------------
    // Accessors
    //-------------------------------------------------------------------------
  public:

    /// \brief Queries where the failure occurred
    ///
    /// \return the source_location object indicating where the failure occurred
    auto where() const noexcept -> const source_location&;

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    source_location m_where;
  };

  //===========================================================================
  // detail utilities : not_null
  //===========================================================================

  namespace detail {

    ///////////////////////////////////////////////////////////////////////////
    /// \brief A private type that exists to construct not_null's using the
    ///        private constructor (through friendship)
    ///////////////////////////////////////////////////////////////////////////
    struct not_null_factory
    {
      template <typename T>
      static constexpr auto make(T&& p) -> not_null<typename std::decay<T>::type>;
    };

    template <typename T, typename U>
    struct not_null_is_explicit_convertible : std::integral_constant<bool,(
      std::is_constructible<T,U>::value &&
      !std::is_convertible<U,T>::value
    )>{};

  } // namespace detail

  //===========================================================================
  // class : not_null
  //===========================================================================

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A wrapper type around a pointer to disallow null assignments
  ///
  /// This type can only be used with pointers that may be understood by
  /// pointer_traits. This requires a type to either define 'element_type'
  /// or specialize pointer_traits for their respective needs.
  ///
  /// This type is a type-wrapper, so that APIs can semantically indicate their
  /// nullability requirement in a concise and coherent way.
  ///
  /// ### Examples
  ///
  /// Basic Usage:
  ///
  /// ```cpp
  /// auto post(not_null<std::unique_ptr<Task>> task) -> void
  /// {
  ///   // pass to internal API that uses 'unique_ptr' directly
  ///   post_internal(std::move(task).as_nullable());
  /// }
  ///
  /// ...
  ///
  /// post(assume_not_null(std::move(p)));
  /// ````
  ///
  /// \tparam T the underlying pointer type
  /////////////////////////////////////////////////////////////////////////////
  template <typename T>
  class not_null
  {
    static_assert(
      nullable_pointer<T>,
      "T must satisfy the nullable_pointer concept, otherwise not_null<T> is "
      "ill-formed."
    );
    static_assert(
      !std::is_same<T,std::nullptr_t>::value,
      "not_null<std::nullptr_t> is ill-formed. I mean, really, come on."
    );
    static_assert(
      !std::is_reference<T>::value,
      "not_null<T&> is ill-formed. "
      "not_null may only work with non-CV qualified value types."
    );
    static_assert(
      !std::is_void<T>::value,
      "not_null<void> is ill-formed. "
      "not_null may only work with non-CV qualified value types."
    );
    static_assert(
      !is_not_null<T>::value,
      "not_null<not_null<T>> is ill-formed."
    );
    static_assert(
      !std::is_const<T>::value && !std::is_volatile<T>::value,
      "not_null<[const] [volatile] T> is ill-formed. "
      "not_null may only work with non-CV qualified value types."
    );

    //-------------------------------------------------------------------------
    // Public Member Types
    //-------------------------------------------------------------------------
  public:

    using traits_type     = std::pointer_traits<T>;

    using element_type    = typename traits_type::element_type;
    using pointer         = element_type*;
    using difference_type = typename traits_type::difference_type;

    template <typename U>
    using rebind = not_null<typename traits_type::template rebind<U>>;

    //-------------------------------------------------------------------------
    // Constructors / Assignment
    //-------------------------------------------------------------------------
  public:

    // not_null is not default-constructible, since this would result in a null
    // value
    not_null() = delete;

    /// \brief Constructs a not_null by converting a not_null from a different
    ///        underlying source
    ///
    /// This constructor allows for conversions between different underlying
    /// pointer types
    ///
    /// \note This constructor only participates in overload resolution if
    ///       `std::is_constructible<T,const U&>::value` is `true`
    ///
    /// \note This constructor is explicit if and only if
    ///       `std::is_convertible<const U&, T>::value` is `false`
    ///
    /// ### Examples
    ///
    /// Implicit use:
    ///
    /// ```cpp
    /// auto p = assume_not_null(
    ///     std::make_shared<Derived>(42)
    /// );
    /// not_null<std::shared_ptr<Base>> q = p;
    ///
    /// assert(p == q);
    /// ```
    ///
    /// Explicit Use:
    ///
    /// ```cpp
    /// // Uses 'p' to construct the underlying unique_ptr
    /// auto p = assume_not_null(
    ///     new int{42}
    /// );
    /// auto q = not_null<std::unique_ptr<int>>{p};
    ///
    /// assert(p == q);
    /// ```
    ///
    /// \param other the other not_null to convert
    template <typename U>
    explicit(detail::not_null_is_explicit_convertible<T,const U&>::value)
    constexpr not_null(const not_null<U>& other)
      noexcept(std::is_nothrow_constructible<T,const U&>::value)
      requires(std::constructible_from<T,const U&>);

    /// \brief Constructs a not_null by converting a not_null from a different
    ///        underlying source
    ///
    /// This constructor allows for conversions between different underlying
    /// pointer types
    ///
    /// \note This constructor only participates in overload resolution if
    ///       `std::is_constructible<T,U&&>::value` is `true`
    ///
    /// \note This constructor is explicit if and only if
    ///       `std::is_convertible<U&&, T>::value` is `false`
    ///
    /// ### Examples
    ///
    /// Implicit Use:
    ///
    /// ```cpp
    /// auto p = assume_not_null(
    ///     std::make_shared<Derived>(42)
    /// );
    /// not_null<std::shared_ptr<Base>> q = std::move(p);
    /// ```
    ///
    /// Explicit Use:
    ///
    /// ```cpp
    /// auto p = assume_not_null(
    ///     std::make_unique<int>(42)
    /// );
    /// auto q = not_null<std::shared_ptr<int>>{
    ///     std::move(p)
    /// };
    /// ```
    ///
    /// \param other the other not_null to convert
    template <typename U>
    explicit(detail::not_null_is_explicit_convertible<T,U&&>::value)
    constexpr not_null(not_null<U>&& other)
      noexcept(std::is_nothrow_constructible<T,U&&>::value)
      requires(std::constructible_from<T,U&&>);

    /// \brief Constructs this not_null by copying the contents of \p other
    ///
    /// \note This constructor does not paticipate in overload resolution
    ///       unless `std::is_copy_constructible<T>::value` is `true`
    ///
    /// \note This constructor is trivial if the underlying type `T`'s
    ///       copy constructor is trivial.
    ///
    /// \param other the other not_null to copy from
    not_null(const not_null& other) = default;

    /// \brief Constructs this not_null by moving the contents of \p other
    ///
    /// \note This constructor does not paticipate in overload resolution
    ///       unless `std::is_move_constructible<T>::value` is `true`
    ///
    /// \note This constructor is trivial if the underlying type `T`'s
    ///       copy constructor is trivial.
    ///
    /// \param other the other not_null to move from
    not_null(not_null&& other) = default;

    //-------------------------------------------------------------------------

    template <typename U>
    constexpr auto operator=(const not_null<U>& other)
      noexcept(std::is_nothrow_assignable<T&,const U&>::value) -> not_null&
      requires(std::is_assignable_v<T&,const U&>);

    template <typename U>
    constexpr auto operator=(not_null<U>&& other)
      noexcept(std::is_nothrow_assignable<T&,U&&>::value) -> not_null&
      requires(std::is_assignable_v<T&,U&&>);

    auto operator=(const not_null& other) -> not_null& = default;

    auto operator=(not_null&& other) -> not_null& = default;

    //-------------------------------------------------------------------------
    // Observers
    //-------------------------------------------------------------------------
  public:

    /// \brief Gets the underlying pointer
    ///
    /// \return the underlying pointer
    constexpr auto get() const noexcept -> pointer;

    /// \brief Contextually convertible to bool
    ///
    /// This is always true
    constexpr explicit operator bool() const noexcept;

    //-------------------------------------------------------------------------

    /// \{
    /// \brief Extracts the underlying nullable pointer from not_null
    ///
    /// \note The r-value overload allows for stealing the underlying nullable
    ///       pointer. In doing so, the invariant of this class may
    ///       intentionally be violated (subject to the underlying pointer).
    ///       Care needs to be taken to avoid potentially reusing this not_null
    ///       after moving. Realistically, using a pointer after moving is
    ///       already a bad idea anyway since it may result in use-after-move.
    ///
    /// \return the underlying nullable pointer
    constexpr auto as_nullable() const & noexcept -> const T&;
    constexpr auto as_nullable() && noexcept -> T&&;
    /// \}

    //-------------------------------------------------------------------------

    /// \brief Dereferences the underlying pointer
    ///
    /// \return the underlying pointer
    constexpr auto operator->() const noexcept -> pointer;

    /// \brief Dereferences the underlying pointer
    ///
    /// \return reference to the underlying pointer
    constexpr auto operator*() const noexcept -> std::add_lvalue_reference_t<element_type>;

    //--------------------------------------------------------------------------
    // Traversal
    //--------------------------------------------------------------------------
  public:

    constexpr auto operator++()
      noexcept(noexcept(++std::declval<T&>())) -> not_null<T>&
      requires(traversable_pointer<T>);

    constexpr auto operator++(int)
      noexcept(noexcept(std::declval<T&>()++))  -> not_null<T>
      requires(traversable_pointer<T>);

    constexpr auto operator--()
      noexcept(noexcept(--std::declval<T&>())) -> not_null<T>&
      requires(traversable_pointer<T>);

    constexpr auto operator--(int)
      noexcept(noexcept(std::declval<T&>()--)) -> not_null<T>
      requires(traversable_pointer<T>);

    constexpr auto operator+=(std::ptrdiff_t n)
      noexcept(noexcept(std::declval<T&>() += std::ptrdiff_t{})) -> not_null<T>&
      requires(traversable_pointer<T>);

    constexpr auto operator-=(std::ptrdiff_t n)
      noexcept(noexcept(std::declval<T&>() -= std::ptrdiff_t{})) -> not_null<T>&
      requires(traversable_pointer<T>);

    //-------------------------------------------------------------------------
    // Private Members
    //-------------------------------------------------------------------------
  private:

    T m_pointer;

    //-------------------------------------------------------------------------
    // Private Constructors
    //-------------------------------------------------------------------------
  private:

    struct ctor_tag{};

    template <typename P>
    constexpr not_null(ctor_tag, P&& ptr)
      noexcept(std::is_nothrow_constructible<std::decay_t<P>,P>::value);

    friend detail::not_null_factory;
  };

  //===========================================================================
  // non-member functions : class : not_null
  //===========================================================================

  //---------------------------------------------------------------------------
  // Utilities
  //---------------------------------------------------------------------------

  /// \brief Creates a `not_null` object by checking that `ptr` is not null
  ///        first
  ///
  /// On contract violation a `not_null_contract_violation` is exception thrown.
  /// This exceptions is not intended to be caught and handled in most
  /// workflows; rather this is meant as a simple way to tear-down an
  /// application placed into an  state through the use of stack-unwinding.
  ///
  /// `check_not_null` contains the overhead of checking for null first, but
  /// is opt-in. If a type is known to never be null, consider `assume_not_null`
  /// below.
  ///
  /// ### Examples
  ///
  /// Basic use:
  ///
  /// ```cpp
  /// // Adapting legacy API
  /// auto consume_impl(not_null<std::unique_ptr<Widget>>) -> void;
  ///
  /// auto consume(std::unique_ptr<Widget> p) -> void
  /// {
  ///     // Expect this invariant in our code; crash if not.
  ///     consume_impl(check_not_null(std::move(p));
  /// }
  ///
  /// ```
  ///
  /// \throw not_null_contract_violation if `ptr == nullptr`
  /// \param ptr the pointer to check for nullability first
  /// \param where [optional] the location where the check is occurring
  ///        (defaults to the call-site)
  /// \return a `not_null` object containing `ptr`
  template <typename T>
  [[nodiscard]]
  constexpr auto check_not_null(T&& ptr, source_location where = source_location::current())
    -> not_null<std::decay_t<T>>;

  /// \brief Creates a `not_null` object by *assuming* that `ptr` is not null
  ///
  /// Since this function does no proper checking, it is up to the user to
  /// guarantee that `ptr` does not violate the invariant. If the invariant is
  /// *not* upheld, the user may experience **undefined behavior** due to
  /// potential null pointer dereferences, and due to other code assuming nulls
  /// can never happen.
  ///
  /// This function should only be used in cases where it can be guaranteed that
  /// `ptr` can never be null, such as for an object's invariant, or when
  /// using `not_null` with already known non-null objects.
  ///
  /// ### Examples
  ///
  /// Basic use:
  ///
  /// ```cpp
  /// auto x = 5;
  /// auto nn = assume_not_null(&x); // we know 'x' cannot be null
  ///
  /// assert(nn == &x);
  /// ```
  ///
  /// or
  ///
  /// ```cpp
  /// // 'make_shared' never returns null
  /// auto p = assume_not_null(
  ///   std::make_shared<int>(42)
  /// );
  ///
  /// consume_not_null(std::move(p));
  /// ```
  ///
  /// \param ptr the pointer that cannot be null
  /// \return a not_null containing the pointer
  template <typename T>
  [[nodiscard]]
  constexpr auto assume_not_null(T&& ptr)
    noexcept(std::is_nothrow_constructible<typename std::decay<T>::type,T>::value)
    -> not_null<std::decay_t<T>>;

  //---------------------------------------------------------------------------
  // Arithmetic
  //---------------------------------------------------------------------------

  template <typename T>
  constexpr auto operator+(const not_null<T>& p, std::ptrdiff_t n)
    noexcept(noexcept(std::declval<const T&>() + std::ptrdiff_t{})) -> not_null<T>
    requires(traversable_pointer<T>);

  template <typename T>
  constexpr auto operator+(std::ptrdiff_t n, const not_null<T>& p)
    noexcept(noexcept(std::ptrdiff_t{} + std::declval<const T&>())) -> not_null<T>
    requires(traversable_pointer<T>);

  template <typename T>
  constexpr auto operator-(const not_null<T>& p, std::ptrdiff_t n)
    noexcept(noexcept(std::declval<const T&>() - std::ptrdiff_t{})) -> not_null<T>
    requires(traversable_pointer<T>);

  template <typename T>
  constexpr auto operator-(const not_null<T>& lhs, const not_null<T>& rhs)
    noexcept(noexcept(std::declval<const T&>() - std::declval<const T&>()))
    -> decltype(lhs.get() - rhs.get())
    requires(traversable_pointer<T>);

  //---------------------------------------------------------------------------
  // Comparisons
  //---------------------------------------------------------------------------

  template <typename T>
  constexpr auto operator==(const not_null<T>& lhs, std::nullptr_t) noexcept -> bool;
  template <typename T>
  constexpr auto operator==(std::nullptr_t, const not_null<T>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() == std::declval<const U&>())>
  constexpr auto operator==(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() == std::declval<const U&>())>
  constexpr auto operator==(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() == std::declval<const U&>())>
  constexpr auto operator==(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T>
  constexpr auto operator!=(const not_null<T>& lhs, std::nullptr_t) noexcept -> bool;
  template <typename T>
  constexpr auto operator!=(std::nullptr_t, const not_null<T>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() != std::declval<const U&>())>
  constexpr auto operator!=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() != std::declval<const U&>())>
  constexpr auto operator!=(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() != std::declval<const U&>())>
  constexpr auto operator!=(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() < std::declval<const U&>())>
  constexpr auto operator<(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() < std::declval<const U&>())>
  constexpr auto operator<(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() < std::declval<const U&>())>
  constexpr auto operator<(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() > std::declval<const U&>())>
  constexpr auto operator>(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() > std::declval<const U&>())>
  constexpr auto operator>(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() > std::declval<const U&>())>
  constexpr auto operator>(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() <= std::declval<const U&>())>
  constexpr auto operator<=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() <= std::declval<const U&>())>
  constexpr auto operator<=(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() <= std::declval<const U&>())>
  constexpr auto operator<=(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() >= std::declval<const U&>())>
  constexpr auto operator>=(const not_null<T>& lhs, const not_null<U>& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() >= std::declval<const U&>())>
  constexpr auto operator>=(const not_null<T>& lhs, const U& rhs) noexcept -> bool;
  template <typename T, typename U,
            typename = decltype(std::declval<const T&>() >= std::declval<const U&>())>
  constexpr auto operator>=(const T& lhs, const not_null<U>& rhs) noexcept -> bool;

} // namespace msl

//=============================================================================
// detail utilities : not_null
//=============================================================================

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::detail::not_null_factory::make(T&& p)
  -> not_null<typename std::decay<T>::type>
{
  using value_type = typename std::decay<T>::type;

  return not_null<value_type>{
    typename not_null<value_type>::ctor_tag{},
    std::forward<T>(p)
  };
}

//=============================================================================
// class : not_null_contract::violation
//=============================================================================

MSL_FORCE_INLINE
auto msl::not_null_contract::violation::where()
  const noexcept -> const source_location&
{
  return m_where;
}

//=============================================================================
// class : not_null
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

template <typename T>
template <typename U>
MSL_FORCE_INLINE constexpr
msl::not_null<T>::not_null(const not_null<U>& other)
  noexcept(std::is_nothrow_constructible<T,const U&>::value)
  requires(std::constructible_from<T,const U&>)
  : m_pointer(other.as_nullable())
{

}

template <typename T>
template <typename U>
MSL_FORCE_INLINE constexpr
msl::not_null<T>::not_null(not_null<U>&& other)
  noexcept(std::is_nothrow_constructible<T,U&&>::value)
  requires(std::constructible_from<T,U&&>)
  : m_pointer(std::move(other).as_nullable())
{

}

//-----------------------------------------------------------------------------

template <typename T>
template <typename U>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator=(const not_null<U>& other)
  noexcept(std::is_nothrow_assignable<T&,const U&>::value) -> not_null&
  requires(std::is_assignable_v<T&,const U&>)
{
  m_pointer = other.as_nullable();
  return (*this);
}

template <typename T>
template <typename U>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator=(not_null<U>&& other)
  noexcept(std::is_nothrow_assignable<T&,U&&>::value) -> not_null&
  requires(std::is_assignable_v<T&,U&&>)
{
  m_pointer = std::move(other).as_nullable();
  return (*this);
}

//-----------------------------------------------------------------------------
// Observers
//-----------------------------------------------------------------------------

template<typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::get()
  const noexcept -> pointer
{
  return intrinsics::assume_not_null(std::to_address(m_pointer));
}

template <typename T>
MSL_FORCE_INLINE constexpr
msl::not_null<T>::operator bool()
  const noexcept
{
  return true;
}

//-----------------------------------------------------------------------------

template<typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::as_nullable()
  const & noexcept -> const T&
{
  return m_pointer;
}

template<typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::as_nullable()
  && noexcept -> T&&
{
  return std::move(m_pointer);
}

//-----------------------------------------------------------------------------

template<typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator->()
  const noexcept -> pointer
{
  return get();
}

template<typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator*()
  const noexcept -> std::add_lvalue_reference_t<element_type>
{
  return *get();
}

//------------------------------------------------------------------------------
// Traversal
//------------------------------------------------------------------------------

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator++()
  noexcept(noexcept(++std::declval<T&>())) -> not_null<T>&
  requires(msl::traversable_pointer<T>)
{
  ++m_pointer;
  return (*this);
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator++(int)
  noexcept(noexcept(std::declval<T&>()++)) -> not_null<T>
  requires(msl::traversable_pointer<T>)
{
  const auto copy = (*this);
  ++(*this);
  return copy;
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator--()
  noexcept(noexcept(--std::declval<T&>())) -> not_null<T>&
  requires(msl::traversable_pointer<T>)
{
  --m_pointer;
  return (*this);
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator--(int)
  noexcept(noexcept(std::declval<T&>()--)) -> not_null<T>
  requires(msl::traversable_pointer<T>)
{
  const auto copy = (*this);
  --(*this);
  return copy;
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator+=(std::ptrdiff_t n)
  noexcept(noexcept(std::declval<T&>() += std::ptrdiff_t{})) -> not_null<T>&
  requires(msl::traversable_pointer<T>)
{
  m_pointer += n;
  return (*this);
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::not_null<T>::operator-=(std::ptrdiff_t n)
  noexcept(noexcept(std::declval<T&>() -= std::ptrdiff_t{})) -> not_null<T>&
  requires(msl::traversable_pointer<T>)
{
  m_pointer -= n;
  return (*this);
}

//-----------------------------------------------------------------------------
// Private Constructor
//-----------------------------------------------------------------------------

template <typename T>
template <typename P>
MSL_FORCE_INLINE constexpr
msl::not_null<T>::not_null(ctor_tag, P&& ptr)
  noexcept(std::is_nothrow_constructible<std::decay_t<P>, P>::value)
  : m_pointer(std::forward<P>(ptr))
{

}

//=============================================================================
// non-member functions : class : not_null
//=============================================================================

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::check_not_null(T&& ptr, source_location where)
  -> not_null<std::decay_t<T>>
{
  if (ptr == nullptr) MSL_UNLIKELY {
    not_null_contract::violate(where);
  }
  MSL_LIKELY return assume_not_null(std::forward<T>(ptr));
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::assume_not_null(T&& ptr)
  noexcept(std::is_nothrow_constructible<typename std::decay<T>::type,T>::value)
  -> not_null<typename std::decay<T>::type>
{
  return detail::not_null_factory::make(std::forward<T>(ptr));
}

//-----------------------------------------------------------------------------
// Arithmetic
//-----------------------------------------------------------------------------

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::operator+(const not_null<T>& p, std::ptrdiff_t n)
  noexcept(noexcept(std::declval<const T&>() + std::ptrdiff_t{})) -> not_null<T>
  requires(msl::traversable_pointer<T>)
{
  return assume_not_null(p.get() + n);
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::operator+(std::ptrdiff_t n, const not_null<T>& p)
  noexcept(noexcept(std::ptrdiff_t{} + std::declval<const T&>())) -> not_null<T>
  requires(msl::traversable_pointer<T>)
{
  return assume_not_null(n + p.get());
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::operator-(const not_null<T>& p, std::ptrdiff_t n)
  noexcept(noexcept(std::declval<const T&>() - std::ptrdiff_t{})) -> not_null<T>
  requires(msl::traversable_pointer<T>)
{
  return assume_not_null(p.get() - n);
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::operator-(const not_null<T>& lhs, const not_null<T>& rhs)
  noexcept(noexcept(std::declval<const T&>() - std::declval<const T&>())) -> decltype(lhs.get() - rhs.get())
  requires(msl::traversable_pointer<T>)
{
  return lhs.get() - rhs.get();
}

//-----------------------------------------------------------------------------
// Comparisons
//-----------------------------------------------------------------------------

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::operator==(const not_null<T>&, std::nullptr_t)
  noexcept -> bool
{
  return false;
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::operator==(std::nullptr_t, const not_null<T>&)
  noexcept -> bool
{
  return false;
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator==(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() == rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator==(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() == rhs;
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator==(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs == rhs.as_nullable();
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::operator!=(const not_null<T>&, std::nullptr_t)
  noexcept -> bool
{
  return true;
}

template <typename T>
MSL_FORCE_INLINE constexpr
auto msl::operator!=(std::nullptr_t, const not_null<T>&)
  noexcept -> bool
{
  return true;
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator!=(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() != rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator!=(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() != rhs;
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator!=(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs != rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator<(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() < rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator<(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() < rhs;
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator<(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs < rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator>(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() > rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator>(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() > rhs;
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator>(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs > rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator<=(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() <= rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator<=(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() <= rhs;
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator<=(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs <= rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator>=(const not_null<T>& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() >= rhs.as_nullable();
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator>=(const not_null<T>& lhs, const U& rhs)
  noexcept -> bool
{
  return lhs.as_nullable() >= rhs;
}

template <typename T, typename U, typename>
MSL_FORCE_INLINE constexpr
auto msl::operator>=(const T& lhs, const not_null<U>& rhs)
  noexcept -> bool
{
  return lhs >= rhs.as_nullable();
}

#endif /* MSL_POINTERS_NOT_NULL_HPP */
