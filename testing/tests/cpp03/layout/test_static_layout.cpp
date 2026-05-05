// test_id: cpp03.layout.static_storage_contract
// api_ids: [cpp.vector.push_back, cpp.string.push_back, cpp.deque.contract]
// req_ids: [REQ-02, REQ-03, REQ-06]
// patterns: [STATIC-LAYOUT]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It checks operational layout promises from the spec: inline storage, sane size/alignment, and no hidden heap handle observable through public C layout.
 *   - The checks stay within C99/C++03 limits and avoid pretending C++03 has standard_layout traits.
 *   - Treat these as portability tripwires; exact sizeof values may vary by padding and alignment.
 */
#include <cstddef>
#include <sstl/config.hpp>
#include <sstl/vector.hpp>
#include <sstl/string.hpp>
#include <sstl/deque.hpp>
#include "test_harness.hpp"

struct align16_payload {
  char data[16];
};

static void owning_containers_have_inline_storage_bounds() {
  typedef sstl::vector<int, 16> vec16;
  typedef sstl::string<16> str16;
  typedef sstl::deque<int, 16> deq16;
  typedef sstl::vector<align16_payload, 2> aligned_vec2;
  typedef sstl::deque<align16_payload, 2> aligned_deq2;
  SSTL_TEST_ASSERT(sizeof(vec16) >= sizeof(int) * 16u);
  SSTL_TEST_ASSERT(sizeof(str16) >= 17u);
  SSTL_TEST_ASSERT(sizeof(deq16) >= sizeof(int) * 16u);
  SSTL_TEST_ASSERT(sizeof(aligned_vec2) >= sizeof(align16_payload) * 2u);
#ifdef SSTL_ALIGNOF
  SSTL_TEST_ASSERT(SSTL_ALIGNOF(aligned_vec2) >= SSTL_ALIGNOF(align16_payload));
  SSTL_TEST_ASSERT(SSTL_ALIGNOF(aligned_deq2) >= SSTL_ALIGNOF(align16_payload));
#endif
}

int main() {
  const sstl_test::test_case tests[] = {
    {"owning_containers_have_inline_storage_bounds", owning_containers_have_inline_storage_bounds}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
