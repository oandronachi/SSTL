// test_id: cpp03.vector.oracle.deterministic_trace
// api_ids: [cpp.vector.push_back, cpp.vector.erase, cpp.vector.insert]
// req_ids: [REQ-05, REQ-06]
// patterns: [STL-ORACLE-EQUIV]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is a direct SSTL contract test; prefer changing the manifest and comments together when behavior changes.
 *   - The assertions are black-box: they verify public observable state rather than private representation.
 *   - If this test fails against an implementation, first compare the implementation API to the SSTL public contract.
 */
#include <vector>
#include <sstl/vector.hpp>
#include "test_harness.hpp"

static void deterministic_trace_matches_std_vector() {
  std::vector<int> ref;
  sstl::vector<int, 8> sut;
  for (int i = 0; i != 6; ++i) {
    ref.push_back(i * 3);
    SSTL_TEST_ASSERT(sut.push_back(i * 3));
  }
  ref.insert(ref.begin() + 2, 99);
  SSTL_TEST_ASSERT(sut.insert(sut.begin() + 2, 99) != sut.end());
  ref.erase(ref.begin() + 4);
  sut.erase(sut.begin() + 4);
  SSTL_TEST_EQ(ref.size(), sut.size());
  for (std::size_t i = 0; i != ref.size(); ++i) {
    SSTL_TEST_EQ(ref[i], sut[i]);
  }
}

int main() {
  const sstl_test::test_case tests[] = {
    {"deterministic_trace_matches_std_vector", deterministic_trace_matches_std_vector}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
