// test_id: cpp03.vector.resize
// api_ids: [cpp.vector.resize]
// req_ids: [REQ-03, REQ-05, REQ-06]
// patterns: [CAPACITY-BOUNDARY, AUDIT-NOALLOC]
/*
 * Notes:
 *   - resize is capacity-routed: growth beyond the fixed bound must fail without changing live elements.
 *   - Growing value-initializes new slots; shrinking removes tail elements while preserving earlier values.
 *   - The zero-capacity case matters because it catches accidental hidden storage growth.
 */
#include <sstl/vector.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void resize_grows_shrinks_and_preserves_capacity() {
  sstl_test::noalloc_guard guard;
  sstl::vector<int, 3> v;
  SSTL_TEST_ASSERT(v.resize(2));
  SSTL_TEST_EQ(v.size(), 2u);
  SSTL_TEST_EQ(v[0], 0);
  v[0] = 7;
  v[1] = 8;
  SSTL_TEST_ASSERT(v.resize(1));
  SSTL_TEST_EQ(v.size(), 1u);
  SSTL_TEST_EQ(v[0], 7);
  SSTL_TEST_ASSERT(v.resize(3));
  SSTL_TEST_EQ(v[0], 7);
  SSTL_TEST_EQ(v[1], 0);
  SSTL_TEST_EQ(v[2], 0);
  SSTL_TEST_ASSERT(!v.resize(4));
  SSTL_TEST_EQ(v.size(), 3u);
}

static void zero_capacity_resize_only_accepts_zero() {
  sstl_test::noalloc_guard guard;
  sstl::vector<int, 0> v;
  SSTL_TEST_ASSERT(v.resize(0));
  SSTL_TEST_ASSERT(!v.resize(1));
  SSTL_TEST_ASSERT(v.empty());
}

int main() {
  const sstl_test::test_case tests[] = {
    {"resize_grows_shrinks_and_preserves_capacity", resize_grows_shrinks_and_preserves_capacity},
    {"zero_capacity_resize_only_accepts_zero", zero_capacity_resize_only_accepts_zero}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
