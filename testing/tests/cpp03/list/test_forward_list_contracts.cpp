// test_id: cpp03.forward_list.splice_after.contracts
// api_ids: [cpp.forward_list.erase_after, cpp.forward_list.front, cpp.forward_list.insert_after, cpp.forward_list.merge, cpp.forward_list.pop_front, cpp.forward_list.push_front, cpp.forward_list.splice_after, cpp.forward_list.try_front, cpp.forward_list.try_merge, cpp.forward_list.try_pop_front, cpp.forward_list.try_splice_after]
// req_ids: [REQ-03, REQ-05, REQ-06]
// patterns: [CAPACITY-BOUNDARY, ITER-INVALIDATION]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It covers static-node-pool semantics, especially the difference between same-container relinking and cross-container destination-owned transfer.
 *   - All-or-nothing behavior matters because a failed transfer must not partially consume destination capacity or erase source elements.
 *   - Iterator expectations are about public validity and preserved values, not private node addresses.
 */
#include <sstl/forward_list.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void forward_list_after_operations_and_transfer_are_capacity_checked() {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 3> src;
  sstl::forward_list<int, 1> dst;
  SSTL_TEST_ASSERT(src.insert_after(src.before_begin(), 3) != src.end());
  SSTL_TEST_ASSERT(src.insert_after(src.before_begin(), 2) != src.end());
  SSTL_TEST_ASSERT(src.insert_after(src.before_begin(), 1) != src.end());
  SSTL_TEST_ASSERT(dst.insert_after(dst.before_begin(), 9) != dst.end());
  SSTL_TEST_ASSERT(!dst.try_splice_after(dst.before_begin(), src));
  SSTL_TEST_ASSERT(dst.erase_after(dst.before_begin()) == dst.end());
  SSTL_TEST_ASSERT(dst.try_splice_after(dst.before_begin(), src, src.before_begin()));
  SSTL_TEST_EQ(dst.front(), 1);
  SSTL_TEST_EQ(*dst.begin(), 1);
}

static void erase_after_returns_nodes_to_intrusive_free_list() {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 2> list;
  sstl::forward_list<int, 2>::iterator first = list.insert_after(list.before_begin(), 1);
  SSTL_TEST_ASSERT(first != list.end());
  SSTL_TEST_ASSERT(list.insert_after(first, 2) != list.end());
  int* first_slot = &*first;
  SSTL_TEST_ASSERT(list.erase_after(list.before_begin()) != list.end());
  sstl::forward_list<int, 2>::iterator inserted = list.insert_after(list.before_begin(), 3);
  SSTL_TEST_ASSERT(inserted != list.end());
  SSTL_TEST_EQ(*inserted, 3);
  SSTL_TEST_ASSERT(&*inserted == first_slot);
}

static void forward_list_try_merge_preserves_capacity_contract() {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 3> src;
  sstl::forward_list<int, 2> dst;
  SSTL_TEST_ASSERT(src.insert_after(src.before_begin(), 4) != src.end());
  SSTL_TEST_ASSERT(src.insert_after(src.before_begin(), 2) != src.end());
  SSTL_TEST_ASSERT(dst.insert_after(dst.before_begin(), 1) != dst.end());
  SSTL_TEST_ASSERT(dst.insert_after(dst.begin(), 3) != dst.end());
  SSTL_TEST_ASSERT(!dst.try_merge(src));
  SSTL_TEST_EQ(src.size(), 2u);
  SSTL_TEST_EQ(dst.size(), 2u);
  sstl::forward_list<int, 4> larger;
  SSTL_TEST_ASSERT(larger.insert_after(larger.before_begin(), 1) != larger.end());
  SSTL_TEST_ASSERT(larger.try_merge(src));
  SSTL_TEST_EQ(larger.size(), 3u);
  SSTL_TEST_EQ(src.size(), 0u);
  SSTL_TEST_EQ(*larger.try_front(), 1);
  sstl::forward_list<int, 1> empty;
  SSTL_TEST_ASSERT(empty.try_front() == 0);
}

static void forward_list_front_push_and_pop_follow_return_policy(void) {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 2> list;
  int out = -1;

  SSTL_TEST_ASSERT(!list.try_pop_front(&out));
  SSTL_TEST_EQ(out, -1);
  SSTL_TEST_ASSERT(list.push_front(2));
  SSTL_TEST_ASSERT(list.push_front(1));
  SSTL_TEST_ASSERT(!list.push_front(0));
  SSTL_TEST_EQ(list.front(), 1);
  SSTL_TEST_EQ(*list.try_front(), 1);
  SSTL_TEST_ASSERT(list.try_pop_front(&out));
  SSTL_TEST_EQ(out, 1);
  SSTL_TEST_EQ(list.front(), 2);
  list.pop_front();
  SSTL_TEST_ASSERT(list.empty());
  SSTL_TEST_ASSERT(list.try_front() == 0);
}

static void forward_list_range_splice_after_transfers_half_open_range(void) {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 4> src;
  sstl::forward_list<int, 5> dst;
  sstl::forward_list<int, 4>::iterator src_pos = src.before_begin();
  src_pos = src.insert_after(src_pos, 1);
  src_pos = src.insert_after(src_pos, 2);
  src_pos = src.insert_after(src_pos, 3);
  src_pos = src.insert_after(src_pos, 4);
  SSTL_TEST_ASSERT(src_pos != src.end());
  SSTL_TEST_ASSERT(dst.insert_after(dst.before_begin(), 9) != dst.end());

  sstl::forward_list<int, 4>::iterator before_first = src.begin();
  sstl::forward_list<int, 4>::iterator before_last = src.begin();
  ++before_last;
  ++before_last;
  ++before_last;

  SSTL_TEST_ASSERT(dst.splice_after(dst.before_begin(), src, before_first, before_last));
  SSTL_TEST_EQ(src.size(), 2u);
  SSTL_TEST_EQ(dst.size(), 3u);
  SSTL_TEST_EQ(*src.begin(), 1);
  sstl::forward_list<int, 4>::iterator src_second = src.begin();
  ++src_second;
  SSTL_TEST_EQ(*src_second, 4);
  sstl::forward_list<int, 5>::iterator dst_it = dst.begin();
  SSTL_TEST_EQ(*dst_it, 2);
  ++dst_it;
  SSTL_TEST_EQ(*dst_it, 3);
  ++dst_it;
  SSTL_TEST_EQ(*dst_it, 9);
}

static void forward_list_range_splice_after_same_container_before_last_noops(void) {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 5> list;
  sstl::forward_list<int, 5>::iterator pos = list.before_begin();
  pos = list.insert_after(pos, 1);
  pos = list.insert_after(pos, 2);
  pos = list.insert_after(pos, 3);
  pos = list.insert_after(pos, 4);
  pos = list.insert_after(pos, 5);
  SSTL_TEST_ASSERT(pos != list.end());

  sstl::forward_list<int, 5>::iterator before_first = list.begin();
  sstl::forward_list<int, 5>::iterator before_last = list.begin();
  ++before_last;
  ++before_last;
  ++before_last;

  SSTL_TEST_ASSERT(list.splice_after(before_last, list, before_first, before_last));
  const int expected[] = {1, 2, 3, 4, 5};
  sstl::forward_list<int, 5>::iterator it = list.begin();
  for (unsigned i = 0u; i != 5u; ++i, ++it) {
    SSTL_TEST_ASSERT(it != list.end());
    SSTL_TEST_EQ(*it, expected[i]);
  }
  SSTL_TEST_ASSERT(it == list.end());
}

static void forward_list_range_splice_after_preserves_all_or_nothing_capacity(void) {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 3> src;
  sstl::forward_list<int, 2> dst;
  sstl::forward_list<int, 3>::iterator src_pos = src.before_begin();
  src_pos = src.insert_after(src_pos, 1);
  src_pos = src.insert_after(src_pos, 2);
  src_pos = src.insert_after(src_pos, 3);
  SSTL_TEST_ASSERT(src_pos != src.end());
  SSTL_TEST_ASSERT(dst.insert_after(dst.before_begin(), 9) != dst.end());

  SSTL_TEST_ASSERT(!dst.try_splice_after(dst.before_begin(), src, src.before_begin(), src.end()));
  SSTL_TEST_EQ(src.size(), 3u);
  SSTL_TEST_EQ(dst.size(), 1u);
  SSTL_TEST_EQ(*src.begin(), 1);
  SSTL_TEST_EQ(*dst.begin(), 9);
}

#if SSTL_ON_ERROR == SSTL_RETURN
static void forward_list_erase_after_rejects_unreachable_last(void) {
  sstl_test::noalloc_guard guard;
  sstl::forward_list<int, 3> list;
  sstl::forward_list<int, 3>::iterator pos = list.before_begin();
  pos = list.insert_after(pos, 1);
  pos = list.insert_after(pos, 2);
  pos = list.insert_after(pos, 3);
  SSTL_TEST_ASSERT(pos != list.end());

  sstl::forward_list<int, 3>::iterator first = list.begin();
  ++first;
  sstl::forward_list<int, 3>::iterator returned = list.erase_after(first, list.begin());
  SSTL_TEST_ASSERT(returned == list.end());
  SSTL_TEST_EQ(list.size(), 3u);
  sstl::forward_list<int, 3>::iterator it = list.begin();
  SSTL_TEST_EQ(*it, 1);
  ++it;
  SSTL_TEST_EQ(*it, 2);
  ++it;
  SSTL_TEST_EQ(*it, 3);
}
#endif

int main() {
  const sstl_test::test_case tests[] = {
    {"forward_list_after_operations_and_transfer_are_capacity_checked", forward_list_after_operations_and_transfer_are_capacity_checked},
    {"erase_after_returns_nodes_to_intrusive_free_list", erase_after_returns_nodes_to_intrusive_free_list},
    {"forward_list_try_merge_preserves_capacity_contract", forward_list_try_merge_preserves_capacity_contract},
    {"forward_list_front_push_and_pop_follow_return_policy", forward_list_front_push_and_pop_follow_return_policy},
    {"forward_list_range_splice_after_transfers_half_open_range", forward_list_range_splice_after_transfers_half_open_range},
    {"forward_list_range_splice_after_same_container_before_last_noops", forward_list_range_splice_after_same_container_before_last_noops},
    {"forward_list_range_splice_after_preserves_all_or_nothing_capacity", forward_list_range_splice_after_preserves_all_or_nothing_capacity}
#if SSTL_ON_ERROR == SSTL_RETURN
    , {"forward_list_erase_after_rejects_unreachable_last", forward_list_erase_after_rejects_unreachable_last}
#endif
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
