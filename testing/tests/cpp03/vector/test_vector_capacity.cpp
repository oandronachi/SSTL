// test_id: cpp03.vector.capacity.full.return
// api_ids: [cpp.vector.insert, cpp.vector.push_back]
// req_ids: [REQ-02, REQ-03, REQ-04]
// patterns: [AUDIT-NOALLOC, CAPACITY-BOUNDARY, POLICY-MATRIX]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <sstl/vector.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void push_to_full_fails_without_growth() {
  sstl_test::noalloc_guard guard;
  sstl::vector<int, 2> v;
  SSTL_TEST_ASSERT(v.empty());
  SSTL_TEST_ASSERT(v.push_back(10));
  SSTL_TEST_ASSERT(v.push_back(20));
  SSTL_TEST_ASSERT(v.full());
  SSTL_TEST_ASSERT(!v.push_back(30));
  SSTL_TEST_EQ(v.size(), 2u);
  SSTL_TEST_EQ(v[0], 10);
  SSTL_TEST_EQ(v[1], 20);
}

static void insert_when_full_returns_end() {
  sstl_test::noalloc_guard guard;
  sstl::vector<int, 2> v;
  SSTL_TEST_ASSERT(v.push_back(1));
  SSTL_TEST_ASSERT(v.push_back(3));
  SSTL_TEST_ASSERT(v.insert(v.begin() + 1, 2) == v.end());
  SSTL_TEST_EQ(v.size(), 2u);
  SSTL_TEST_EQ(v[0], 1);
  SSTL_TEST_EQ(v[1], 3);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"push_to_full_fails_without_growth", push_to_full_fails_without_growth},
    {"insert_when_full_returns_end", insert_when_full_returns_end}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
