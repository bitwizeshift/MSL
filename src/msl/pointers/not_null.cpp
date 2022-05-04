#include "msl/pointers/not_null.hpp"

#include <atomic>     // std::atomic<T>
#include <cstdlib>    // std::abort
#include <functional> // std::invoke
#include <string>     // std::string

namespace msl {
namespace {

  [[noreturn]]
  auto default_handler(source_location where)
    -> void
  {
    throw not_null_contract::violation{where};
  }

  auto g_handler = std::atomic<msl::not_null_contract::violation_handler>{
    &default_handler
  };
} // namespace <anonymous>
} // namespace msl

//=============================================================================
// class : not_null_contract::violation
//=============================================================================

//-----------------------------------------------------------------------------
// Constructors / Assignment
//-----------------------------------------------------------------------------

msl::not_null_contract::violation::violation(source_location where)
  : logic_error{
      std::string{"check_not_null invoked with null pointer in function '"} +
      where.function_name() + "' at location " +
      where.file_name() + ":" + std::to_string(where.line()) +
      "."
    },
    m_where{where}
{

}

//=============================================================================
// class : not_null_contract
//=============================================================================

//-----------------------------------------------------------------------------
// Contract
//-----------------------------------------------------------------------------

MSL_COLD
auto msl::not_null_contract::violate(source_location where)
  -> void
{
  std::invoke(g_handler.load(), where);

  // If the handler above does not either throw an exception or terminate
  // execution, then we dial back to std::abort instead.
  std::abort();
}

auto msl::not_null_contract::get_violation_handler()
  noexcept -> violation_handler
{
  return g_handler.load();
}

auto msl::not_null_contract::set_violation_handler(violation_handler handler)
  noexcept -> violation_handler
{
  if (handler == nullptr) MSL_UNLIKELY {
    return g_handler.exchange(&default_handler);
  }
  return g_handler.exchange(handler);
}
