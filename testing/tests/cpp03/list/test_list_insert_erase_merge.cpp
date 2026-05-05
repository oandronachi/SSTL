// test_id: cpp03.list.insert_erase_merge
// api_ids: [cpp.list.insert, cpp.list.erase, cpp.list.merge]
// req_ids: [REQ-03, REQ-05, REQ-06]
// patterns: [CAPACITY-BOUNDARY, STL-ORACLE-EQUIV, AUDIT-NOALLOC]
/*
 * Notes:
 *   - Insert and erase are iterator-positioned operations, so the tests check returned iterators as well as contents.
 *   - Merge is checked as an ordered destination-owned transfer: the source is emptied only after the destination can hold all nodes.
 *   - Capacity failure must be all-or-nothing because embedded callers cannot clean up partial cross-container transfers safely.
 */
#include <sstl/list.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void insert_and_erase_return_expected_iterators() {
  sstl_test::noalloc_guard guard;
  sstl::list<int, 4> l;
  SSTL_TEST_ASSERT(l.push_back(1));
  SSTL_TEST_ASSERT(l.push_back(3));
  sstl::list<int, 4>::iterator pos = l.begin();
  ++pos;
  sstl::list<int, 4>::iterator inserted = l.insert(pos, 2);
  SSTL_TEST_EQ(*inserted, 2);
  sstl::list<int, 4>::iterator next = l.erase(inserted);
  SSTL_TEST_ASSERT(next != l.end());
  SSTL_TEST_EQ(*next, 3);
  SSTL_TEST_EQ(l.size(), 2u);
}

static void erase_returns_nodes_to_intrusive_free_list() {
  sstl_test::noalloc_guard guard;
  sstl::list<int, 2> l;
  SSTL_TEST_ASSERT(l.push_back(1));
  SSTL_TEST_ASSERT(l.push_back(2));
  sstl::list<int, 2>::iterator first = l.begin();
  int* first_slot = &*first;
  SSTL_TEST_ASSERT(l.erase(first) != l.end());
  SSTL_TEST_ASSERT(l.push_back(3));
  SSTL_TEST_EQ(l.back(), 3);
  SSTL_TEST_ASSERT(&l.back() == first_slot);
}

static void merge_preserves_sorted_order_and_capacity_boundaries() {
  sstl_test::noalloc_guard guard;
  sstl::list<int, 5> left;
  sstl::list<int, 3> too_small;
  sstl::list<int, 3> right;
  SSTL_TEST_ASSERT(left.push_back(1));
  SSTL_TEST_ASSERT(left.push_back(4));
  SSTL_TEST_ASSERT(right.push_back(2));
  SSTL_TEST_ASSERT(right.push_back(3));
  SSTL_TEST_ASSERT(left.merge(right));
  SSTL_TEST_ASSERT(right.empty());
  int expected = 1;
  for (sstl::list<int, 5>::iterator it = left.begin(); it != left.end(); ++it, ++expected) {
    SSTL_TEST_EQ(*it, expected);
  }

  SSTL_TEST_ASSERT(too_small.push_back(1));
  SSTL_TEST_ASSERT(too_small.push_back(3));
  SSTL_TEST_ASSERT(right.push_back(2));
  SSTL_TEST_ASSERT(right.push_back(4));
  SSTL_TEST_ASSERT(!too_small.merge(right));
  SSTL_TEST_EQ(too_small.size(), 2u);
  SSTL_TEST_EQ(right.size(), 2u);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"insert_and_erase_return_expected_iterators", insert_and_erase_return_expected_iterators},
    {"erase_returns_nodes_to_intrusive_free_list", erase_returns_nodes_to_intrusive_free_list},
    {"merge_preserves_sorted_order_and_capacity_boundaries", merge_preserves_sorted_order_and_capacity_boundaries}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
