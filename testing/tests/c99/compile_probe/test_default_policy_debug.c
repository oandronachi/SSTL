/* test_id: c99.policy.default_debug */
/* api_ids: [c.policy.default, c.policy.panic] */
/* req_ids: [REQ-04, REQ-07] */
/* patterns: [POLICY-MATRIX, C-API-PARITY] */
/*
 * Notes:
 *   - This translation unit is intentionally compiled without an explicit
 *     SSTL_ON_ERROR definition from CMake.
 *   - It undefines NDEBUG before including SSTL headers so the probe checks the
 *     debug-build default independent of the surrounding build type.
 *   - The test avoids triggering panic; it verifies policy selection and hook
 *     availability only.
 */
#ifdef NDEBUG
# undef NDEBUG
#endif

#include <sstl/c/sstl_vector.h>

#include "test_harness.h"

#if SSTL_ON_ERROR != SSTL_PANIC
# error Debug default policy must be SSTL_PANIC when SSTL_ON_ERROR is unset.
#endif

#ifndef SSTL_PANIC_HOOK_DEFINED
# error Debug default panic policy must provide a default panic hook.
#endif

SSTL_VECTOR_DECLARE(default_debug_vec, int, 1)
SSTL_VECTOR_DEFINE(default_debug_vec, int, 1)

static void debug_default_policy_is_panic(void) {
  default_debug_vec v;
  default_debug_vec_init(&v);
  SSTL_C_EQ(default_debug_vec_size(&v), 0u);
  SSTL_C_ASSERT(default_debug_vec_push_back(&v, 7));
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"debug_default_policy_is_panic", debug_default_policy_is_panic}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
