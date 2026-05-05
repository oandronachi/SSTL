// test_id: cpp03.forward_list.algorithms
// api_ids: [cpp.forward_list.remove, cpp.forward_list.remove_if, cpp.forward_list.unique, cpp.forward_list.sort, cpp.forward_list.merge]
// req_ids: [REQ-03, REQ-05, REQ-06]
// patterns: [CAPACITY-BOUNDARY, STL-ORACLE-EQUIV, AUDIT-NOALLOC]
/*
 * Notes:
 *   - forward_list has no backward links, so these operations exercise the before-begin path repeatedly.
 *   - sort uses linked-node relinking, not a scratch buffer, and merge keeps the destination-owned transfer rule.
 *   - The checks intentionally read through iterators after each operation to verify public order only.
 */
#include <sstl/forward_list.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static bool is_even_value(int x) { return (x % 2) == 0; }

static sstl::forward_list<int, 8>::iterator append_value(sstl::forward_list<int, 8>& l,
                                                         sstl::forward_list<int, 8>::iterator tail,
                                                         int value) {
  return l.insert_after(tail, value);
}

static void remove_unique_and_sort_relink_nodes() {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 8> l;
  sstl::forward_list<int, 8>::iterator tail = l.before_begin();
  tail = append_value(l, tail, 3);
  tail = append_value(l, tail, 1);
  tail = append_value(l, tail, 2);
  tail = append_value(l, tail, 2);
  tail = append_value(l, tail, 4);
  l.remove_if(is_even_value);
  l.remove(9);
  l.sort();
  l.unique();
  int expected[] = {1, 3};
  unsigned index = 0u;
  for (sstl::forward_list<int, 8>::iterator it = l.begin(); it != l.end(); ++it, ++index) {
    SSTL_TEST_EQ(*it, expected[index]);
  }
  SSTL_TEST_EQ(index, 2u);
}

static void merge_preserves_sorted_order_and_source_on_capacity_failure() {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 8> left;
  sstl::forward_list<int, 8> right;
  sstl::forward_list<int, 2> small;
  sstl::forward_list<int, 8>::iterator left_tail = left.before_begin();
  sstl::forward_list<int, 8>::iterator right_tail = right.before_begin();
  left_tail = append_value(left, left_tail, 1);
  left_tail = append_value(left, left_tail, 4);
  right_tail = append_value(right, right_tail, 2);
  right_tail = append_value(right, right_tail, 3);
  SSTL_TEST_ASSERT(left.merge(right));
  SSTL_TEST_ASSERT(right.empty());
  int expected[] = {1, 2, 3, 4};
  unsigned index = 0u;
  for (sstl::forward_list<int, 8>::iterator it = left.begin(); it != left.end(); ++it, ++index) {
    SSTL_TEST_EQ(*it, expected[index]);
  }

  SSTL_TEST_ASSERT(small.insert_after(small.before_begin(), 5) != small.end());
  SSTL_TEST_ASSERT(small.insert_after(small.before_begin(), 1) != small.end());
  right_tail = right.before_begin();
  right_tail = append_value(right, right_tail, 6);
  right_tail = append_value(right, right_tail, 7);
  SSTL_TEST_ASSERT(!small.merge(right));
  SSTL_TEST_EQ(small.size(), 2u);
  SSTL_TEST_EQ(right.size(), 2u);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"remove_unique_and_sort_relink_nodes", remove_unique_and_sort_relink_nodes},
    {"merge_preserves_sorted_order_and_source_on_capacity_failure", merge_preserves_sorted_order_and_source_on_capacity_failure}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
