// test_id: cpp03.list.transfer.static_pool
// api_ids: [cpp.list.merge, cpp.list.splice, cpp.list.try_back, cpp.list.try_front, cpp.list.try_merge, cpp.list.try_splice]
// req_ids: [REQ-03, REQ-05, REQ-06]
// patterns: [CAPACITY-BOUNDARY, ITER-INVALIDATION]
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

static void cross_container_splice_is_all_or_nothing() {
  sstl_test::noalloc_guard guard;
  sstl::list<int, 3> src;
  sstl::list<int, 2> dst;
  SSTL_TEST_ASSERT(src.push_back(1));
  SSTL_TEST_ASSERT(src.push_back(2));
  SSTL_TEST_ASSERT(src.push_back(3));
  SSTL_TEST_ASSERT(dst.push_back(9));
  SSTL_TEST_ASSERT(!dst.try_splice(dst.end(), src));
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(!dst.splice(dst.end(), src));
#endif
  SSTL_TEST_EQ(src.size(), 3u);
  SSTL_TEST_EQ(dst.size(), 1u);
  SSTL_TEST_EQ(*src.try_front(), 1);
  SSTL_TEST_EQ(*src.try_back(), 3);
  dst.clear();
  SSTL_TEST_ASSERT(dst.try_splice(dst.end(), src, src.begin()));
  SSTL_TEST_EQ(src.size(), 2u);
  SSTL_TEST_EQ(dst.size(), 1u);
  SSTL_TEST_EQ(*dst.begin(), 1);
}

static void merge_try_aliases_preserve_all_or_nothing_capacity() {
  sstl_test::noalloc_guard guard;
  sstl::list<int, 4> dst;
  sstl::list<int, 3> src;
  SSTL_TEST_ASSERT(dst.push_back(1));
  SSTL_TEST_ASSERT(dst.push_back(4));
  SSTL_TEST_ASSERT(src.push_back(2));
  SSTL_TEST_ASSERT(src.push_back(3));
  SSTL_TEST_ASSERT(src.push_back(5));
  SSTL_TEST_ASSERT(!dst.try_merge(src));
  SSTL_TEST_EQ(dst.size(), 2u);
  SSTL_TEST_EQ(src.size(), 3u);
  dst.clear();
  SSTL_TEST_ASSERT(dst.push_back(1));
  SSTL_TEST_ASSERT(dst.try_merge(src));
  SSTL_TEST_EQ(dst.size(), 4u);
  SSTL_TEST_EQ(src.size(), 0u);
  sstl::list<int, 4>::iterator it = dst.begin();
  SSTL_TEST_EQ(*it, 1);
  ++it;
  SSTL_TEST_EQ(*it, 2);
  ++it;
  SSTL_TEST_EQ(*it, 3);
  ++it;
  SSTL_TEST_EQ(*it, 5);
  sstl::list<int, 2> empty;
  SSTL_TEST_ASSERT(empty.try_front() == 0);
  SSTL_TEST_ASSERT(empty.try_back() == 0);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"cross_container_splice_is_all_or_nothing", cross_container_splice_is_all_or_nothing},
    {"merge_try_aliases_preserve_all_or_nothing_capacity", merge_try_aliases_preserve_all_or_nothing_capacity}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
