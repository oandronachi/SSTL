// test_id: stress.cpp03.vector.long_cycles
// api_ids: [cpp.vector.push_back]
// req_ids: [REQ-02, REQ-03, REQ-05]
// patterns: [STRESS, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is a direct SSTL contract test; prefer changing the manifest and comments together when behavior changes.
 *   - The assertions are black-box: they verify public observable state rather than private representation.
 *   - If this test fails against an implementation, first compare the implementation API to the SSTL public contract.
 */
#include <sstl/vector.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void repeated_fill_clear_cycles_preserve_capacity() {
  sstl_test::noalloc_guard guard;
  sstl::vector<int, 32> v;
  for (int cycle = 0; cycle != 1000; ++cycle) {
    for (int i = 0; i != 32; ++i) {
      SSTL_TEST_ASSERT(v.push_back(i + cycle));
    }
    SSTL_TEST_ASSERT(v.full());
    SSTL_TEST_ASSERT(!v.push_back(9999));
    v.clear();
    SSTL_TEST_ASSERT(v.empty());
    SSTL_TEST_EQ(v.capacity(), 32u);
  }
}

int main() {
  const sstl_test::test_case tests[] = {
    {"repeated_fill_clear_cycles_preserve_capacity", repeated_fill_clear_cycles_preserve_capacity}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
