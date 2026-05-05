// test_id: cpp03.algorithm.oracle.basic
// api_ids: [cpp.algorithm.accumulate, cpp.algorithm.copy, cpp.algorithm.count, cpp.algorithm.reverse, cpp.algorithm.set_union, cpp.algorithm.sort]
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

static void algorithms_match_std_for_contiguous_ranges() {
  sstl_test::noalloc_guard guard;
  int a[] = {7, 2, 9, 1, 2};
  int b[] = {7, 2, 9, 1, 2};
  std::sort(a, a + 5);
  sstl::sort(b, b + 5);
  for (int i = 0; i != 5; ++i) {
    SSTL_TEST_EQ(a[i], b[i]);
  }
  SSTL_TEST_ASSERT(sstl::binary_search(b, b + 5, 7));
  SSTL_TEST_ASSERT(sstl::find(b, b + 5, 9) != b + 5);
  SSTL_TEST_EQ(sstl::count(b, b + 5, 2), 2);

  int copied[5] = {0, 0, 0, 0, 0};
  sstl::copy(b, b + 5, copied);
  sstl::reverse(copied, copied + 5);
  std::reverse(a, a + 5);
  for (int j = 0; j != 5; ++j) {
    SSTL_TEST_EQ(a[j], copied[j]);
  }

  int lhs[] = {1, 3, 5};
  int rhs[] = {2, 3, 4};
  int merged[6] = {0, 0, 0, 0, 0, 0};
  int* end = sstl::set_union(lhs, lhs + 3, rhs, rhs + 3, merged);
  SSTL_TEST_EQ(end - merged, 5);
  SSTL_TEST_EQ(*sstl::lower_bound(merged, end, 4), 4);
  SSTL_TEST_EQ(sstl::accumulate(merged, end, 0), 15);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"algorithms_match_std_for_contiguous_ranges", algorithms_match_std_for_contiguous_ranges}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
