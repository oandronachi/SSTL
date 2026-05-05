// test_id: cpp03.owning.zero_capacity
// api_ids: [cpp.vector.push_back, cpp.string.push_back]
// req_ids: [REQ-01, REQ-03, REQ-04, REQ-06]
// patterns: [CAPACITY-BOUNDARY, STATIC-LAYOUT]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <sstl/array.hpp>
#include <sstl/vector.hpp>
#include <sstl/string.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"
#include "tracked.hpp"

struct no_default_zero_probe {
  static int live;
  explicit no_default_zero_probe(int value) : value_(value) { ++live; }
  no_default_zero_probe(const no_default_zero_probe& other) : value_(other.value_) { ++live; }
  ~no_default_zero_probe() { --live; }
  int value_;
private:
  no_default_zero_probe();
};

int no_default_zero_probe::live = 0;

static void zero_capacity_vector_has_no_live_elements() {
  sstl_test::tracked::reset();
  sstl_test::noalloc_guard guard;
  sstl::vector<sstl_test::tracked, 0> v;
  sstl_test::tracked x(7);
  SSTL_TEST_EQ(v.capacity(), 0u);
  SSTL_TEST_ASSERT(v.empty());
  SSTL_TEST_ASSERT(v.full());
  SSTL_TEST_ASSERT(v.try_front() == 0);
  SSTL_TEST_ASSERT(v.try_back() == 0);
  SSTL_TEST_ASSERT(!v.push_back(x));
  SSTL_TEST_EQ(v.size(), 0u);
  SSTL_TEST_EQ(sstl_test::tracked::live, 1);
}

static void zero_capacity_string_is_empty_and_full() {
  sstl_test::noalloc_guard guard;
  sstl::string<0> s;
  SSTL_TEST_EQ(s.capacity(), 0u);
  SSTL_TEST_ASSERT(s.empty());
  SSTL_TEST_ASSERT(s.full());
  SSTL_TEST_ASSERT(s.try_front() == 0);
  SSTL_TEST_ASSERT(s.try_back() == 0);
  SSTL_TEST_ASSERT(!s.push_back('x'));
  SSTL_TEST_EQ(s.size(), 0u);
  SSTL_TEST_EQ(s.c_str()[0], '\0');
}

static void zero_capacity_array_constructs_no_elements() {
  sstl_test::noalloc_guard guard;
  no_default_zero_probe::live = 0;
  {
    sstl::array<no_default_zero_probe, 0> a;
    SSTL_TEST_EQ(a.size(), 0u);
    SSTL_TEST_ASSERT(a.empty());
    SSTL_TEST_ASSERT(a.try_front() == 0);
    SSTL_TEST_ASSERT(a.try_back() == 0);
    SSTL_TEST_EQ(no_default_zero_probe::live, 0);
  }
  SSTL_TEST_EQ(no_default_zero_probe::live, 0);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"zero_capacity_vector_has_no_live_elements", zero_capacity_vector_has_no_live_elements},
    {"zero_capacity_string_is_empty_and_full", zero_capacity_string_is_empty_and_full},
    {"zero_capacity_array_constructs_no_elements", zero_capacity_array_constructs_no_elements}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
