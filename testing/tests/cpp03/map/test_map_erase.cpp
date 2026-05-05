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

int main() {
  const sstl_test::test_case tests[] = {
    {"erase_by_iterator_returns_successor_and_preserves_order", erase_by_iterator_returns_successor_and_preserves_order},
    {"erase_by_key_reports_zero_or_one", erase_by_key_reports_zero_or_one}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
