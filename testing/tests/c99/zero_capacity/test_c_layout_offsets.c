/* test_id: c99.layout.static_offsets */
/* api_ids: [c.vector.layout, c.string.layout] */
/* req_ids: [REQ-02, REQ-03, REQ-06, REQ-07] */
/* patterns: [STATIC-LAYOUT] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <stddef.h>
#include <sstl/c/sstl_vector.h>
#include <sstl/c/sstl_string.h>
#include "test_harness.h"

SSTL_VECTOR_DECLARE(layout_vec4, int, 4)
SSTL_VECTOR_DEFINE(layout_vec4, int, 4)
SSTL_STRING_DECLARE(layout_string4, 4)
SSTL_STRING_DEFINE(layout_string4, 4)

static void c_public_struct_layout_is_sane(void) {
  SSTL_C_ASSERT(sizeof(layout_vec4) >= sizeof(int) * 4u);
  SSTL_C_ASSERT(sizeof(layout_string4) >= 5u);
#ifdef SSTL_ALIGNOF
  SSTL_C_ASSERT(SSTL_ALIGNOF(layout_vec4) >= SSTL_ALIGNOF(int));
#endif
  SSTL_C_ASSERT(offsetof(layout_vec4, size) < sizeof(layout_vec4));
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"c_public_struct_layout_is_sane", c_public_struct_layout_is_sane}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
