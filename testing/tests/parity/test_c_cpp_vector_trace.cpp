// test_id: parity.vector.c_cpp_trace
// api_ids: [cpp.vector.parity, c.vector.parity]
// req_ids: [REQ-07]
// patterns: [C-API-PARITY]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This checks C/C++ API parity by running equivalent logical operations through both surfaces and comparing observable state.
 *   - It intentionally avoids implementation internals so macro-generated C types and C++ templates can evolve independently.
 *   - A mismatch here usually means either C macro semantics drifted from C++ or the test is assuming a non-shared convenience API.
 */
extern "C" {
#include <sstl/c/sstl_vector.h>
}
#include <sstl/vector.hpp>
#include "test_harness.hpp"

SSTL_VECTOR_DECLARE(cpp_parity_vec, int, 4)
SSTL_VECTOR_DEFINE(cpp_parity_vec, int, 4)

static void c_and_cpp_vector_trace_match() {
  cpp_parity_vec cv;
  sstl::vector<int, 4> vv;
  cpp_parity_vec_init(&cv);
  for (int i = 0; i != 4; ++i) {
    SSTL_TEST_ASSERT(cpp_parity_vec_push_back(&cv, i + 1));
    SSTL_TEST_ASSERT(vv.push_back(i + 1));
  }
  SSTL_TEST_ASSERT(!cpp_parity_vec_push_back(&cv, 99));
  SSTL_TEST_ASSERT(!vv.push_back(99));
  SSTL_TEST_EQ(cpp_parity_vec_size(&cv), vv.size());
  for (unsigned i = 0; i != vv.size(); ++i) {
    SSTL_TEST_EQ(*cpp_parity_vec_at(&cv, i), vv[i]);
  }
}

int main() {
  const sstl_test::test_case tests[] = {
    {"c_and_cpp_vector_trace_match", c_and_cpp_vector_trace_match}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
