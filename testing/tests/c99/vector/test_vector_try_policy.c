/* test_id: c99.vector.try_policy.return */
/* api_ids: [c.vector.try_at, c.vector.try_push_back, c.vector.try_pop_back] */
/* req_ids: [REQ-04, REQ-07] */
/* patterns: [POLICY-MATRIX, C-API-PARITY] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is policy-aware: RETURN checks sentinel/status results, PANIC checks the configured panic hook, and UB validates only legal operations because contract-violating calls may omit recovery checks.
 *   - The same source is compiled repeatedly with different SSTL_ON_ERROR values via local CMake presets.
 *   - The setjmp/longjmp panic path is a test harness device; production panic behavior only needs to call sstl_panic as specified.
 */
#include <sstl/c/sstl_vector.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_VECTOR_DECLARE(try_vec1, int, 1)
SSTL_VECTOR_DEFINE(try_vec1, int, 1)

static void try_apis_return_status_without_panic(void) {
  try_vec1 v;
  int out = 0;
  sstl_c_noalloc_begin();
  try_vec1_init(&v);
  SSTL_C_ASSERT(try_vec1_try_at(&v, 0u) == 0);
  SSTL_C_ASSERT(!try_vec1_try_pop_back(&v, &out));
  SSTL_C_ASSERT(try_vec1_try_push_back(&v, 4));
  SSTL_C_ASSERT(!try_vec1_try_push_back(&v, 5));
  SSTL_C_ASSERT(try_vec1_try_at(&v, 0u) != 0);
  SSTL_C_EQ(*try_vec1_try_at(&v, 0u), 4);
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"try_apis_return_status_without_panic", try_apis_return_status_without_panic}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
