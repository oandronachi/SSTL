/* test_id: c99.string_view.contracts */
/* api_ids: [c.string_view.contract] */
/* req_ids: [REQ-05, REQ-06, REQ-07] */
/* patterns: [AUDIT-NOALLOC, C-API-PARITY] */
/*
 * Notes:
 *   - A C string_view is a non-owning pointer/length pair. These checks never
 *     expect copied character storage and therefore run under the no-allocation
 *     audit guard.
 *   - The tests cover both successful lookup behavior and RETURN-policy
 *     sentinel behavior for out-of-range access.
 *   - Prefix and suffix removal must only adjust the view metadata; the source
 *     character array is caller-owned and must remain untouched.
 */
#include <sstl/c/sstl_string_view.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_STRING_VIEW_DECLARE(c_test_sv)
SSTL_STRING_VIEW_DEFINE(c_test_sv)

static void construct_find_compare_and_prefix_suffix_work(void) {
  c_test_sv sv;
  sstl_c_noalloc_begin();
  sv = c_test_sv_make("abcabc", 6u);
  SSTL_C_EQ(c_test_sv_size(&sv), 6u);
  SSTL_C_ASSERT(!c_test_sv_empty(&sv));
  SSTL_C_EQ(*c_test_sv_data(&sv), 'a');
  SSTL_C_EQ(*c_test_sv_at(&sv, 2u), 'c');
  SSTL_C_EQ(*c_test_sv_try_at(&sv, 3u), 'a');
  SSTL_C_EQ(c_test_sv_find(&sv, "bc"), 1u);
  SSTL_C_EQ(c_test_sv_rfind(&sv, "bc"), 4u);
  SSTL_C_EQ(c_test_sv_find(&sv, "zz"), SSTL_STRING_VIEW_NPOS);
  SSTL_C_EQ(c_test_sv_compare(&sv, "abcabc"), 0);
  SSTL_C_ASSERT(c_test_sv_compare(&sv, "abcabd") < 0);
  SSTL_C_ASSERT(c_test_sv_compare(&sv, "abcabb") > 0);
  SSTL_C_ASSERT(c_test_sv_starts_with(&sv, "abc"));
  SSTL_C_ASSERT(c_test_sv_ends_with(&sv, "abc"));
  sstl_c_noalloc_end();
}

static void remove_prefix_suffix_updates_view_only(void) {
  const char text[] = "012345";
  c_test_sv sv;
  sstl_c_noalloc_begin();
  sv = c_test_sv_from_c_str(text);
  c_test_sv_remove_prefix(&sv, 2u);
  SSTL_C_EQ(c_test_sv_size(&sv), 4u);
  SSTL_C_EQ(*c_test_sv_data(&sv), '2');
  c_test_sv_remove_suffix(&sv, 2u);
  SSTL_C_EQ(c_test_sv_size(&sv), 2u);
  SSTL_C_EQ(c_test_sv_find(&sv, "23"), 0u);
  SSTL_C_EQ(text[0], '0');
  SSTL_C_EQ(text[5], '5');
  sstl_c_noalloc_end();
}

static void empty_and_bounds_return_sentinels(void) {
  c_test_sv sv;
  sstl_c_noalloc_begin();
  sv = c_test_sv_from_c_str(0);
  SSTL_C_ASSERT(c_test_sv_empty(&sv));
  SSTL_C_EQ(c_test_sv_find(&sv, ""), 0u);
  SSTL_C_EQ(c_test_sv_rfind(&sv, ""), 0u);
  SSTL_C_EQ(c_test_sv_find(&sv, "x"), SSTL_STRING_VIEW_NPOS);
  SSTL_C_ASSERT(c_test_sv_try_at(&sv, 0u) == 0);
  SSTL_C_ASSERT(c_test_sv_at(&sv, 0u) == 0);
  c_test_sv_remove_prefix(&sv, 99u);
  c_test_sv_remove_suffix(&sv, 99u);
  SSTL_C_EQ(c_test_sv_size(&sv), 0u);
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"construct_find_compare_and_prefix_suffix_work", construct_find_compare_and_prefix_suffix_work},
    {"remove_prefix_suffix_updates_view_only", remove_prefix_suffix_updates_view_only},
    {"empty_and_bounds_return_sentinels", empty_and_bounds_return_sentinels}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
