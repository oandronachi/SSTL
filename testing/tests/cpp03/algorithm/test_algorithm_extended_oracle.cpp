// test_id: cpp03.algorithm.oracle.extended
// api_ids: [cpp.algorithm.find_if, cpp.algorithm.all_of, cpp.algorithm.any_of, cpp.algorithm.none_of, cpp.algorithm.equal, cpp.algorithm.mismatch, cpp.algorithm.search, cpp.algorithm.lexicographical_compare, cpp.algorithm.min_element, cpp.algorithm.max_element, cpp.algorithm.copy_backward, cpp.algorithm.fill, cpp.algorithm.transform, cpp.algorithm.replace, cpp.algorithm.remove, cpp.algorithm.rotate, cpp.algorithm.unique, cpp.algorithm.heap, cpp.algorithm.partition, cpp.algorithm.set_intersection, cpp.algorithm.equal_range, cpp.algorithm.inner_product, cpp.algorithm.partial_sum, cpp.algorithm.adjacent_difference, cpp.algorithm.iota]
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
#include <numeric>
#include <sstl/algorithm.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static bool is_even(int x) { return (x % 2) == 0; }
static int times_two(int x) { return x * 2; }
static bool less_than_four(int x) { return x < 4; }

static void non_modifying_algorithms_match_std() {
  sstl_test::noalloc_guard guard;
  int a[] = {1, 2, 3, 4, 5};
  int b[] = {1, 2, 9, 4, 5};
  int needle[] = {3, 4};
  SSTL_TEST_EQ(sstl::find_if(a, a + 5, is_even) - a, std::find_if(a, a + 5, is_even) - a);
  SSTL_TEST_ASSERT(!sstl::all_of(a, a + 5, less_than_four));
  SSTL_TEST_ASSERT(sstl::any_of(a, a + 5, is_even));
  SSTL_TEST_ASSERT(!sstl::none_of(a, a + 5, is_even));
  SSTL_TEST_EQ(sstl::equal(a, a + 2, b), std::equal(a, a + 2, b));
  SSTL_TEST_EQ(sstl::mismatch(a, a + 5, b).first - a, std::mismatch(a, a + 5, b).first - a);
  SSTL_TEST_EQ(sstl::search(a, a + 5, needle, needle + 2) - a, std::search(a, a + 5, needle, needle + 2) - a);
  SSTL_TEST_EQ(sstl::lexicographical_compare(a, a + 5, b, b + 5), std::lexicographical_compare(a, a + 5, b, b + 5));
  SSTL_TEST_EQ(*sstl::min_element(b, b + 5), *std::min_element(b, b + 5));
  SSTL_TEST_EQ(*sstl::max_element(b, b + 5), *std::max_element(b, b + 5));
}

static void modifying_algorithms_match_std() {
  sstl_test::noalloc_guard guard;
  int s1[] = {1, 2, 3, 4, 5};
  int s2[] = {1, 2, 3, 4, 5};
  int out1[5] = {0, 0, 0, 0, 0};
  int out2[5] = {0, 0, 0, 0, 0};
  sstl::copy_backward(s1, s1 + 5, out1 + 5);
  std::copy_backward(s2, s2 + 5, out2 + 5);
  for (int i = 0; i != 5; ++i) SSTL_TEST_EQ(out1[i], out2[i]);
  sstl::fill(out1, out1 + 5, 3);
  std::fill(out2, out2 + 5, 3);
  sstl::transform(out1, out1 + 5, out1, times_two);
  std::transform(out2, out2 + 5, out2, times_two);
  sstl::replace(out1, out1 + 5, 6, 2);
  std::replace(out2, out2 + 5, 6, 2);
  for (int j = 0; j != 5; ++j) SSTL_TEST_EQ(out1[j], out2[j]);

  int r1[] = {1, 2, 2, 3, 4};
  int r2[] = {1, 2, 2, 3, 4};
  SSTL_TEST_EQ(sstl::remove(r1, r1 + 5, 2) - r1, std::remove(r2, r2 + 5, 2) - r2);
  sstl::rotate(r1, r1 + 1, r1 + 5);
  std::rotate(r2, r2 + 1, r2 + 5);
  SSTL_TEST_EQ(sstl::unique(r1, r1 + 5) - r1, std::unique(r2, r2 + 5) - r2);
}

static void ordering_numeric_and_set_algorithms_match_std() {
  sstl_test::noalloc_guard guard;
  int h1[] = {4, 1, 3, 2};
  int h2[] = {4, 1, 3, 2};
  sstl::make_heap(h1, h1 + 4);
  std::make_heap(h2, h2 + 4);
  SSTL_TEST_EQ(h1[0], h2[0]);
  sstl::pop_heap(h1, h1 + 4);
  std::pop_heap(h2, h2 + 4);
  SSTL_TEST_EQ(h1[3], h2[3]);

  int p1[] = {1, 4, 2, 5, 3};
  int p2[] = {1, 4, 2, 5, 3};
  SSTL_TEST_EQ(sstl::partition(p1, p1 + 5, less_than_four) - p1, std::partition(p2, p2 + 5, less_than_four) - p2);

  int a[] = {1, 2, 3, 5};
  int b[] = {2, 3, 4};
  int o1[8] = {0};
  int o2[8] = {0};
  SSTL_TEST_EQ(sstl::set_intersection(a, a + 4, b, b + 3, o1) - o1,
               std::set_intersection(a, a + 4, b, b + 3, o2) - o2);
  SSTL_TEST_EQ(sstl::equal_range(a, a + 4, 3).first - a, std::equal_range(a, a + 4, 3).first - a);

  int n1[5] = {0, 0, 0, 0, 0};
  int n2[5] = {0, 0, 0, 0, 0};
  sstl::iota(n1, n1 + 5, 1);
  for (int k = 0; k != 5; ++k) n2[k] = k + 1;
  SSTL_TEST_EQ(sstl::inner_product(n1, n1 + 5, n2, 0), std::inner_product(n1, n1 + 5, n2, 0));
  sstl::partial_sum(n1, n1 + 5, o1);
  std::partial_sum(n2, n2 + 5, o2);
  for (int i = 0; i != 5; ++i) SSTL_TEST_EQ(o1[i], o2[i]);
  sstl::adjacent_difference(n1, n1 + 5, o1);
  std::adjacent_difference(n2, n2 + 5, o2);
  for (int j = 0; j != 5; ++j) SSTL_TEST_EQ(o1[j], o2[j]);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"non_modifying_algorithms_match_std", non_modifying_algorithms_match_std},
    {"modifying_algorithms_match_std", modifying_algorithms_match_std},
    {"ordering_numeric_and_set_algorithms_match_std", ordering_numeric_and_set_algorithms_match_std}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
