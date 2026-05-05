/* test_id: c99.policy.matrix.asserted */
/* api_ids: [c.policy.return, c.policy.panic, c.policy.ub, c.vector.at, c.vector.pop_back] */
/* req_ids: [REQ-04, REQ-07] */
/* patterns: [POLICY-MATRIX] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is policy-aware: RETURN checks sentinel/status results, PANIC checks the configured panic hook, and UB validates only legal operations because contract-violating calls may omit recovery checks.
 *   - The same source is compiled repeatedly with different SSTL_ON_ERROR values via local CMake presets.
 *   - The setjmp/longjmp panic path is a test harness device; production panic behavior only needs to call sstl_panic as specified.
 */
#define SSTL_NO_DEFAULT_PANIC 1
#include <setjmp.h>
#include <sstl/c/sstl_vector.h>
#include "test_harness.h"

SSTL_VECTOR_DECLARE(policy_vec, int, 1)
SSTL_VECTOR_DEFINE(policy_vec, int, 1)

static jmp_buf g_panic_env;
static int g_panic_count = 0;

void sstl_panic(const char* msg) {
  (void)msg;
  ++g_panic_count;
  longjmp(g_panic_env, 1);
}

static void policy_routed_c_access_matches_configured_mode(void) {
  policy_vec v;
  policy_vec_init(&v);

#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_C_ASSERT(policy_vec_at(&v, 0u) == 0);
  SSTL_C_ASSERT(!policy_vec_pop_back(&v, 0));
  SSTL_C_ASSERT(policy_vec_push_back(&v, 7));
  SSTL_C_ASSERT(!policy_vec_push_back(&v, 8));
  SSTL_C_EQ(policy_vec_size(&v), 1u);
#elif SSTL_ON_ERROR == SSTL_PANIC
  g_panic_count = 0;
  if (setjmp(g_panic_env) == 0) {
    (void)policy_vec_at(&v, 0u);
    SSTL_C_ASSERT(0);
  }
  SSTL_C_EQ(g_panic_count, 1);
  if (setjmp(g_panic_env) == 0) {
    (void)policy_vec_pop_back(&v, 0);
    SSTL_C_ASSERT(0);
  }
  SSTL_C_EQ(g_panic_count, 2);
#elif SSTL_ON_ERROR == SSTL_UB
  SSTL_C_ASSERT(policy_vec_push_back(&v, 7));
  SSTL_C_EQ(*policy_vec_at(&v, 0u), 7);
#else
# error Unknown SSTL_ON_ERROR value
#endif
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"policy_routed_c_access_matches_configured_mode", policy_routed_c_access_matches_configured_mode}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
