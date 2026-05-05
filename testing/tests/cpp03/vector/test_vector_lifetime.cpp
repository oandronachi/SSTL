// test_id: cpp03.vector.lifetime.tracked
// api_ids: [cpp.vector.erase, cpp.vector.push_back]
// req_ids: [REQ-02, REQ-03, REQ-05]
// patterns: [LIFETIME, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It uses Tracked to make construction, copying, assignment, and destruction visible without relying on heap allocation or exceptions.
 *   - The important invariant is that every live element owned by SSTL is destroyed exactly once when erased, cleared, reset, overwritten, or when the container dies.
 *   - Counter expectations intentionally account for stack-local Tracked values that are not owned by SSTL.
 */
#include <sstl/vector.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"
#include "tracked.hpp"

static void vector_destroys_exactly_live_elements() {
  sstl_test::tracked::reset();
  {
    sstl_test::noalloc_guard guard;
    sstl::vector<sstl_test::tracked, 4> v;
    sstl_test::tracked a(1);
    sstl_test::tracked b(2);
    SSTL_TEST_ASSERT(v.push_back(a));
    SSTL_TEST_ASSERT(v.push_back(b));
    SSTL_TEST_EQ(v.size(), 2u);
    SSTL_TEST_EQ(sstl_test::tracked::live, 4);
    v.erase(v.begin());
    SSTL_TEST_EQ(v.size(), 1u);
    v.clear();
    SSTL_TEST_EQ(v.size(), 0u);
    SSTL_TEST_EQ(sstl_test::tracked::live, 2);
  }
  SSTL_TEST_EQ(sstl_test::tracked::live, 0);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"vector_destroys_exactly_live_elements", vector_destroys_exactly_live_elements}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
