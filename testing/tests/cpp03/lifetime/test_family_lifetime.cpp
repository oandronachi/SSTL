// test_id: cpp03.family.lifetime.tracked
// api_ids: [cpp.deque.lifetime, cpp.list.lifetime, cpp.forward_list.lifetime, cpp.unordered_map.contract, cpp.unordered_set.contract, cpp.optional.lifetime, cpp.variant.lifetime]
// req_ids: [REQ-02, REQ-05, REQ-06]
// patterns: [LIFETIME, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It uses Tracked to make construction, copying, assignment, and destruction visible without relying on heap allocation or exceptions.
 *   - The important invariant is that every live element owned by SSTL is destroyed exactly once when erased, cleared, reset, overwritten, or when the container dies.
 *   - Counter expectations intentionally account for stack-local Tracked values that are not owned by SSTL.
 */
#include <sstl/deque.hpp>
#include <sstl/list.hpp>
#include <sstl/forward_list.hpp>
#include <sstl/unordered_map.hpp>
#include <sstl/unordered_set.hpp>
#include <sstl/optional.hpp>
#include <sstl/variant.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"
#include "tracked.hpp"

static void sequence_containers_destroy_tracked_elements() {
  sstl_test::tracked::reset();
  {
    sstl_test::noalloc_guard guard;
    sstl::deque<sstl_test::tracked, 3> d;
    sstl_test::tracked a(1);
    SSTL_TEST_ASSERT(d.push_back(a));
    SSTL_TEST_ASSERT(d.push_front(a));
    d.pop_back();
    d.clear();
    SSTL_TEST_EQ(sstl_test::tracked::live, 1);
  }
  SSTL_TEST_EQ(sstl_test::tracked::live, 0);

  sstl_test::tracked::reset();
  {
    sstl_test::noalloc_guard guard;
    sstl::list<sstl_test::tracked, 2> l;
    sstl_test::tracked a(2);
    SSTL_TEST_ASSERT(l.push_back(a));
    SSTL_TEST_ASSERT(l.push_front(a));
    l.clear();
    SSTL_TEST_EQ(sstl_test::tracked::live, 1);
  }
  SSTL_TEST_EQ(sstl_test::tracked::live, 0);

  sstl_test::tracked::reset();
  {
    sstl_test::noalloc_guard guard;
    sstl::forward_list<sstl_test::tracked, 2> fl;
    sstl_test::tracked a(3);
    SSTL_TEST_ASSERT(fl.insert_after(fl.before_begin(), a) != fl.end());
    SSTL_TEST_ASSERT(fl.insert_after(fl.before_begin(), a) != fl.end());
    fl.clear();
    SSTL_TEST_EQ(sstl_test::tracked::live, 1);
  }
  SSTL_TEST_EQ(sstl_test::tracked::live, 0);
}

static void utility_containers_destroy_active_alternatives() {
  sstl_test::tracked::reset();
  {
    sstl_test::noalloc_guard guard;
    sstl::optional<sstl_test::tracked> opt;
    sstl_test::tracked a(4);
    opt = a;
    SSTL_TEST_ASSERT(opt);
    opt.reset();
    SSTL_TEST_EQ(sstl_test::tracked::live, 1);
  }
  SSTL_TEST_EQ(sstl_test::tracked::live, 0);

  sstl_test::tracked::reset();
  {
    sstl_test::noalloc_guard guard;
    sstl_test::tracked a(5);
    sstl::variant2<sstl_test::tracked, int> var(a);
    SSTL_TEST_EQ(var.index(), 0u);
    var = 7;
    SSTL_TEST_EQ(var.index(), 1u);
    SSTL_TEST_EQ(sstl_test::tracked::live, 1);
  }
  SSTL_TEST_EQ(sstl_test::tracked::live, 0);
}

static void unordered_containers_copy_and_iterate_live_values() {
  /*
   * The unordered containers store elements in raw storage slots. A compiler
   * generated copy would duplicate those bytes without running Tracked's copy
   * constructor, and both containers would later destroy the same logical
   * lifetime. The final live-counter assertion makes that bug visible.
   */
  sstl_test::tracked::reset();
  {
    sstl_test::noalloc_guard guard;
    typedef sstl::unordered_map<int, sstl_test::tracked, 4, 7> map_type;
    map_type original;
    sstl_test::tracked ten(10);
    sstl_test::tracked twenty(20);

    SSTL_TEST_ASSERT(original.insert(sstl::make_pair(1, ten)).second);
    SSTL_TEST_ASSERT(original.insert(sstl::make_pair(8, twenty)).second);

    map_type::iterator null_map_it;
    ++null_map_it;
    SSTL_TEST_ASSERT(null_map_it == map_type::iterator());

    unsigned count = 0u;
    int key_sum = 0;
    int value_sum = 0;
    for (map_type::const_iterator it = original.begin(); it != original.end(); ++it) {
      ++count;
      key_sum += it->first;
      value_sum += it->second.value;
    }
    SSTL_TEST_EQ(count, 2u);
    SSTL_TEST_EQ(key_sum, 9);
    SSTL_TEST_EQ(value_sum, 30);

    map_type copied(original);
    SSTL_TEST_EQ(copied.size(), 2u);
    copied = copied;
    SSTL_TEST_EQ(copied.size(), 2u);
    SSTL_TEST_ASSERT(copied.find(1) != copied.end());
    SSTL_TEST_ASSERT(copied.find(8) != copied.end());
    SSTL_TEST_EQ(copied.find(1)->second.value, 10);
    SSTL_TEST_EQ(copied.find(8)->second.value, 20);
    SSTL_TEST_ASSERT(!(original.begin() == copied.begin()));
    SSTL_TEST_ASSERT(original.begin() != copied.begin());
    map_type::iterator first = original.begin();
    map_type::iterator second = first;
    ++second;
    SSTL_TEST_ASSERT(!(first == second));

    map_type assigned;
    assigned = copied;
    SSTL_TEST_EQ(assigned.size(), 2u);
    SSTL_TEST_ASSERT(assigned.find(1) != assigned.end());
    SSTL_TEST_ASSERT(assigned.find(8) != assigned.end());
    SSTL_TEST_ASSERT(assigned.find(3) == assigned.end());
    map_type::const_iterator null_const_map_it;
    ++null_const_map_it;
    SSTL_TEST_ASSERT(null_const_map_it == map_type::const_iterator());
    const map_type& const_original = original;
    map_type::const_iterator cfirst = const_original.begin();
    map_type::const_iterator csecond = cfirst;
    ++csecond;
    SSTL_TEST_ASSERT(!(cfirst == csecond));
  }
  SSTL_TEST_EQ(sstl_test::tracked::live, 0);

  sstl_test::tracked::reset();
  {
    sstl_test::noalloc_guard guard;
    typedef sstl::unordered_set<sstl_test::tracked, 4, 7> set_type;
    set_type original;
    sstl_test::tracked one(1);
    sstl_test::tracked two(2);

    SSTL_TEST_ASSERT(original.insert(one).second);
    SSTL_TEST_ASSERT(original.insert(two).second);
    SSTL_TEST_ASSERT(!original.insert(one).second);

    unsigned count = 0u;
    int value_sum = 0;
    for (set_type::const_iterator it = original.begin(); it != original.end(); ++it) {
      ++count;
      value_sum += it->value;
    }
    SSTL_TEST_EQ(count, 2u);
    SSTL_TEST_EQ(value_sum, 3);

    set_type copied(original);
    SSTL_TEST_EQ(copied.size(), 2u);
    SSTL_TEST_ASSERT(copied.find(one) != copied.end());
    SSTL_TEST_ASSERT(copied.find(two) != copied.end());

    set_type assigned;
    assigned = copied;
    SSTL_TEST_EQ(assigned.size(), 2u);
    SSTL_TEST_ASSERT(assigned.find(one) != assigned.end());
    SSTL_TEST_ASSERT(assigned.find(two) != assigned.end());
  }
  SSTL_TEST_EQ(sstl_test::tracked::live, 0);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"sequence_containers_destroy_tracked_elements", sequence_containers_destroy_tracked_elements},
    {"utility_containers_destroy_active_alternatives", utility_containers_destroy_active_alternatives},
    {"unordered_containers_copy_and_iterate_live_values", unordered_containers_copy_and_iterate_live_values}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
