// test_id: cpp03.vector.try_policy.return
// api_ids: [cpp.vector.try_at, cpp.vector.try_push_back, cpp.vector.try_pop_back, cpp.vector.try_insert]
// req_ids: [REQ-04, REQ-05]
// patterns: [POLICY-MATRIX, CAPACITY-BOUNDARY]
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

static void try_apis_return_sentinels_without_panic() {
  sstl_test::noalloc_guard guard;
  sstl::vector<int, 1> v;
  int out = 0;
  SSTL_TEST_ASSERT(v.try_at(0) == 0);
  SSTL_TEST_ASSERT(!v.try_pop_back(&out));
  SSTL_TEST_ASSERT(v.try_push_back(4));
  SSTL_TEST_ASSERT(!v.try_push_back(5));
  SSTL_TEST_ASSERT(v.try_insert(v.begin(), 3) == v.end());
  SSTL_TEST_ASSERT(v.try_at(0) != 0);
  SSTL_TEST_EQ(*v.try_at(0), 4);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"try_apis_return_sentinels_without_panic", try_apis_return_sentinels_without_panic}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
