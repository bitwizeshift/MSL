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

#include "msl/quantities/alignment.hpp"

#include <atomic>     // std::atomic
#include <exception>  // std::terminate
#include <functional> // std::invoke

namespace msl {
namespace {

  std::atomic<alignment_contract::violation_handler> g_handler;

  [[noreturn]]
  auto default_handler(bytes request, source_location where)
    -> void
  {
    throw alignment_contract::violation{request, where};
  }

} // namespace <anonymous>
} // namespace msl


//=============================================================================
// class : not_null_contract::violation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

msl::alignment_contract::violation::violation(bytes request, source_location where)
  : logic_error{
      "alignment contract violated by an invalid boundar: " +
      std::to_string(request.count()),
    },
    m_where{where}
{

}

//=============================================================================
// class : alignment_contract
//=============================================================================

//-----------------------------------------------------------------------------
// Contract
//-----------------------------------------------------------------------------

MSL_COLD
auto msl::alignment_contract::violate(bytes request, source_location where)
  -> void
{
  std::invoke(g_handler.load(), request, where);

  // If the handler above does not either throw an exception or terminate
  // execution, then we dial back to std::abort instead.
  std::abort();
}

auto msl::alignment_contract::get_violation_handler()
  noexcept -> violation_handler
{
  return g_handler.load();
}

auto msl::alignment_contract::set_violation_handler(violation_handler handler)
  noexcept -> violation_handler
{
  if (handler == nullptr) MSL_UNLIKELY {
    return g_handler.exchange(&default_handler);
  }
  return g_handler.exchange(handler);
}
