////////////////////////////////////////////////////////////////////////////////
/// \file tagged_ptr.hpp
///
/// \brief This header provides the definition of a tagged pointer that
///        leverages the unclaimed/unused alignment bits of a pointer.
////////////////////////////////////////////////////////////////////////////////

/*
  The MIT License (MIT)

  Copyright (c) 2021 Matthew Rodusek All rights reserved.

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

#ifndef MSL_POINTERS_TAGGED_PTR_HPP
#define MSL_POINTERS_TAGGED_PTR_HPP

#include "msl/utilities/intrinsics.hpp" // MSL_FORCE_INLINE
#include "msl/utilities/assert.hpp"     // MSL_ASSERT
#include "msl/config.hpp"               // MSL_DISABLE_STRICT_MODE

#include <cstdint>  // std::uintptr_t
#include <cstddef>  // std::size_t
#include <concepts> // std::convertible_to
#include <compare>  // std::three_way_compare
#include <bit>      // std::countr_zero

namespace msl {

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A pointer that leverages the unused bits that are a result of its
  ///        natural alignment so that additional data may be associated with
  ///        it.
  ///
  /// All objects have a natural alignment, and for most objects its greater
  /// than 1. This naturally means that the lowest set bits of its address will
  /// be set to 0, and is otherwise unused and effectively wasted space.
  ///
  /// This pointer makes use of those bits by using them for storing additional
  /// data as a "tag". Tags can be useful for storing information such as
  /// chunk-lifetime, reference-counts, etc.
  ///
  /// \note
  /// `tagged_ptr` is not a "smart" pointer, and thus does not actually denote
  /// any form of ownership.
  ///
  /// \warning
  /// Make sure that `Bits` are actually free and nonzero when using
  /// `tagged_ptr`, otherwise an application will experience undefined behavior
  /// due to incorrect pointers being returned. In many cases, `Bits` should be
  /// at most the bits in the natural alignment of `T` -- but this is not always
  /// true if we are dealing with allocated pointers which are guaranteed to be
  /// aligned to at least `alignof(std::max_align_t)`.
  ///
  /// \note
  /// Due to the additional data stored in the lower-bits, dereferencing has the
  /// added cost of masking the higher bits for access. For performance-critical
  /// areas that use frequent dereferencing, the returned value from `get()`
  /// should be cached, rather than using the convenience indirection accessors
  /// such as `operator->`, `operator*`, or `operator[]`.
  ///
  /// \tparam T the type of pointer. May be an object, or an unbound array for
  ///           pointers to sequences
  /// \tparam Bits the number of (lower) bits to use for the tag. This should
  ///              coincide with the expected alignment of the pointer for `T`
  //////////////////////////////////////////////////////////////////////////////
  template <typename T, std::size_t Bits = std::countr_zero(alignof(T))>
  class tagged_ptr
  {
    static_assert(!std::is_bounded_array_v<T>);
    static_assert(Bits > 0u);
    static_assert(Bits < (sizeof(T*) * 8u));

    //--------------------------------------------------------------------------
    // Public Members
    //--------------------------------------------------------------------------
  public:

    using element_type = std::remove_all_extents_t<T>;
    using pointer      = element_type*;

    //--------------------------------------------------------------------------
    // Constructors / Assignment
    //--------------------------------------------------------------------------
  public:

    /// \{
    /// \brief Constructs a null tagged pointer
    ///
    /// \warning
    /// The C++ standard does not formally state what the numeric representation
    /// of a null pointer is. In almost all known cases this is usually `0`, but
    /// this is not technically guaranteed. For complete portability's sake, it
    /// is important to be aware that null pointers may not actually be taggable
    /// and will thus trip an assertion. It is impossible to determine this at
    /// compile-time, and so this is left as a warning -- though in almost all
    /// known systems this should never be an issue.
    tagged_ptr() noexcept;
    tagged_ptr(std::nullptr_t) noexcept;
    /// \}

    /// \brief Constructs this tagged_ptr from a pointer that is convertible to
    ///        T*
    ///
    /// \param p the pointer to convert
    tagged_ptr(pointer p) noexcept;

    /// \brief Constructs this tagged_ptr from a pointer that is convertible to
    ///        T* and the tag to set
    ///
    /// \param p the pointer to convert
    /// \param tag the tag to set
    tagged_ptr(pointer p, std::uintptr_t tag) noexcept;

    /// \brief Converts the tagged pointer \p other to a tagged_ptr of this type
    ///
    /// This constructor is implicit only if `U` and `T` differ by
    /// CV-qualifications. Otherwise this constructor is `explicit` due to
    /// the alignment possibly being disturbed by up-casting.
    ///
    /// \warning
    /// When up-casting, the pointer address _may_ change in some circumstances,
    /// such as in the case of having multiple-bases. In such a case, it may
    /// reduce the available bits that may be tagged.
    ///
    /// \param other the other pointer to convert
    template <typename U>
    tagged_ptr(const tagged_ptr<U, Bits>& other) noexcept
      requires(std::convertible_to<typename tagged_ptr<U, Bits>::pointer,pointer> && !std::is_array_v<T>);

    /// \brief Copies the contents of \p other
    ///
    /// \param other the other pointer to copy
    tagged_ptr(const tagged_ptr& other) = default;

    //--------------------------------------------------------------------------

    /// \brief Copies the contents of \p other
    ///
    /// \param other the other pointer to copy
    /// \return reference to `(*this)`
    auto operator=(const tagged_ptr& other) -> tagged_ptr& = default;

    /// \brief Assigns the underlying tagged pointer to the pointer \p p
    ///
    /// \note This assignment will not overwrite the tag bits
    ///
    /// \param p the pointer to set
    /// \return reference to `(*this)`
    auto operator=(pointer p) noexcept -> tagged_ptr&;

    /// \brief Assigns the underlying tagged pointer to null
    ///
    /// \note This assignment will not overwrite the tag bits
    ///
    /// \return reference to `(*this)`
    auto operator=(std::nullptr_t) noexcept -> tagged_ptr&;

    //--------------------------------------------------------------------------
    // Observers
    //--------------------------------------------------------------------------
  public:

    /// \brief Gets the underlying pointer
    ///
    /// \return the pointer
    auto get() const noexcept -> pointer;

    /// \brief Gets the tag for this underlying pointer
    ///
    /// \return the tag
    auto tag() const noexcept -> std::uintptr_t;

    /// \brief Boolean conversion operator
    explicit operator bool() const noexcept;

    //--------------------------------------------------------------------------
    // Modifiers
    //--------------------------------------------------------------------------
  public:

    /// \brief sets the tag to \p t
    ///
    /// \note
    /// Any set bits past `Bits` will be truncated and ignored
    auto tag(std::uintptr_t t) noexcept -> void;

    /// \brief Resets the pointer that is tagged, but does not reset the tagging
    ///
    /// \param p the pointer to set
    auto reset(pointer p) noexcept -> void;

    /// \brief Resets the pointer that is tagged to null
    auto reset(std::nullptr_t) noexcept -> void;

    /// \brief Resets the pointer that is tagged, but does not reset the tagging
    ///
    /// \param p the pointer to set
    /// \param tag the tag to set
    auto reset(pointer p, std::uintptr_t tag) noexcept -> void;

    /// \brief Resets the pointer that is tagged to null
    auto reset(std::nullptr_t, std::uintptr_t tag) noexcept -> void;

    //--------------------------------------------------------------------------
    // Indirection
    //--------------------------------------------------------------------------
  public:

    auto operator*() const noexcept -> T&
      requires(!std::is_array_v<T>);
    auto operator->() const noexcept -> T*
      requires(!std::is_array_v<T>);
    auto operator[](std::size_t index) const noexcept -> T&
      requires(std::is_array_v<T>);

    //--------------------------------------------------------------------------
    // Comparison
    //--------------------------------------------------------------------------
  public:

    template <typename U>
    auto operator==(const tagged_ptr<U, Bits>& other) const noexcept -> bool
      requires(std::convertible_to<U*, element_type*>);

    auto operator==(std::nullptr_t) const noexcept -> bool;

    //--------------------------------------------------------------------------

    template <typename U>
    auto operator<=>(const tagged_ptr<U, Bits>& other) const noexcept -> bool
      requires(std::convertible_to<U*, element_type*>);

    auto operator<=>(std::nullptr_t) const noexcept;

    //--------------------------------------------------------------------------
    // Private Members
    //--------------------------------------------------------------------------
  private:

    static inline constexpr auto tag_mask     = std::uintptr_t{1u << Bits} - 1u;
    static inline constexpr auto pointer_mask = ~tag_mask;

    std::uintptr_t m_pointer;
  };

} // namespace msl

//-------------------------------------------------------------------------------
// Constructors / Assignment
//-------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
msl::tagged_ptr<T, Bits>::tagged_ptr()
  noexcept
  : tagged_ptr{nullptr}
{

}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
msl::tagged_ptr<T, Bits>::tagged_ptr(std::nullptr_t)
  noexcept
  : tagged_ptr{static_cast<pointer>(nullptr)}
{

}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
msl::tagged_ptr<T, Bits>::tagged_ptr(pointer p)
  noexcept
  : m_pointer{reinterpret_cast<std::uintptr_t>(p)}
{
  MSL_ASSERT(
    (m_pointer & tag_mask) == 0u,
    "Pointer is not suitably aligned to be tagged!"
  );
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
msl::tagged_ptr<T, Bits>::tagged_ptr(pointer p, std::uintptr_t tag)
  noexcept
  : m_pointer{
      reinterpret_cast<std::uintptr_t>(p) | (tag & tag_mask)
    }
{
  MSL_ASSERT(
    (reinterpret_cast<std::uintptr_t>(p) & tag_mask) == 0u,
    "Pointer is not suitably aligned to be tagged!"
  );
}

template <typename T, std::size_t Bits>
template <typename U>
MSL_FORCE_INLINE
msl::tagged_ptr<T, Bits>::tagged_ptr(const tagged_ptr<U, Bits>& other)
  noexcept requires(std::convertible_to<typename tagged_ptr<U, Bits>::pointer,pointer> && !std::is_array_v<T>)
  : tagged_ptr{other.get(), other.tag()}
{
  // This conversion constructor intentionally doesn't just copy
  // `other.m_pointer`, since up-casting a derived pointer may result in a
  // different base address, and this needs to be checked.
}

//------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator=(pointer p)
  noexcept -> tagged_ptr&
{
  reset(p);
  return (*this);
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator=(std::nullptr_t)
  noexcept -> tagged_ptr&
{
  reset(nullptr);
  return (*this);
}

//------------------------------------------------------------------------------
// Observers
//------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::get()
  const noexcept -> pointer
{
  // Whatever the pointer returned, we can assume it must be aligned at least to
  // either 'alignof(T)' or -- if Bits is greater than this -- it must be aligned
  // at least to '1 << Bits'
  static constexpr auto bit_alignment = (1u << Bits);
  static constexpr auto assumed_alignment = (
    (alignof(T) > bit_alignment)
    ? alignof(T) : bit_alignment
  );

  return intrinsics::assume_aligned<assumed_alignment>(
    reinterpret_cast<T*>(m_pointer & pointer_mask)
  );
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::tag()
  const noexcept -> std::uintptr_t
{
  return (m_pointer & tag_mask);
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
msl::tagged_ptr<T, Bits>::operator bool()
  const noexcept
{
  return get() != nullptr;
}

//------------------------------------------------------------------------------
// Modifiers
//------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::tag(std::uintptr_t t)
  noexcept -> void
{
  m_pointer |= (t & tag_mask);
}

//------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::reset(pointer p)
  noexcept -> void
{
  MSL_ASSERT(
    (reinterpret_cast<std::uintptr_t>(p) & tag_mask) == 0u,
    "Pointer is not suitably aligned to be tagged!"
  );

  m_pointer &= tag_mask;
  m_pointer |= (reinterpret_cast<std::uintptr_t>(p) & pointer_mask);
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::reset(std::nullptr_t)
  noexcept -> void
{
  reset(static_cast<pointer>(nullptr));
}

//------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::reset(pointer p, std::uintptr_t tag)
  noexcept -> void
{
  auto* const q = static_cast<pointer>(p);

  MSL_ASSERT(
    (reinterpret_cast<std::uintptr_t>(q) & tag_mask) == 0u,
    "Pointer is not suitably aligned to be tagged!"
  );

  m_pointer = (
    (reinterpret_cast<std::uintptr_t>(q) & pointer_mask) | (tag & tag_mask)
  );
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::reset(std::nullptr_t, std::uintptr_t tag)
  noexcept -> void
{
  reset(static_cast<pointer>(nullptr), tag);
}

//------------------------------------------------------------------------------
// Element Access
//------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator*()
  const noexcept -> T&
  requires(!std::is_array_v<T>)
{
  return *get();
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator->()
  const noexcept -> T*
  requires(!std::is_array_v<T>)
{
  return get();
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator[](std::size_t index)
  const noexcept -> T&
  requires(std::is_array_v<T>)
{
  return *(get() + index);
}

//------------------------------------------------------------------------------
// Comparison
//------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
template <typename U>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator==(const tagged_ptr<U, Bits>& other)
  const noexcept -> bool
  requires(std::convertible_to<U*, element_type*>)
{
#if MSL_DISABLE_STRICT_MODE
  return m_pointer == other.m_pointer;
#else
  // Technically, although almost all implementations will accept
  // `m_pointer == other.m_pointer` as always being correct, the C++ standard
  // does not guarantee that `uintptr_t`s will compare equal from a given `T*`,
  // even if the `T*`s compare equal. To work around this, comparison must
  // compare the pointers themselves -- which are guaranteed to respect this.
  return (get() == other.get()) && (tag() == other.tag());
#endif
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator==(std::nullptr_t)
  const noexcept -> bool
{
#if MSL_DISABLE_STRICT_MODE
  return m_pointer == 0u;
#else
  return (get() == nullptr) && (tag() == 0u);
#endif
}

//------------------------------------------------------------------------------

template <typename T, std::size_t Bits>
template <typename U>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator<=>(const tagged_ptr<U, Bits>& other)
  const noexcept -> bool
  requires(std::convertible_to<U*, element_type*>)
{
#if MSL_DISABLE_STRICT_MODE
  return m_pointer <=> other.m_pointer;
#else
  const auto* const l = get();
  const auto* const r = other.get();

  if (l == r) {
    return tag() <=> other.tag();
  }
  return l <=> r;
#endif
}

template <typename T, std::size_t Bits>
MSL_FORCE_INLINE
auto msl::tagged_ptr<T, Bits>::operator<=>(std::nullptr_t)
  const noexcept
{
#if MSL_DISABLE_STRICT_MODE
  return m_pointer <=> 0u;
#else
  const auto* const self = get();

  if (self == nullptr) {
    return tag() <=> 0u;
  }
  return get() <=> nullptr;
#endif
}

#endif /* MSL_POINTERS_TAGGED_PTR_HPP */
