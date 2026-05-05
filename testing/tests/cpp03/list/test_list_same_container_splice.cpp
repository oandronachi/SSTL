// test_id: cpp03.list.same_container_splice.iterator_preservation
// api_ids: [cpp.list.splice.same_container]
// req_ids: [REQ-05, REQ-06]
// patterns: [ITER-INVALIDATION, STL-ORACLE-EQUIV]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It covers static-node-pool semantics, especially the difference between same-container relinking and cross-container destination-owned transfer.
 *   - All-or-nothing behavior matters because a failed transfer must not partially consume destination capacity or erase source elements.
 *   - Iterator expectations are about public validity and preserved values, not private node addresses.
 */
#include <sstl/list.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void same_container_splice_preserves_iterators_to_moved_nodes() {
  sstl_test::noalloc_guard guard;
  sstl::list<int, 5> l;
  SSTL_TEST_ASSERT(l.push_back(1));
  SSTL_TEST_ASSERT(l.push_back(2));
  SSTL_TEST_ASSERT(l.push_back(3));
  sstl::list<int, 5>::iterator two = l.begin();
  ++two;
  sstl::list<int, 5>::iterator old_two = two;
  l.splice(l.begin(), l, two);
  SSTL_TEST_EQ(*l.begin(), 2);
  SSTL_TEST_EQ(*old_two, 2);
  SSTL_TEST_EQ(l.size(), 3u);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"same_container_splice_preserves_iterators_to_moved_nodes", same_container_splice_preserves_iterators_to_moved_nodes}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
