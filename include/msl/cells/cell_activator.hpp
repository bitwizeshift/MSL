///////////////////////////////////////////////////////////////////////////////
/// \file cell_activator.hpp
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
#ifndef MSL_CELLS_CELL_TRAITS_HPP
#define MSL_CELLS_CELL_TRAITS_HPP

#include "msl/cells/cell.hpp"
#include "msl/cells/active_cell.hpp"
#include "msl/cells/unsafe_cell.hpp"

#include "msl/utilities/uninitialized_storage.hpp"
#include "msl/utilities/assert.hpp"

namespace msl {

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A static utility class for managing cell activation logic
  ///
  /// This encapsulates the various functionality surrounding cell activation
  /// and deactivation
  /////////////////////////////////////////////////////////////////////////////
  class cell_activator
  {
  public:

    cell_activator() = delete;
    ~cell_activator() = delete;

    //-------------------------------------------------------------------------
    // Activation
    //-------------------------------------------------------------------------

    /// \brief Constructs an object into the cell denoted by \p c
    ///
    /// If construction is successful, the cell \p c will compare null, and a
    /// new active_cell will be returned in its place that contains a pointer
    /// to the constructed object
    ///
    /// \note This function does not participate in overload resolution unless
    ///       T is not an array type, and if the underlying type T of the cell
    ///       can be constructed by Args
    ///
    /// \param c the cell to activate
    /// \param args the arguments to pass to the constructor of the cell's type
    /// \return an active_cell pointing to c.data() on success
    template <typename T, std::size_t Align, typename...Args,
              typename = std::enable_if_t<!std::is_array_v<T> && std::is_constructible_v<T,Args...>>>
    static active_cell<T, Align> activate(cell<T, Align>&& c, Args&&...args)
      noexcept(std::is_nothrow_constructible_v<T,Args...>);

    /// \{
    /// \brief Default-constructs objects into the cell denoted by \p c
    ///
    /// If construction is successful, the cell \p c will compare null, and a
    /// new active_cell will be returned in its place that contains a pointer
    /// to the constructed object sequence.
    ///
    /// \param c the cell to actiavte
    /// \return an active_cell pointing to c.data() on success
    template <typename T, std::size_t Align>
    static active_cell<T[], Align> activate_array(cell<T[], Align>&& c)
      noexcept(std::is_nothrow_default_constructible_v<T>);
    template <typename T, std::size_t N, std::size_t Align>
    static active_cell<T[N], Align> activate_array(cell<T[N], Align>&& c)
      noexcept(std::is_nothrow_default_constructible_v<T>);
    /// \}

    /// \{
    /// \brief Default-constructs objects into the cell denoted by \p c
    ///
    /// If construction is successful, the cell \p c will compare null, and a
    /// new active_cell will be returned in its place that contains a pointer
    /// to the constructed object sequence.
    ///
    /// \param c the cell to actiavte
    /// \return an active_cell pointing to c.data() on success
    template <typename T, std::size_t Align>
    static active_cell<T[], Align> activate_array(cell<T[], Align>&& c, const T& copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);
    template <typename T, std::size_t N, std::size_t Align>
    static active_cell<T[N], Align> activate_array(cell<T[N], Align>&& c, const T& copy)
      noexcept(std::is_nothrow_copy_constructible_v<T>);
    /// \}

    //-------------------------------------------------------------------------
    // Deactivation
    //-------------------------------------------------------------------------

    /// \{
    /// \brief Destroys the underlying object of \p c and returns an inactive
    ///        cell
    ///
    /// \note A program is ill-formed if the underlying referenced type's
    ///       destructor throws an exception
    ///
    /// \param c the active_cell to deactivate
    /// \return the inactive cell
    template <typename T, std::size_t Align>
    static cell<T, Align> deactivate(active_cell<T, Align> c) noexcept;
    template <typename T, std::size_t Align>
    static cell<T, Align> deactivate(active_cell<T[], Align> c) noexcept;
    template <typename T, std::size_t N, std::size_t Align>
    static cell<T, Align> deactivate(active_cell<T[N], Align> c) noexcept;
    /// \}
  };

} // namespace msl

//-----------------------------------------------------------------------------
// Activation
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align, typename... Args, typename>
inline msl::active_cell<T, Align>
  msl::cell_activator::activate(cell<T, Align>&& c, Args&&...args)
  noexcept(std::is_nothrow_constructible_v<T, Args...>)
{
  MSL_ASSERT(p.data() != nullptr);

  auto* p = uninitialized_storage::construct_at<T>(
    c.data(),
    std::forward<Args>(args)...
  );

  // defer setting to null until after construction, in case constructor throws
  c = cell<T, Align>{};

  return active_cell<T, Align>{p};
}

template <typename T, std::size_t Align>
inline msl::active_cell<T[], Align>
  msl::cell_activator::activate_array(cell<T[], Align>&& c)
  noexcept(std::is_nothrow_default_constructible_v<T>)
{
  MSL_ASSERT(p.data() != nullptr);

  auto* p = uninitialized_storage::construct_array_at<T>(
    c.data(),
    c.length().count()
  );

  auto result = msl::active_cell<T[], Align>{
    p,
    c.length()
  };

  // defer setting to null until after construction, in case constructor throws
  c = cell<T[], Align>{};

  return result;
}

template <typename T, std::size_t N, std::size_t Align>
inline msl::active_cell<T[N], Align>
  msl::cell_activator::activate_array(cell<T[N], Align>&& c)
  noexcept(std::is_nothrow_default_constructible_v<T>)
{
  MSL_ASSERT(p.data() != nullptr);

  auto* p = uninitialized_storage::construct_array_at<T>(
    c.data(),
    c.length().count()
  );

  auto result = msl::active_cell<T[], Align>{
    p,
    c.length()
  };

  // Set to null once activated
  c = cell<T[N], Align>{};

  return result;
}

template <typename T, std::size_t Align>
inline msl::active_cell<T[], Align>
  msl::cell_activator::activate_array(cell<T[], Align>&& c, const T& copy)
  noexcept(std::is_nothrow_copy_constructible_v<T>)
{
  MSL_ASSERT(p.data() != nullptr);

  auto* p = uninitialized_storage::construct_array_at<T>(
    c.data(),
    c.length().count(),
    copy
  );

  // defer setting to null until after construction, in case constructor throws
  auto result = msl::active_cell<T[], Align>{
    p,
    c.length()
  };

  // Set to null once activated
  c = cell<T[N], Align>{};

  return result;
}

template <typename T, std::size_t N, std::size_t Align>
inline msl::active_cell<T[N], Align>
  msl::cell_activator::activate_array(cell<T[N], Align>&& c, const T& copy)
  noexcept(std::is_nothrow_copy_constructible_v<T>)
{
  MSL_ASSERT(p.data() != nullptr);

  auto* p = uninitialized_storage::construct_array_at<T>(
    c.data(),
    c.length().count(),
    copy
  );

  auto result = msl::active_cell<T[], Align>{
    p,
    c.length()
  };

  // Set to null once activated
  c = cell<T[N], Align>{};

  return result;
}

//-----------------------------------------------------------------------------
// Deactivation
//-----------------------------------------------------------------------------

template <typename T, std::size_t Align>
inline msl::cell<T, Align>
  msl::cell_activator::deactivate(active_cell<T, Align> c)
  noexcept
{
  uninitialized_storage::destroy_at<T>(c.data());

  return cell<T, Align>{c.data()};
}

template <typename T, std::size_t Align>
inline msl::cell<T, Align>
  msl::cell_activator::deactivate(active_cell<T[], Align> c)
  noexcept
{
  uninitialized_storage::destroy_array_at<T>(c.data(), c.length().count());

  return cell<T[], Align>{c.data(), c.length()};
}

template <typename T, std::size_t N, std::size_t Align>
inline msl::cell<T, Align> msl::cell_activator::deactivate(active_cell<T[N], Align> c)
  noexcept
{
  uninitialized_storage::destroy_array_at<T>(c.data(), N);

  return cell<T[N], Align>{c.data()};
}
#endif /* MSL_CELLS_CELL_TRAITS_HPP */
