// test_id: cpp03.policy.matrix.asserted
// api_ids: [cpp.policy.return, cpp.policy.panic, cpp.policy.ub, cpp.vector.at, cpp.vector.pop_back]
// req_ids: [REQ-04]
// patterns: [POLICY-MATRIX]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is policy-aware: RETURN checks sentinel/status results, PANIC checks the configured panic hook, and UB validates only legal operations because contract-violating calls may omit recovery checks.
 *   - The same source is compiled repeatedly with different SSTL_ON_ERROR values via local CMake presets.
 *   - The setjmp/longjmp panic path is a test harness device; production panic behavior only needs to call sstl_panic as specified.
 */
#define SSTL_NO_DEFAULT_PANIC 1
#include <setjmp.h>
#include <sstl/vector.hpp>
#include "test_harness.hpp"

static jmp_buf g_panic_env;
static int g_panic_count = 0;

extern "C" void sstl_panic(const char*) {
  ++g_panic_count;
  longjmp(g_panic_env, 1);
}

static void policy_routed_access_matches_configured_mode() {
  sstl::vector<int, 1> v;

#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(v.try_at(0) == 0);
  SSTL_TEST_ASSERT(!v.try_pop_back(0));
  SSTL_TEST_ASSERT(v.push_back(7));
  SSTL_TEST_ASSERT(!v.push_back(8));
  SSTL_TEST_EQ(v.size(), 1u);
#elif SSTL_ON_ERROR == SSTL_PANIC
  g_panic_count = 0;
  if (setjmp(g_panic_env) == 0) {
    (void)v.at(0);
    SSTL_TEST_ASSERT(0 && "at(0) on empty vector must panic");
  }
  SSTL_TEST_EQ(g_panic_count, 1);
  if (setjmp(g_panic_env) == 0) {
    v.pop_back();
    SSTL_TEST_ASSERT(0 && "pop_back on empty vector must panic");
  }
  SSTL_TEST_EQ(g_panic_count, 2);
#elif SSTL_ON_ERROR == SSTL_UB
  SSTL_TEST_ASSERT(v.push_back(7));
  SSTL_TEST_EQ(v.at(0), 7);
  v.pop_back();
  SSTL_TEST_ASSERT(v.empty());
#else
# error Unknown SSTL_ON_ERROR value
#endif
}

int main() {
  const sstl_test::test_case tests[] = {
    {"policy_routed_access_matches_configured_mode", policy_routed_access_matches_configured_mode}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
