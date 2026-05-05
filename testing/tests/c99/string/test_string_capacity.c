/* test_id: c99.string.capacity.nul_terminated */
/* api_ids: [c.string.push_back] */
/* req_ids: [REQ-02, REQ-03, REQ-04, REQ-07] */
/* patterns: [AUDIT-NOALLOC, CAPACITY-BOUNDARY, C-API-PARITY] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <sstl/c/sstl_string.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_STRING_DECLARE(tiny_string, 3)
SSTL_STRING_DEFINE(tiny_string, 3)

static void string_stays_nul_terminated_at_capacity(void) {
  tiny_string s;
  sstl_c_noalloc_begin();
  tiny_string_init(&s);
  SSTL_C_ASSERT(tiny_string_push_back(&s, 'a'));
  SSTL_C_ASSERT(tiny_string_push_back(&s, 'b'));
  SSTL_C_ASSERT(tiny_string_push_back(&s, 'c'));
  SSTL_C_ASSERT(tiny_string_full(&s));
  SSTL_C_ASSERT(!tiny_string_push_back(&s, 'd'));
  SSTL_C_EQ(tiny_string_size(&s), 3u);
  SSTL_C_EQ(tiny_string_c_str(&s)[0], 'a');
  SSTL_C_EQ(tiny_string_c_str(&s)[1], 'b');
  SSTL_C_EQ(tiny_string_c_str(&s)[2], 'c');
  SSTL_C_EQ(tiny_string_c_str(&s)[3], '\0');
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"string_stays_nul_terminated_at_capacity", string_stays_nul_terminated_at_capacity}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
