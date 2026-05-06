// test_id: cpp03.map.erase
// api_ids: [cpp.map.erase]
// req_ids: [REQ-05, REQ-06]
// patterns: [STL-ORACLE-EQUIV, AUDIT-NOALLOC]
/*
 * Notes:
 *   - Erase is checked by iterator and by key because callers use both forms in normal map code.
 *   - The iterator-return contract is important: it lets a loop erase while walking without a second lookup.
 *   - The sequence removes leaf, one-child, and two-child positions from the tree-shaped insertion order.
 */
#include <sstl/map.hpp>
#include <sstl/set.hpp>
#include <sstl/flat_set.hpp>
#include <sstl/unordered_map.hpp>
#include <sstl/unordered_set.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void erase_by_iterator_returns_successor_and_preserves_order() {
  sstl_test::noalloc_guard guard;
  sstl::map<int, int, 8> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(4, 40)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(6, 60)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(3, 30)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(5, 50)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(7, 70)).second);

  sstl::map<int, int, 8>::iterator next = m.erase(m.find(4));
  SSTL_TEST_ASSERT(next != m.end());
  SSTL_TEST_EQ(next->first, 5);
  SSTL_TEST_ASSERT(m.find(4) == m.end());
  SSTL_TEST_EQ(m.size(), 6u);

  int expected = 1;
  for (sstl::map<int, int, 8>::iterator it = m.begin(); it != m.end(); ++it) {
    if (expected == 4) ++expected;
    SSTL_TEST_EQ(it->first, expected);
    ++expected;
  }
}

static void erase_by_key_reports_zero_or_one() {
  sstl_test::noalloc_guard guard;
  sstl::map<int, int, 4> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(3, 30)).second);
  SSTL_TEST_EQ(m.erase(1), 1u);
  SSTL_TEST_EQ(m.erase(1), 0u);
  SSTL_TEST_ASSERT(m.find(1) == m.end());
  SSTL_TEST_ASSERT(m.find(2) != m.end());
  SSTL_TEST_ASSERT(m.find(3) != m.end());
}

static void partial_range_erase_stops_at_live_successor() {
  sstl_test::noalloc_guard guard;
  sstl::map<int, int, 8> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(3, 30)).second);
  sstl::map<int, int, 8>::iterator map_last = m.find(3);
  sstl::map<int, int, 8>::iterator map_returned = m.erase(m.find(2), map_last);
  SSTL_TEST_EQ(m.size(), 2u);
  SSTL_TEST_ASSERT(m.find(2) == m.end());
  SSTL_TEST_ASSERT(m.find(1) != m.end());
  SSTL_TEST_ASSERT(m.find(3) != m.end());
  SSTL_TEST_ASSERT(map_returned == map_last);
  SSTL_TEST_EQ(map_returned->first, 3);

  sstl::set<int, 8> s;
  SSTL_TEST_ASSERT(s.insert(2).second);
  SSTL_TEST_ASSERT(s.insert(1).second);
  SSTL_TEST_ASSERT(s.insert(3).second);
  sstl::set<int, 8>::iterator set_last = s.find(3);
  sstl::set<int, 8>::iterator set_returned = s.erase(s.find(2), set_last);
  SSTL_TEST_EQ(s.size(), 2u);
  SSTL_TEST_ASSERT(s.find(2) == s.end());
  SSTL_TEST_ASSERT(s.find(1) != s.end());
  SSTL_TEST_ASSERT(s.find(3) != s.end());
  SSTL_TEST_ASSERT(set_returned == set_last);
  SSTL_TEST_EQ(*set_returned, 3);
}

static void erase_preserves_existing_successor_iterators() {
  sstl_test::noalloc_guard guard;
  sstl::map<int, int, 8> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(3, 30)).second);
  sstl::map<int, int, 8>::iterator held = m.find(3);
  sstl::map<int, int, 8>::iterator returned = m.erase(m.find(2));
  SSTL_TEST_ASSERT(returned == held);
  SSTL_TEST_EQ(held->first, 3);
  SSTL_TEST_EQ(held->second, 30);

  sstl::set<int, 8> s;
  SSTL_TEST_ASSERT(s.insert(2).second);
  SSTL_TEST_ASSERT(s.insert(1).second);
  SSTL_TEST_ASSERT(s.insert(3).second);
  sstl::set<int, 8>::iterator set_held = s.find(3);
  sstl::set<int, 8>::iterator set_returned = s.erase(s.find(2));
  SSTL_TEST_ASSERT(set_returned == set_held);
  SSTL_TEST_EQ(*set_held, 3);
}

static void flat_set_range_erase_returns_normalized_successor() {
  sstl_test::noalloc_guard guard;
  sstl::flat_set<int, 8> fs;
  SSTL_TEST_ASSERT(fs.insert(1).second);
  SSTL_TEST_ASSERT(fs.insert(2).second);
  SSTL_TEST_ASSERT(fs.insert(3).second);
  sstl::flat_set<int, 8>::iterator returned = fs.erase(fs.find(1), fs.find(3));
  SSTL_TEST_EQ(fs.size(), 1u);
  SSTL_TEST_ASSERT(fs.find(1) == fs.end());
  SSTL_TEST_ASSERT(fs.find(2) == fs.end());
  SSTL_TEST_ASSERT(fs.find(3) != fs.end());
  SSTL_TEST_ASSERT(returned == fs.find(3));
  SSTL_TEST_EQ(*returned, 3);
}

#if SSTL_ON_ERROR == SSTL_RETURN
static void reversed_range_erase_returns_sentinel_without_mutation() {
  sstl_test::noalloc_guard guard;
  sstl::map<int, int, 8> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(3, 30)).second);

  sstl::map<int, int, 8>::iterator map_returned = m.erase(m.find(3), m.find(2));
  SSTL_TEST_ASSERT(map_returned == m.end());
  SSTL_TEST_EQ(m.size(), 3u);
  SSTL_TEST_ASSERT(m.find(1) != m.end());
  SSTL_TEST_ASSERT(m.find(2) != m.end());
  SSTL_TEST_ASSERT(m.find(3) != m.end());

  sstl::set<int, 8> s;
  SSTL_TEST_ASSERT(s.insert(1).second);
  SSTL_TEST_ASSERT(s.insert(2).second);
  SSTL_TEST_ASSERT(s.insert(3).second);

  sstl::set<int, 8>::iterator set_returned = s.erase(s.find(3), s.find(2));
  SSTL_TEST_ASSERT(set_returned == s.end());
  SSTL_TEST_EQ(s.size(), 3u);
  SSTL_TEST_ASSERT(s.find(1) != s.end());
  SSTL_TEST_ASSERT(s.find(2) != s.end());
  SSTL_TEST_ASSERT(s.find(3) != s.end());
}

static void unordered_reversed_range_erase_returns_sentinel_without_mutation() {
  sstl_test::noalloc_guard guard;
  sstl::unordered_map<int, int, 8> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(3, 30)).second);

  sstl::unordered_map<int, int, 8>::iterator map_returned = m.erase(m.find(3), m.find(2));
  SSTL_TEST_ASSERT(map_returned == m.end());
  SSTL_TEST_EQ(m.size(), 3u);
  SSTL_TEST_ASSERT(m.find(1) != m.end());
  SSTL_TEST_ASSERT(m.find(2) != m.end());
  SSTL_TEST_ASSERT(m.find(3) != m.end());

  sstl::unordered_set<int, 8> s;
  SSTL_TEST_ASSERT(s.insert(1).second);
  SSTL_TEST_ASSERT(s.insert(2).second);
  SSTL_TEST_ASSERT(s.insert(3).second);

  sstl::unordered_set<int, 8>::iterator set_returned = s.erase(s.find(3), s.find(2));
  SSTL_TEST_ASSERT(set_returned == s.end());
  SSTL_TEST_EQ(s.size(), 3u);
  SSTL_TEST_ASSERT(s.find(1) != s.end());
  SSTL_TEST_ASSERT(s.find(2) != s.end());
  SSTL_TEST_ASSERT(s.find(3) != s.end());
}
#endif

int main() {
  const sstl_test::test_case tests[] = {
    {"erase_by_iterator_returns_successor_and_preserves_order", erase_by_iterator_returns_successor_and_preserves_order},
    {"erase_by_key_reports_zero_or_one", erase_by_key_reports_zero_or_one},
    {"partial_range_erase_stops_at_live_successor", partial_range_erase_stops_at_live_successor},
    {"erase_preserves_existing_successor_iterators", erase_preserves_existing_successor_iterators},
    {"flat_set_range_erase_returns_normalized_successor", flat_set_range_erase_returns_normalized_successor}
#if SSTL_ON_ERROR == SSTL_RETURN
    , {"reversed_range_erase_returns_sentinel_without_mutation", reversed_range_erase_returns_sentinel_without_mutation},
    {"unordered_reversed_range_erase_returns_sentinel_without_mutation", unordered_reversed_range_erase_returns_sentinel_without_mutation}
#endif
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
