// test_id: cpp03.string.capacity.nul_terminated
// api_ids: [cpp.string.push_back]
// req_ids: [REQ-02, REQ-03, REQ-04, REQ-05]
// patterns: [AUDIT-NOALLOC, CAPACITY-BOUNDARY]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <sstl/string.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void string_stays_nul_terminated_at_capacity() {
  sstl_test::noalloc_guard guard;
  sstl::string<3> s;
  SSTL_TEST_ASSERT(s.push_back('a'));
  SSTL_TEST_ASSERT(s.push_back('b'));
  SSTL_TEST_ASSERT(s.push_back('c'));
  SSTL_TEST_ASSERT(s.full());
  SSTL_TEST_ASSERT(!s.push_back('d'));
  SSTL_TEST_EQ(s.size(), 3u);
  SSTL_TEST_EQ(s.c_str()[0], 'a');
  SSTL_TEST_EQ(s.c_str()[1], 'b');
  SSTL_TEST_EQ(s.c_str()[2], 'c');
  SSTL_TEST_EQ(s.c_str()[3], '\0');
}

int main() {
  const sstl_test::test_case tests[] = {
    {"string_stays_nul_terminated_at_capacity", string_stays_nul_terminated_at_capacity}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
