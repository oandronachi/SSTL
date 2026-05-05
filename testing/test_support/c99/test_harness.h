/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This is a tiny dependency-light harness so SSTL tests can compile in strict C99/C++03 without Catch2 or Unity being present locally.
 *   - The output format is intentionally simple and easy to scan in CTest logs.
 *   - Failures stop immediately because these tests are contract probes, not long-running diagnostic collectors.
 */
#ifndef SSTL_TEST_SUPPORT_C99_TEST_HARNESS_H
#define SSTL_TEST_SUPPORT_C99_TEST_HARNESS_H

#include <stdio.h>
#include <stdlib.h>

typedef void (*sstl_c_test_fn)(void);

typedef struct sstl_c_test_case {
  const char* name;
  sstl_c_test_fn run;
} sstl_c_test_case;

static void sstl_c_fail(const char* expr, const char* file, int line) {
  fprintf(stderr, "%s:%d: assertion failed: %s\n", file, line, expr);
  /*
    Keep failure reporting console-only.  Hard process breaks trigger visible
    Windows crash pop-ups, which makes iterative local testing miserable.
  */
  exit(1);
}

static int sstl_c_run_all(const sstl_c_test_case* tests, int count) {
  int i;
  for (i = 0; i != count; ++i) {
    tests[i].run();
    printf("ok %d - %s\n", i + 1, tests[i].name);
  }
  return 0;
}

#define SSTL_C_ASSERT(expr) \
  do { if (!(expr)) { sstl_c_fail(#expr, __FILE__, __LINE__); } } while (0)

#define SSTL_C_EQ(a, b) SSTL_C_ASSERT((a) == (b))
#define SSTL_C_NE(a, b) SSTL_C_ASSERT((a) != (b))

#endif
