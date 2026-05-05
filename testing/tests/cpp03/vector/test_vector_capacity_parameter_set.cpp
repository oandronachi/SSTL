// test_id: cpp03.vector.capacity.parameter_set
// api_ids: [cpp.vector.push_back]
// req_ids: [REQ-03, REQ-04, REQ-06]
// patterns: [CAPACITY-BOUNDARY]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <sstl/vector.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

template <sstl::size_t N>
static void fill_exact_capacity() {
  sstl::vector<int, N> v;
  SSTL_TEST_EQ(v.capacity(), static_cast<unsigned>(N));
  for (unsigned i = 0; i != N; ++i) {
    SSTL_TEST_ASSERT(v.push_back(static_cast<int>(i)));
  }
  SSTL_TEST_ASSERT(v.full());
  SSTL_TEST_ASSERT(!v.push_back(999));
  SSTL_TEST_EQ(v.size(), static_cast<unsigned>(N));
}

static void capacity_boundary_required_parameter_set() {
  sstl_test::noalloc_guard guard;
  fill_exact_capacity<0>();
  fill_exact_capacity<1>();
  fill_exact_capacity<2>();
  fill_exact_capacity<16>();
  fill_exact_capacity<1024>();
}

int main() {
  const sstl_test::test_case tests[] = {
    {"capacity_boundary_required_parameter_set", capacity_boundary_required_parameter_set}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
