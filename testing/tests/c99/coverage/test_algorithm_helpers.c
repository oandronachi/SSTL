/* test_id: c99.algorithm.helpers.coverage */
/* api_ids: [c.algorithm.contract] */
/* req_ids: [REQ-03, REQ-06] */
/* patterns: [EDGE-BRANCH-COVERAGE, AUDIT-NOALLOC] */
/*
 * Notes:
 *   - The C helper algorithms are tiny inline functions, so one ordinary
 *     compile probe can leave comparison branches unvisited.
 *   - This test deliberately exercises less-than, equal, and greater-than
 *     outcomes for each comparison helper, plus empty and non-empty hashing.
 *   - The assertions use only stack data and the local C99 test harness.
 */
#include <sstl/c/sstl_algorithm.h>

#include "test_harness.h"

static void comparison_helpers_report_all_orderings(void) {
  int one = 1;
  int two = 2;
  int three = 3;
  unsigned one_u = 1u;
  unsigned two_u = 2u;
  unsigned three_u = 3u;
  const char* null_text = 0;
  const char* a = "a";
  const char* b = "b";
  const char* same = "same";
  const char* prefix = "prefix";
  const char* pre = "pre";
  const char* empty = "";
  const char* x = "x";

  SSTL_C_ASSERT(sstl_cmp_int_v(&one, &two) < 0);
  SSTL_C_ASSERT(sstl_cmp_int_v(&two, &one) > 0);
  SSTL_C_EQ(sstl_cmp_int_v(&three, &three), 0);

  SSTL_C_ASSERT(sstl_cmp_uint_v(&one_u, &two_u) < 0);
  SSTL_C_ASSERT(sstl_cmp_uint_v(&two_u, &one_u) > 0);
  SSTL_C_EQ(sstl_cmp_uint_v(&three_u, &three_u), 0);

  SSTL_C_ASSERT(sstl_cmp_str_v(&a, &b) < 0);
  SSTL_C_ASSERT(sstl_cmp_str_v(&b, &a) > 0);
  SSTL_C_EQ(sstl_cmp_str_v(&same, &same), 0);
  SSTL_C_ASSERT(sstl_cmp_str_v(&prefix, &pre) > 0);
  SSTL_C_ASSERT(sstl_cmp_str_v(&empty, &x) < 0);
  SSTL_C_ASSERT(sstl_cmp_str_v(&null_text, &a) < 0);
  SSTL_C_ASSERT(sstl_cmp_str_v(&a, &null_text) > 0);

  SSTL_C_ASSERT(sstl_eq_int_v(&one, &one));
  SSTL_C_ASSERT(!sstl_eq_int_v(&one, &two));
  SSTL_C_ASSERT(sstl_eq_uint_v(&one_u, &one_u));
  SSTL_C_ASSERT(!sstl_eq_uint_v(&one_u, &two_u));
  SSTL_C_ASSERT(sstl_eq_str_v(&same, &same));
  SSTL_C_ASSERT(!sstl_eq_str_v(&a, &b));
}

static void hash_helper_handles_empty_and_nonempty_ranges(void) {
  const unsigned char bytes[] = {0u, 1u, 2u, 255u};
  unsigned one_u = 1u;
  unsigned two_u = 2u;
  const char* text = "hash me";
  const char* empty_text = "";
  const char* null_text = 0;
  const size_t empty_hash = sstl_hash_bytes_v(bytes, 0u);
  const size_t full_hash = sstl_hash_bytes_v(bytes, sizeof(bytes));
  const size_t one_hash = sstl_hash_uint_v(&one_u);
  const size_t two_hash = sstl_hash_uint_v(&two_u);
  const size_t text_hash = sstl_hash_str_v(&text);
  const size_t empty_text_hash = sstl_hash_str_v(&empty_text);
  const size_t null_text_hash = sstl_hash_str_v(&null_text);

  SSTL_C_NE(empty_hash, 0u);
  SSTL_C_NE(full_hash, 0u);
  SSTL_C_NE(empty_hash, full_hash);
  SSTL_C_NE(one_hash, two_hash);
  SSTL_C_NE(text_hash, empty_text_hash);
  SSTL_C_EQ(empty_text_hash, null_text_hash);
}

static void default_panic_hook_is_referenced_when_present(void) {
  /*
    Panic-policy C builds expose a header-local default panic hook. This helper
    test does not assert panic behavior; it simply makes the hook an intentionally
    used symbol in this algorithm-only translation unit so strict `-Werror`
    builds do not reject the probe for containing an unused static function.
  */
#if SSTL_ON_ERROR == SSTL_PANIC && !defined(SSTL_NO_DEFAULT_PANIC)
  sstl_panic("coverage probe");
#else
  SSTL_C_ASSERT(1);
#endif
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"comparison_helpers_report_all_orderings", comparison_helpers_report_all_orderings},
    {"hash_helper_handles_empty_and_nonempty_ranges", hash_helper_handles_empty_and_nonempty_ranges},
    {"default_panic_hook_is_referenced_when_present", default_panic_hook_is_referenced_when_present}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
