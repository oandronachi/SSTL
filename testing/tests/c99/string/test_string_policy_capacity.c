/* test_id: c99.string.policy_capacity */
/* api_ids: [c.string.policy_capacity, c.string.try_at, c.string.at] */
/* req_ids: [REQ-03, REQ-04, REQ-07] */
/* patterns: [POLICY-MATRIX, CAPACITY-BOUNDARY, C-API-PARITY] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is policy-aware: RETURN checks sentinel/status results, PANIC checks the configured panic hook, and UB validates only legal operations because contract-violating calls may omit recovery checks.
 *   - The same source is compiled repeatedly with different SSTL_ON_ERROR values via local CMake presets.
 *   - The setjmp/longjmp panic path is a test harness device; production panic behavior only needs to call sstl_panic as specified.
 */
#define SSTL_NO_DEFAULT_PANIC 1
#include <setjmp.h>
#include <sstl/c/sstl_string.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_STRING_DECLARE(str_cap0, 0)
SSTL_STRING_DEFINE(str_cap0, 0)
SSTL_STRING_DECLARE(str_cap1, 1)
SSTL_STRING_DEFINE(str_cap1, 1)
SSTL_STRING_DECLARE(str_cap2, 2)
SSTL_STRING_DEFINE(str_cap2, 2)
SSTL_STRING_DECLARE(str_cap16, 16)
SSTL_STRING_DEFINE(str_cap16, 16)
SSTL_STRING_DECLARE(str_cap1024, 1024)
SSTL_STRING_DEFINE(str_cap1024, 1024)

static jmp_buf g_string_panic_env;
static int g_string_panic_count = 0;

void sstl_panic(const char* msg) {
  (void)msg;
  ++g_string_panic_count;
  longjmp(g_string_panic_env, 1);
}

#if SSTL_ON_ERROR == SSTL_RETURN
# define SSTL_C_EXPECT_STRING_FULL_FAIL(NAME, PTR) SSTL_C_ASSERT(!NAME##_push_back((PTR), 'z'))
#else
# define SSTL_C_EXPECT_STRING_FULL_FAIL(NAME, PTR) ((void)0)
#endif

#define CHECK_STRING_CAP(NAME, CAP) do { \
  NAME s; \
  size_t i; \
  NAME##_init(&s); \
  SSTL_C_EQ(NAME##_capacity(&s), (size_t)(CAP)); \
  for (i = 0u; i != (size_t)(CAP); ++i) { SSTL_C_ASSERT(NAME##_push_back(&s, 'a')); } \
  SSTL_C_ASSERT(NAME##_full(&s)); \
  SSTL_C_EXPECT_STRING_FULL_FAIL(NAME, &s); \
  SSTL_C_EQ(NAME##_size(&s), (size_t)(CAP)); \
  SSTL_C_EQ(NAME##_c_str(&s)[NAME##_size(&s)], '\0'); \
} while (0)

static void c_string_capacity_boundaries(void) {
  sstl_c_noalloc_begin();
  CHECK_STRING_CAP(str_cap0, 0);
  CHECK_STRING_CAP(str_cap1, 1);
  CHECK_STRING_CAP(str_cap2, 2);
  CHECK_STRING_CAP(str_cap16, 16);
  CHECK_STRING_CAP(str_cap1024, 1024);
  sstl_c_noalloc_end();
}

static void c_string_policy_matches_mode(void) {
  str_cap1 s;
  str_cap1_init(&s);
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_C_ASSERT(str_cap1_at(&s, 0u) == 0);
  SSTL_C_ASSERT(str_cap1_try_at(&s, 0u) == 0);
#elif SSTL_ON_ERROR == SSTL_PANIC
  g_string_panic_count = 0;
  if (setjmp(g_string_panic_env) == 0) {
    (void)str_cap1_at(&s, 0u);
    SSTL_C_ASSERT(0);
  }
  SSTL_C_EQ(g_string_panic_count, 1);
#elif SSTL_ON_ERROR == SSTL_UB
  SSTL_C_ASSERT(str_cap1_push_back(&s, 'x'));
  SSTL_C_EQ(*str_cap1_at(&s, 0u), 'x');
#else
# error Unknown SSTL_ON_ERROR value
#endif
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"c_string_capacity_boundaries", c_string_capacity_boundaries},
    {"c_string_policy_matches_mode", c_string_policy_matches_mode}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
