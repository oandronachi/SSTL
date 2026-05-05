// test_id: cpp03.compile.mixed_c_cpp_panic_hook
// api_ids: [cpp.public_headers, c.public_headers, cpp.policy.panic, c.policy.panic]
// req_ids: [REQ-01, REQ-04, REQ-07, REQ-10]
// patterns: [STL-COMPILE-PARITY, C-API-PARITY, POLICY-MATRIX]
/*
 * Notes:
 *   - This probe intentionally includes the C headers first, inside an
 *     extern "C" block, then includes C++ headers in the same translation unit.
 *   - The important PANIC-policy behavior is compile/link compatibility of the
 *     shared default sstl_panic hook. The test performs only valid operations,
 *     so it never opens an abort dialog or relies on a process-level failure.
 *   - RETURN and UB policy builds still compile this file; they prove the same
 *     mixed-header include order remains harmless when the hook is not active.
 */
extern "C" {
#include <sstl/c/sstl_vector.h>
}

#include <sstl/vector.hpp>

#include "test_harness.hpp"

SSTL_VECTOR_DECLARE(mixed_hook_c_vec, int, 2)
SSTL_VECTOR_DEFINE(mixed_hook_c_vec, int, 2)

static void mixed_c_first_cpp_second_headers_share_the_default_hook_guard() {
  mixed_hook_c_vec cvec;
  mixed_hook_c_vec_init(&cvec);
  SSTL_TEST_ASSERT(mixed_hook_c_vec_push_back(&cvec, 1));
  SSTL_TEST_ASSERT(mixed_hook_c_vec_push_back(&cvec, 2));
  SSTL_TEST_EQ(*mixed_hook_c_vec_at(&cvec, 0u), 1);

  sstl::vector<int, 2> cppvec;
  SSTL_TEST_ASSERT(cppvec.push_back(3));
  SSTL_TEST_ASSERT(cppvec.push_back(4));
  SSTL_TEST_EQ(cppvec[1], 4);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"mixed_c_first_cpp_second_headers_share_the_default_hook_guard", mixed_c_first_cpp_second_headers_share_the_default_hook_guard}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
