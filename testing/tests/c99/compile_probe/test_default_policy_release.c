/* test_id: c99.policy.default_release */
/* api_ids: [c.policy.default, c.policy.return] */
/* req_ids: [REQ-04, REQ-07] */
/* patterns: [POLICY-MATRIX, C-API-PARITY] */
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

#include <sstl/c/sstl_vector.h>

#include "test_harness.h"

#if SSTL_ON_ERROR != SSTL_RETURN
# error Release default policy must be SSTL_RETURN when SSTL_ON_ERROR is unset.
#endif

#ifdef SSTL_PANIC_HOOK_DEFINED
# error Release default RETURN policy must not install the panic hook.
#endif

SSTL_VECTOR_DECLARE(default_release_vec, int, 1)
SSTL_VECTOR_DEFINE(default_release_vec, int, 1)

static void release_default_policy_is_return(void) {
  default_release_vec v;
  default_release_vec_init(&v);
  SSTL_C_ASSERT(default_release_vec_at(&v, 0u) == 0);
  SSTL_C_ASSERT(!default_release_vec_try_pop_back(&v, 0));
  SSTL_C_ASSERT(default_release_vec_push_back(&v, 7));
  SSTL_C_ASSERT(!default_release_vec_push_back(&v, 8));
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"release_default_policy_is_return", release_default_policy_is_return}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
