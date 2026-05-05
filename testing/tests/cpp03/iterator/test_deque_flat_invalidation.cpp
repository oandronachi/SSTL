// test_id: cpp03.iterator.documented_deviations
// api_ids: [cpp.deque.contract, cpp.flat_map.contract]
// req_ids: [REQ-05, REQ-06]
// patterns: [ITER-INVALIDATION]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It verifies iterator invalidation through a debug/test hook rather than by dereferencing invalidated iterators, which would itself be undefined behavior.
 *   - Implementations must provide SSTL_TEST_ITERATOR_IS_VALID(container, iterator) or iterator.is_valid_for(container) in iterator-debug builds.
 *   - The contract being checked is the documented stricter invalidation for deque and flat containers.
 */
#include <sstl/deque.hpp>
#include <sstl/flat_map.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"
#include "iterator_contract.hpp"

static void deque_mutation_invalidates_all_iterators_by_generation() {
  sstl_test::noalloc_guard guard;
  sstl::deque<int, 4> d;
  SSTL_TEST_ASSERT(d.push_back(1));
  SSTL_TEST_ASSERT(d.push_back(2));
  sstl::deque<int, 4>::iterator old = d.begin();
  SSTL_TEST_EXPECT_VALID(d, old);
  SSTL_TEST_ASSERT(d.push_front(0));
  SSTL_TEST_EXPECT_INVALIDATED(d, old);
  SSTL_TEST_EQ(d.front(), 0);
  SSTL_TEST_EQ(d.back(), 2);
}

static void deque_iterator_random_access_operations_match_declared_category() {
  /*
   * deque::iterator advertises random_access_iterator_tag. This test exercises
   * the operations generic code expects from that category, including offset
   * arithmetic, iterator distance, indexed access, and relational comparison.
   */
  sstl_test::noalloc_guard guard;
  sstl::deque<int, 5> d;
  SSTL_TEST_ASSERT(d.push_back(10));
  SSTL_TEST_ASSERT(d.push_back(20));
  SSTL_TEST_ASSERT(d.push_back(30));
  SSTL_TEST_ASSERT(d.push_back(40));

  sstl::deque<int, 5>::iterator first = d.begin();
  sstl::deque<int, 5>::iterator third = first + 2;
  SSTL_TEST_EQ(first[0], 10);
  SSTL_TEST_EQ(first[2], 30);
  SSTL_TEST_EQ(*third, 30);
  SSTL_TEST_EQ(third - first, 2);
  SSTL_TEST_EQ(*(1 + first), 20);
  SSTL_TEST_ASSERT(first < third);
  SSTL_TEST_ASSERT(third > first);
  SSTL_TEST_ASSERT(!(third < first));
  SSTL_TEST_ASSERT(first <= third);
  SSTL_TEST_ASSERT(first <= first);
  SSTL_TEST_ASSERT(!(third <= first));
  SSTL_TEST_ASSERT(third >= first);
  third -= 1;
  SSTL_TEST_EQ(*third, 20);
  third += 2;
  SSTL_TEST_EQ(*third, 40);
  SSTL_TEST_EQ(d.end() - d.begin(), 4);

  sstl::deque<int, 5> other;
  SSTL_TEST_ASSERT(other.push_back(99));
  SSTL_TEST_ASSERT(!(first == other.begin()));
  SSTL_TEST_ASSERT(!(first < other.begin()));
  SSTL_TEST_ASSERT(!(first <= other.begin()));
  SSTL_TEST_ASSERT(!d.is_valid_iterator(d.end() + 1));

  const sstl::deque<int, 5>& cd = d;
  const sstl::deque<int, 5>& cother = other;
  sstl::deque<int, 5>::const_iterator cfirst = cd.begin();
  sstl::deque<int, 5>::const_iterator cthird = cfirst + 2;
  SSTL_TEST_EQ(cfirst[1], 20);
  SSTL_TEST_EQ(*(1 + cfirst), 20);
  SSTL_TEST_EQ(*cthird, 30);
  SSTL_TEST_EQ(cd.end() - cd.begin(), 4);
  SSTL_TEST_ASSERT(cfirst < cthird);
  SSTL_TEST_ASSERT(!(cthird < cfirst));
  SSTL_TEST_ASSERT(!(cfirst < cother.begin()));
  SSTL_TEST_ASSERT(!cd.is_valid_iterator(cd.end() + 1));
}

static void flat_map_mutation_invalidates_iterators_by_generation() {
  sstl_test::noalloc_guard guard;
  sstl::flat_map<int, int, 4> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 10)).second);
  m.revalidate_iterators();
  sstl::flat_map<int, int, 4>::iterator old = m.begin();
  SSTL_TEST_EXPECT_VALID(m, old);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_EXPECT_INVALIDATED(m, old);
  sstl::flat_map<int, int, 4>::iterator fresh = m.begin();
  SSTL_TEST_EXPECT_VALID(m, fresh);
  SSTL_TEST_ASSERT(m.find(1) != m.end());
  SSTL_TEST_ASSERT(m.find(2) != m.end());
}

int main() {
  const sstl_test::test_case tests[] = {
    {"deque_mutation_invalidates_all_iterators_by_generation", deque_mutation_invalidates_all_iterators_by_generation},
    {"deque_iterator_random_access_operations_match_declared_category", deque_iterator_random_access_operations_match_declared_category},
    {"flat_map_mutation_invalidates_iterators_by_generation", flat_map_mutation_invalidates_iterators_by_generation}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
