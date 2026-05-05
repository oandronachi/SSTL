/* test_id: c99.vector.capacity.full.return */
/* api_ids: [c.vector.push_back] */
/* req_ids: [REQ-02, REQ-03, REQ-04, REQ-07] */
/* patterns: [AUDIT-NOALLOC, CAPACITY-BOUNDARY, C-API-PARITY] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <sstl/c/sstl_vector.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_VECTOR_DECLARE(int_vec2, int, 2)
SSTL_VECTOR_DEFINE(int_vec2, int, 2)

static void push_to_full_fails_without_growth(void) {
  int_vec2 v;
  int out = 0;
  sstl_c_noalloc_begin();
  int_vec2_init(&v);
  SSTL_C_ASSERT(int_vec2_empty(&v));
  SSTL_C_ASSERT(int_vec2_push_back(&v, 10));
  SSTL_C_ASSERT(int_vec2_push_back(&v, 20));
  SSTL_C_ASSERT(int_vec2_full(&v));
  SSTL_C_ASSERT(!int_vec2_push_back(&v, 30));
  SSTL_C_EQ(int_vec2_size(&v), 2u);
  SSTL_C_EQ(*int_vec2_at(&v, 0u), 10);
  SSTL_C_EQ(*int_vec2_at(&v, 1u), 20);
  SSTL_C_ASSERT(int_vec2_pop_back(&v, &out));
  SSTL_C_EQ(out, 20);
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"push_to_full_fails_without_growth", push_to_full_fails_without_growth}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
