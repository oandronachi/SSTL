/* test_id: c99.vector.capacity.parameter_set */
/* api_ids: [c.vector.push_back] */
/* req_ids: [REQ-03, REQ-04, REQ-06, REQ-07] */
/* patterns: [CAPACITY-BOUNDARY, C-API-PARITY] */
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

SSTL_VECTOR_DECLARE(vec_cap0, int, 0)
SSTL_VECTOR_DEFINE(vec_cap0, int, 0)
SSTL_VECTOR_DECLARE(vec_cap1, int, 1)
SSTL_VECTOR_DEFINE(vec_cap1, int, 1)
SSTL_VECTOR_DECLARE(vec_cap2, int, 2)
SSTL_VECTOR_DEFINE(vec_cap2, int, 2)
SSTL_VECTOR_DECLARE(vec_cap16, int, 16)
SSTL_VECTOR_DEFINE(vec_cap16, int, 16)
SSTL_VECTOR_DECLARE(vec_cap1024, int, 1024)
SSTL_VECTOR_DEFINE(vec_cap1024, int, 1024)

#define CHECK_CAP(NAME, CAP) do { \
  NAME v; \
  size_t i; \
  NAME##_init(&v); \
  SSTL_C_EQ(NAME##_capacity(&v), (size_t)(CAP)); \
  for (i = 0u; i != (size_t)(CAP); ++i) { SSTL_C_ASSERT(NAME##_push_back(&v, (int)i)); } \
  SSTL_C_ASSERT(NAME##_full(&v)); \
  SSTL_C_ASSERT(!NAME##_push_back(&v, 999)); \
  SSTL_C_EQ(NAME##_size(&v), (size_t)(CAP)); \
} while (0)

static void capacity_boundary_required_parameter_set(void) {
  sstl_c_noalloc_begin();
  CHECK_CAP(vec_cap0, 0);
  CHECK_CAP(vec_cap1, 1);
  CHECK_CAP(vec_cap2, 2);
  CHECK_CAP(vec_cap16, 16);
  CHECK_CAP(vec_cap1024, 1024);
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"capacity_boundary_required_parameter_set", capacity_boundary_required_parameter_set}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
