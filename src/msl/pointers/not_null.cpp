#include "msl/pointers/not_null.hpp"
#include "msl/utilities/intrinsics.hpp"

MSL_COLD
auto msl::detail::throw_null_pointer_error()
  -> void
{
  throw not_null_contract_violation{};
}
