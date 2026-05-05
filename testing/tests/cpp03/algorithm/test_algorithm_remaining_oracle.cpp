// test_id: cpp03.algorithm.oracle.remaining
// api_ids: [cpp.algorithm.find_if_not, cpp.algorithm.count_if, cpp.algorithm.copy_n, cpp.algorithm.copy_if, cpp.algorithm.replace_if, cpp.algorithm.remove_if, cpp.algorithm.fill_n, cpp.algorithm.swap_ranges, cpp.algorithm.iter_swap, cpp.algorithm.stable_sort, cpp.algorithm.partial_sort, cpp.algorithm.nth_element, cpp.algorithm.is_sorted, cpp.algorithm.is_sorted_until, cpp.algorithm.push_heap, cpp.algorithm.sort_heap, cpp.algorithm.stable_partition, cpp.algorithm.merge, cpp.algorithm.includes, cpp.algorithm.set_difference, cpp.algorithm.set_symmetric_difference, cpp.algorithm.upper_bound, cpp.algorithm.minmax_element, cpp.algorithm.comparator_overloads]
// req_ids: [REQ-02, REQ-05]
// patterns: [STL-ORACLE-EQUIV, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It compares SSTL algorithms with a standard-library or explicit mathematical oracle while keeping the translation unit C++03-compatible.
 *   - When the C++03 standard library lacks a newer oracle, the expected result is stated directly so future readers can audit the law being checked.
 *   - Keep api_ids in the file header and manifests/test_manifest.yaml synchronized when adding or removing algorithms.
 */
#include <algorithm>
#include <sstl/algorithm.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static bool is_even_r(int x) { return (x % 2) == 0; }
static bool less_than_five_r(int x) { return x < 5; }
static bool same_last_digit_r(int a, int b) { return (a % 10) == (b % 10); }

struct stable_item {
  int key;
  int order;
};

static bool stable_item_less(const stable_item& a, const stable_item& b) {
  return a.key < b.key;
}

static void remaining_non_modifying_and_copy_algorithms() {
  sstl_test::noalloc_guard guard;
  int a[] = {2, 4, 5, 6, 7};
  SSTL_TEST_EQ(*sstl::find_if_not(a, a + 5, is_even_r), 5);
  SSTL_TEST_EQ(sstl::count_if(a, a + 5, is_even_r), 3);

  int cn[3] = {0, 0, 0};
  sstl::copy_n(a, 3, cn);
  SSTL_TEST_EQ(cn[0], 2);
  SSTL_TEST_EQ(cn[2], 5);

  int cf[5] = {0, 0, 0, 0, 0};
  int* cf_end = sstl::copy_if(a, a + 5, cf, is_even_r);
  SSTL_TEST_EQ(cf_end - cf, 3);
  SSTL_TEST_EQ(cf[0], 2);
  SSTL_TEST_EQ(cf[2], 6);

  SSTL_TEST_ASSERT(sstl::is_sorted(a, a + 2));
  SSTL_TEST_EQ(sstl::is_sorted_until(a, a + 5), a + 5);
  SSTL_TEST_EQ(*sstl::upper_bound(a, a + 5, 5), 6);
  SSTL_TEST_EQ(sstl::minmax_element(a, a + 5).first, a);
  SSTL_TEST_EQ(*sstl::minmax_element(a, a + 5).second, 7);
}

static void remaining_modifying_algorithms() {
  sstl_test::noalloc_guard guard;
  int a[] = {1, 2, 3, 4, 5};
  sstl::replace_if(a, a + 5, is_even_r, 8);
  SSTL_TEST_EQ(a[1], 8);
  SSTL_TEST_EQ(a[3], 8);
  SSTL_TEST_EQ(sstl::remove_if(a, a + 5, is_even_r) - a, 3);

  int b[4] = {0, 0, 0, 0};
  sstl::fill_n(b, 4, 6);
  SSTL_TEST_EQ(b[0], 6);
  SSTL_TEST_EQ(b[3], 6);

  int left[] = {1, 2, 3};
  int right[] = {7, 8, 9};
  sstl::swap_ranges(left, left + 3, right);
  SSTL_TEST_EQ(left[0], 7);
  SSTL_TEST_EQ(right[2], 3);
  sstl::iter_swap(left, left + 2);
  SSTL_TEST_EQ(left[0], 9);
  SSTL_TEST_EQ(left[2], 7);

  int part[] = {6, 1, 7, 2, 8, 3};
  int* split = sstl::stable_partition(part, part + 6, less_than_five_r);
  SSTL_TEST_EQ(split - part, 3);
  const int expected_part[] = {1, 2, 3, 6, 7, 8};
  for (int i = 0; i != 6; ++i) {
    SSTL_TEST_EQ(part[i], expected_part[i]);
  }
}

static void remaining_sort_heap_and_set_algorithms() {
  sstl_test::noalloc_guard guard;
  stable_item items[] = {{2, 0}, {1, 1}, {2, 2}, {1, 3}};
  sstl::stable_sort(items, items + 4, stable_item_less);
  SSTL_TEST_EQ(items[0].order, 1);
  SSTL_TEST_EQ(items[1].order, 3);
  SSTL_TEST_EQ(items[2].order, 0);
  SSTL_TEST_EQ(items[3].order, 2);

  int ps[] = {9, 1, 8, 2, 7, 3};
  sstl::partial_sort(ps, ps + 3, ps + 6);
  SSTL_TEST_EQ(ps[0], 1);
  SSTL_TEST_EQ(ps[1], 2);
  SSTL_TEST_EQ(ps[2], 3);
  int nth[] = {9, 1, 8, 2, 7, 3};
  sstl::nth_element(nth, nth + 2, nth + 6);
  SSTL_TEST_EQ(nth[2], 3);

  int heap[5] = {4, 1, 3, 2, 5};
  sstl::make_heap(heap, heap + 4);
  sstl::push_heap(heap, heap + 5);
  SSTL_TEST_EQ(heap[0], 5);
  sstl::sort_heap(heap, heap + 5);
  SSTL_TEST_ASSERT(sstl::is_sorted(heap, heap + 5));

  int a[] = {1, 2, 4, 6};
  int b[] = {2, 3, 4, 5};
  int out[8] = {0};
  SSTL_TEST_EQ(sstl::merge(a, a + 4, b, b + 4, out) - out, 8);
  SSTL_TEST_ASSERT(sstl::includes(out, out + 8, a, a + 4));
  SSTL_TEST_EQ(sstl::set_difference(a, a + 4, b, b + 4, out) - out, 2);
  SSTL_TEST_EQ(out[0], 1);
  SSTL_TEST_EQ(out[1], 6);
  SSTL_TEST_EQ(sstl::set_symmetric_difference(a, a + 4, b, b + 4, out) - out, 4);
}

static void comparator_overloads_match_descending_oracles() {
  sstl_test::noalloc_guard guard;
  sstl::greater<int> desc;

  /*
   * The range is sorted according to the supplied comparator, not operator<.
   * These checks make sure the binary-search family accepts that alternate
   * ordering everywhere the STL exposes a Compare-taking overload.
   */
  int bounds[] = {9, 7, 7, 5, 3};
  SSTL_TEST_ASSERT(sstl::is_sorted(bounds, bounds + 5, desc));
  SSTL_TEST_EQ(sstl::is_sorted_until(bounds, bounds + 5, desc), bounds + 5);
  SSTL_TEST_EQ(sstl::lower_bound(bounds, bounds + 5, 7, desc) - bounds, 1);
  SSTL_TEST_EQ(sstl::upper_bound(bounds, bounds + 5, 7, desc) - bounds, 3);
  SSTL_TEST_EQ(sstl::equal_range(bounds, bounds + 5, 7, desc).first - bounds, 1);
  SSTL_TEST_EQ(sstl::equal_range(bounds, bounds + 5, 7, desc).second - bounds, 3);
  SSTL_TEST_ASSERT(sstl::binary_search(bounds, bounds + 5, 5, desc));
  SSTL_TEST_ASSERT(!sstl::binary_search(bounds, bounds + 5, 6, desc));

  int broken[] = {9, 7, 8, 5};
  SSTL_TEST_EQ(sstl::is_sorted_until(broken, broken + 4, desc) - broken, 2);

  int left[] = {8, 6, 4, 2};
  int right[] = {7, 6, 3, 2};
  int out[8] = {0};
  const int expected_merge[] = {8, 7, 6, 6, 4, 3, 2, 2};
  SSTL_TEST_EQ(sstl::merge(left, left + 4, right, right + 4, out, desc) - out, 8);
  for (int i = 0; i != 8; ++i) SSTL_TEST_EQ(out[i], expected_merge[i]);

  int includes_a[] = {9, 7, 7, 5, 3};
  int includes_b[] = {7, 7, 3};
  int includes_c[] = {7, 7, 7};
  SSTL_TEST_ASSERT(sstl::includes(includes_a, includes_a + 5, includes_b, includes_b + 3, desc));
  SSTL_TEST_ASSERT(!sstl::includes(includes_a, includes_a + 5, includes_c, includes_c + 3, desc));

  const int expected_union[] = {8, 7, 6, 4, 3, 2};
  SSTL_TEST_EQ(sstl::set_union(left, left + 4, right, right + 4, out, desc) - out, 6);
  for (int u = 0; u != 6; ++u) SSTL_TEST_EQ(out[u], expected_union[u]);

  const int expected_intersection[] = {6, 2};
  SSTL_TEST_EQ(sstl::set_intersection(left, left + 4, right, right + 4, out, desc) - out, 2);
  for (int n = 0; n != 2; ++n) SSTL_TEST_EQ(out[n], expected_intersection[n]);

  const int expected_difference[] = {8, 4};
  SSTL_TEST_EQ(sstl::set_difference(left, left + 4, right, right + 4, out, desc) - out, 2);
  for (int d = 0; d != 2; ++d) SSTL_TEST_EQ(out[d], expected_difference[d]);

  const int expected_symmetric_difference[] = {8, 7, 4, 3};
  SSTL_TEST_EQ(sstl::set_symmetric_difference(left, left + 4, right, right + 4, out, desc) - out, 4);
  for (int s = 0; s != 4; ++s) SSTL_TEST_EQ(out[s], expected_symmetric_difference[s]);

  int almost_duplicates[] = {11, 21, 32, 42, 53};
  int* unique_end = sstl::unique(almost_duplicates, almost_duplicates + 5, same_last_digit_r);
  SSTL_TEST_EQ(unique_end - almost_duplicates, 3);
  SSTL_TEST_EQ(almost_duplicates[0], 11);
  SSTL_TEST_EQ(almost_duplicates[1], 32);
  SSTL_TEST_EQ(almost_duplicates[2], 53);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"remaining_non_modifying_and_copy_algorithms", remaining_non_modifying_and_copy_algorithms},
    {"remaining_modifying_algorithms", remaining_modifying_algorithms},
    {"remaining_sort_heap_and_set_algorithms", remaining_sort_heap_and_set_algorithms},
    {"comparator_overloads_match_descending_oracles", comparator_overloads_match_descending_oracles}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
