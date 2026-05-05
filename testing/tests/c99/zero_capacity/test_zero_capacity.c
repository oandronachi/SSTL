/* test_id: c99.owning.zero_capacity */
/* api_ids: [c.vector.push_back, c.string.push_back] */
/* req_ids: [REQ-01, REQ-03, REQ-04, REQ-06, REQ-07] */
/* patterns: [CAPACITY-BOUNDARY, STATIC-LAYOUT] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <sstl/c/sstl_vector.h>
#include <sstl/c/sstl_string.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_VECTOR_DECLARE(zero_vec, int, 0)
SSTL_VECTOR_DEFINE(zero_vec, int, 0)
SSTL_STRING_DECLARE(zero_string, 0)
SSTL_STRING_DEFINE(zero_string, 0)

static void zero_capacity_vector_is_empty_full_and_nonzero_sizeof(void) {
  zero_vec v;
  sstl_c_noalloc_begin();
  zero_vec_init(&v);
  SSTL_C_ASSERT(sizeof(v) > 0u);
  SSTL_C_EQ(zero_vec_capacity(&v), 0u);
  SSTL_C_ASSERT(zero_vec_empty(&v));
  SSTL_C_ASSERT(zero_vec_full(&v));
  SSTL_C_ASSERT(!zero_vec_push_back(&v, 1));
  SSTL_C_EQ(zero_vec_size(&v), 0u);
  sstl_c_noalloc_end();
}

static void zero_capacity_string_is_empty_full_and_nul_terminated(void) {
  zero_string s;
  sstl_c_noalloc_begin();
  zero_string_init(&s);
  SSTL_C_ASSERT(sizeof(s) > 0u);
  SSTL_C_EQ(zero_string_capacity(&s), 0u);
  SSTL_C_ASSERT(zero_string_empty(&s));
  SSTL_C_ASSERT(zero_string_full(&s));
  SSTL_C_ASSERT(!zero_string_push_back(&s, 'x'));
  SSTL_C_EQ(zero_string_size(&s), 0u);
  SSTL_C_EQ(zero_string_c_str(&s)[0], '\0');
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"zero_capacity_vector_is_empty_full_and_nonzero_sizeof", zero_capacity_vector_is_empty_full_and_nonzero_sizeof},
    {"zero_capacity_string_is_empty_full_and_nul_terminated", zero_capacity_string_is_empty_full_and_nul_terminated}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
