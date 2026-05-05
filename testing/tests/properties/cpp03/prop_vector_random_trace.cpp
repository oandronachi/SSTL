// test_id: property.cpp03.vector.random_trace.seeded
// api_ids: [cpp.vector.erase, cpp.vector.push_back]
// req_ids: [REQ-03, REQ-05, REQ-06]
// patterns: [STL-ORACLE-EQUIV]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This is a deterministic property-style trace: a fixed PRNG seed gives broad operation coverage while remaining reproducible after a failure.
 *   - The std::vector side is the oracle; SSTL is capacity-filtered so documented full-container failures are compared fairly.
 *   - If a failure appears, keep the seed and shorten the trace before debugging implementation details.
 */
#include <vector>
#include <sstl/vector.hpp>
#include "test_harness.hpp"
#include "prng.hpp"

static void seeded_vector_trace_matches_std_with_capacity_filter() {
  sstl_test::prng rng(0x5353544cu);
  std::vector<int> ref;
  sstl::vector<int, 16> sut;
  for (int step = 0; step != 1000; ++step) {
    const int op = rng.bounded(3);
    if (op == 0) {
      const int value = rng.bounded(257) - 128;
      const bool can_push = ref.size() < 16u;
      const bool pushed = sut.push_back(value);
      SSTL_TEST_EQ(pushed, can_push);
      if (can_push) ref.push_back(value);
    } else if (op == 1 && !ref.empty()) {
      ref.pop_back();
      sut.pop_back();
    } else if (op == 2 && !ref.empty()) {
      const unsigned idx = static_cast<unsigned>(rng.bounded(static_cast<int>(ref.size())));
      ref.erase(ref.begin() + idx);
      sut.erase(sut.begin() + idx);
    }
    SSTL_TEST_EQ(ref.size(), sut.size());
    for (std::size_t i = 0; i != ref.size(); ++i) {
      SSTL_TEST_EQ(ref[i], sut[i]);
    }
  }
}

int main() {
  const sstl_test::test_case tests[] = {
    {"seeded_vector_trace_matches_std_with_capacity_filter", seeded_vector_trace_matches_std_with_capacity_filter}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
