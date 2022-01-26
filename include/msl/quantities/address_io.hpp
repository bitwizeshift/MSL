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
#ifndef MSL_QUANTITIES_ADDRESS_IO_HPP
#define MSL_QUANTITIES_ADDRESS_IO_HPP

#if defined(_MSC_VER)
# pragma once
#endif

#include "msl/quantities/address.hpp"

#include <ostream> // std::basic_ostream
#include <iomanip> // std::hex

namespace msl {

  template <typename CharT, typename Traits>
  auto operator<<(std::basic_ostream<CharT,Traits>& o, address value)
    -> std::basic_ostream<CharT,Traits>&;

} // namespace msl

template <typename CharT, typename Traits>
inline
auto msl::operator<<(std::basic_ostream<CharT,Traits>& o, address value)
  -> std::basic_ostream<CharT,Traits>&
{
  static constexpr auto digits = sizeof(address) / 4u;

  const auto flags = o.flags();

  o << "0x";
  o << std::setw(digits) << std::setfill('0') << std::hex << value.value();

  o.flags(flags);

  return o;
}


#endif /* MSL_QUANTITIES_ADDRESS_IO_HPP */