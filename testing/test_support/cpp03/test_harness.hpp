/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This is a tiny dependency-light harness so SSTL tests can compile in strict C99/C++03 without Catch2 or Unity being present locally.
 *   - The output format is intentionally simple and easy to scan in CTest logs.
 *   - Failures abort immediately because these tests are contract probes, not long-running diagnostic collectors.
 */
#ifndef SSTL_TEST_SUPPORT_CPP03_TEST_HARNESS_HPP
#define SSTL_TEST_SUPPORT_CPP03_TEST_HARNESS_HPP

#include <cstdio>
#include <cstdlib>

namespace sstl_test {

typedef void (*test_fn)();

struct test_case {
  const char* name;
  test_fn run;
};

inline void fail(const char* expr, const char* file, int line) {
  std::fprintf(stderr, "%s:%d: assertion failed: %s\n", file, line, expr);
  /*
    Use exit rather than abort so Windows does not show modal crash dialogs
    during ordinary red/green test runs.  A failed contract test should be a
    clean non-zero process result that CTest can report in the console.
  */
  std::exit(1);
}

inline int run_all(const test_case* tests, int count) {
  for (int i = 0; i != count; ++i) {
    tests[i].run();
    std::printf("ok %d - %s\n", i + 1, tests[i].name);
  }
  return 0;
}

} // namespace sstl_test

#define SSTL_TEST_ASSERT(expr) \
  do { if (!(expr)) { ::sstl_test::fail(#expr, __FILE__, __LINE__); } } while (0)

#define SSTL_TEST_EQ(a, b) SSTL_TEST_ASSERT((a) == (b))
#define SSTL_TEST_NE(a, b) SSTL_TEST_ASSERT((a) != (b))

#endif
