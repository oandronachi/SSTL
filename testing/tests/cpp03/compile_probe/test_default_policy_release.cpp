// test_id: cpp03.policy.default_release
// api_ids: [cpp.policy.default, cpp.policy.return]
// req_ids: [REQ-04]
// patterns: [POLICY-MATRIX, STL-COMPILE-PARITY]
/*
 * Notes:
 *   - This translation unit is intentionally compiled without an explicit
 *     SSTL_ON_ERROR definition from CMake.
 *   - It defines NDEBUG before including SSTL headers so the probe checks the
 *     release-build default independent of the surrounding build type.
 *   - RETURN mode must not install the default panic hook because ordinary
 *     failures are represented by return sentinels.
 */
#ifndef NDEBUG
# define NDEBUG 1
#endif

#include <sstl/vector.hpp>

#include "test_harness.hpp"

#if SSTL_ON_ERROR != SSTL_RETURN
# error Release default policy must be SSTL_RETURN when SSTL_ON_ERROR is unset.
#endif

#ifdef SSTL_PANIC_HOOK_DEFINED
# error Release default RETURN policy must not install the panic hook.
#endif

static void release_default_policy_is_return() {
  sstl::vector<int, 1> v;
  SSTL_TEST_ASSERT(v.try_at(0) == 0);
  SSTL_TEST_ASSERT(!v.try_pop_back(0));
  SSTL_TEST_ASSERT(v.push_back(7));
  SSTL_TEST_ASSERT(!v.push_back(8));
}

int main() {
  const sstl_test::test_case tests[] = {
    {"release_default_policy_is_return", release_default_policy_is_return}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
