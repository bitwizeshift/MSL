///////////////////////////////////////////////////////////////////////////////
/// \file attributes.hpp
///
/// \brief This header defines various "attribute" alias wrappers around
///        types so that consuming code can be written in a self-documenting
///        manner.
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
#ifndef MSL_UTILITIES_ATTRIBUTES_HPP
#define MSL_UTILITIES_ATTRIBUTES_HPP

namespace msl {

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A type wrapper "attribute" that represents a T value that is
  ///        uninitialized.
  ///
  /// The semantic meaning behind this wrapper is meant to indicate that a
  /// pointer refers to an uninitialized address (e.g. no active object exists)
  ///
  /// This does not actually alter the wrapped type in any way, it's meant to be
  /// a means of writing self-documenting code, so that code can read:
  /// \code
  /// uninitialized<int>* p = new int;
  /// \endcode
  ///
  /// \tparam T the type that is uninitialized
  /////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using uninitialized = T;

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A type wrapper "attribute" that represents a T value that points
  ///        to an object with a valid lifetime
  ///
  /// The semantic meaning behind this wrapper is meant to indicate that a
  /// pointer refers to an address that contains an object that has an active
  /// lifetime.
  ///
  /// This does not actually alter the wrapped type in any way, it's meant to be
  /// a means of writing self-documenting code, so that code can read:
  /// \code
  /// initialized<int>* p = new int{5};
  /// \endcode
  ///
  /// \tparam T the type that is initialized
  /////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using initialized = T;

  /////////////////////////////////////////////////////////////////////////////
  /// \brief A type wrapper "attribute" that represents a T value that is an
  ///        owner of the wrapped object.
  ///
  /// The semantic meaning behind this wrapper is meant to indicate that a
  /// pointer is the sole owner of that address, and must be deallocated to
  /// avoid a resource leak. However this also works with any form of resource
  ///
  /// This does not preserve any software-semantics like move-only behavior,
  /// since it actually just acts as a simple aliasing wrapper so that code can
  /// be written in a more self-documenting fashion. For example:
  ///
  /// \code
  /// owner<initialized<int>*> p = new int{5};
  /// \endcode
  ///
  /// \tparam T the type that is initialized
  /////////////////////////////////////////////////////////////////////////////
  template <typename T>
  using owner = T;
}

#endif /* MSL_UTILITIES_ATTRIBUTES_HPP */
