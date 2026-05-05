/* test_id: c99.string.extended_ops */
/* api_ids: [c.string.append_n, c.string.assign, c.string.insert, c.string.erase, c.string.replace, c.string.pop_back, c.string.resize, c.string.swap] */
/* req_ids: [REQ-03, REQ-05, REQ-06, REQ-07] */
/* patterns: [AUDIT-NOALLOC, CAPACITY-BOUNDARY, C-API-PARITY] */
/*
 * Notes:
 *   - The generated C string remains null-terminated after each mutator.
 *   - Growth failures are checked for all-or-nothing behavior so callers can safely retry with a smaller operation.
 *   - The assertions use c_str rather than private data layout except where size is the public API under test.
 */
#include <sstl/c/sstl_string.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_STRING_DECLARE(c_ext_string, 8)
SSTL_STRING_DEFINE(c_ext_string, 8)

static int cstr_eq(const char* a, const char* b) {
  size_t i = 0u;
  while (a[i] && b[i] && a[i] == b[i]) ++i;
  return a[i] == b[i];
}

static void assign_insert_erase_replace_and_append_n_work(void) {
  c_ext_string s;
  sstl_c_noalloc_begin();
  c_ext_string_init(&s);
  SSTL_C_ASSERT(c_ext_string_assign(&s, "ace"));
  SSTL_C_ASSERT(c_ext_string_insert_char(&s, 1u, 'b'));
  SSTL_C_ASSERT(c_ext_string_insert(&s, 3u, "d"));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "abcde"));
  SSTL_C_ASSERT(c_ext_string_erase(&s, 1u, 2u));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "ade"));
  SSTL_C_ASSERT(c_ext_string_replace(&s, 1u, 1u, "BC"));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "aBCe"));
  SSTL_C_ASSERT(c_ext_string_append_n(&s, "fg", 2u));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "aBCefg"));
  SSTL_C_ASSERT(!c_ext_string_append_n(&s, "toolong", 7u));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "aBCefg"));
  sstl_c_noalloc_end();
}

static void pop_back_resize_and_swap_work(void) {
  c_ext_string s;
  c_ext_string other;
  sstl_c_noalloc_begin();
  c_ext_string_init(&s);
  c_ext_string_init(&other);
  SSTL_C_ASSERT(c_ext_string_assign(&s, "abc"));
  SSTL_C_ASSERT(c_ext_string_pop_back(&s));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "ab"));
  SSTL_C_ASSERT(c_ext_string_size(&s) == 2u);
  SSTL_C_ASSERT(c_ext_string_resize(&s, 5u, 'x'));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "abxxx"));
  SSTL_C_ASSERT(c_ext_string_resize(&s, 2u, '\0'));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "ab"));
  SSTL_C_ASSERT(!c_ext_string_resize(&s, 9u, 'z'));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "ab"));
  SSTL_C_ASSERT(c_ext_string_assign(&other, "Q"));
  c_ext_string_swap(&s, &other);
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&s), "Q"));
  SSTL_C_ASSERT(cstr_eq(c_ext_string_c_str(&other), "ab"));
  c_ext_string_init(&s);
  SSTL_C_ASSERT(!c_ext_string_pop_back(&s));
  SSTL_C_ASSERT(c_ext_string_empty(&s));
  SSTL_C_ASSERT(c_ext_string_c_str(&s)[0] == '\0');
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"assign_insert_erase_replace_and_append_n_work", assign_insert_erase_replace_and_append_n_work},
    {"pop_back_resize_and_swap_work", pop_back_resize_and_swap_work}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
