// test_id: cpp03.policy.default_debug
// api_ids: [cpp.policy.default, cpp.policy.panic]
// req_ids: [REQ-04]
// patterns: [POLICY-MATRIX, STL-COMPILE-PARITY]
/*
 * Notes:
 *   - This translation unit is intentionally compiled without an explicit
 *     SSTL_ON_ERROR definition from CMake.
 *   - It undefines NDEBUG before including SSTL headers so the probe checks the
 *     debug-build default independent of the surrounding build type.
 *   - The test does not intentionally trigger panic behavior; it verifies the
 *     selected policy and that the default panic hook is present.
 */
#ifdef NDEBUG
# undef NDEBUG
#endif

#include <sstl/vector.hpp>

#include "test_harness.hpp"

#if SSTL_ON_ERROR != SSTL_PANIC
# error Debug default policy must be SSTL_PANIC when SSTL_ON_ERROR is unset.
#endif

#ifndef SSTL_PANIC_HOOK_DEFINED
# error Debug default panic policy must provide a default panic hook.
#endif

static void debug_default_policy_is_panic() {
  sstl::vector<int, 1> v;
  SSTL_TEST_EQ(v.size(), 0u);
  SSTL_TEST_ASSERT(v.push_back(7));
}

int main() {
  const sstl_test::test_case tests[] = {
    {"debug_default_policy_is_panic", debug_default_policy_is_panic}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
