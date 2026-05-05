// test_id: cpp03.family.policy_capacity
// api_ids: [cpp.string.policy_capacity, cpp.deque.policy_capacity, cpp.list.policy_capacity, cpp.forward_list.policy_capacity, cpp.map.policy_capacity, cpp.set.policy_capacity, cpp.flat_map.policy_capacity, cpp.unordered_map.policy_capacity]
// req_ids: [REQ-03, REQ-04, REQ-05, REQ-06]
// patterns: [POLICY-MATRIX, CAPACITY-BOUNDARY]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is policy-aware: RETURN checks sentinel/status results, PANIC checks the configured panic hook, and UB validates only legal operations because contract-violating calls may omit recovery checks.
 *   - The same source is compiled repeatedly with different SSTL_ON_ERROR values via local CMake presets.
 *   - The setjmp/longjmp panic path is a test harness device; production panic behavior only needs to call sstl_panic as specified.
 */
#define SSTL_NO_DEFAULT_PANIC 1
#include <setjmp.h>
#include <sstl/string.hpp>
#include <sstl/deque.hpp>
#include <sstl/list.hpp>
#include <sstl/forward_list.hpp>
#include <sstl/map.hpp>
#include <sstl/set.hpp>
#include <sstl/flat_map.hpp>
#include <sstl/unordered_map.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static jmp_buf g_family_panic_env;
static int g_family_panic_count = 0;

extern "C" void sstl_panic(const char*) {
  ++g_family_panic_count;
  longjmp(g_family_panic_env, 1);
}

template <sstl::size_t N>
static void check_string_capacity() {
  sstl::string<N> s;
  SSTL_TEST_EQ(s.capacity(), static_cast<unsigned>(N));
  for (unsigned i = 0; i != N; ++i) {
    SSTL_TEST_ASSERT(s.push_back(static_cast<char>('a' + (i % 26u))));
  }
  SSTL_TEST_ASSERT(s.full());
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(!s.push_back('z'));
#endif
  SSTL_TEST_EQ(s.size(), static_cast<unsigned>(N));
}

template <sstl::size_t N>
static void check_deque_capacity() {
  sstl::deque<int, N> d;
  SSTL_TEST_EQ(d.capacity(), static_cast<unsigned>(N));
  for (unsigned i = 0; i != N; ++i) {
    SSTL_TEST_ASSERT(d.push_back(static_cast<int>(i)));
  }
  SSTL_TEST_ASSERT(d.full());
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(!d.push_back(999));
#endif
  SSTL_TEST_EQ(d.size(), static_cast<unsigned>(N));
}

template <sstl::size_t N>
static void check_list_capacity() {
  sstl::list<int, N> l;
  for (unsigned i = 0; i != N; ++i) {
    SSTL_TEST_ASSERT(l.push_back(static_cast<int>(i)));
  }
  SSTL_TEST_ASSERT(l.full());
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(!l.push_back(999));
#endif
  SSTL_TEST_EQ(l.size(), static_cast<unsigned>(N));
}

static void sequence_family_capacity_boundaries() {
  sstl_test::noalloc_guard guard;
  check_string_capacity<0>();
  check_string_capacity<1>();
  check_string_capacity<2>();
  check_string_capacity<16>();
  check_string_capacity<1024>();
  check_deque_capacity<0>();
  check_deque_capacity<1>();
  check_deque_capacity<2>();
  check_deque_capacity<16>();
  check_deque_capacity<1024>();
  check_list_capacity<0>();
  check_list_capacity<1>();
  check_list_capacity<2>();
  check_list_capacity<16>();
}

static void associative_family_capacity_boundaries() {
  sstl_test::noalloc_guard guard;
  sstl::map<int, int, 1> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 1)).second);
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(!m.insert(sstl::make_pair(2, 2)).second);
#endif

  sstl::set<int, 1> s;
  SSTL_TEST_ASSERT(s.insert(1).second);
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(!s.insert(2).second);
#endif

  sstl::flat_map<int, int, 1> fm;
  SSTL_TEST_ASSERT(fm.insert(sstl::make_pair(1, 1)).second);
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(!fm.insert(sstl::make_pair(2, 2)).second);
#endif

  sstl::unordered_map<int, int, 1> um;
  SSTL_TEST_ASSERT(um.insert(sstl::make_pair(1, 1)).second);
#if SSTL_ON_ERROR == SSTL_RETURN
  SSTL_TEST_ASSERT(!um.insert(sstl::make_pair(2, 2)).second);
#endif
}

static void policy_family_invalid_access_matches_mode() {
#if SSTL_ON_ERROR == SSTL_RETURN
  sstl::string<1> s;
  sstl::deque<int, 1> d;
  SSTL_TEST_ASSERT(s.try_at(0) == 0);
  SSTL_TEST_ASSERT(!d.try_pop_back(0));
#elif SSTL_ON_ERROR == SSTL_PANIC
  g_family_panic_count = 0;
  sstl::string<1> s;
  if (setjmp(g_family_panic_env) == 0) {
    (void)s.at(0);
    SSTL_TEST_ASSERT(0 && "string::at on empty string must panic");
  }
  SSTL_TEST_EQ(g_family_panic_count, 1);

  sstl::deque<int, 1> d;
  if (setjmp(g_family_panic_env) == 0) {
    d.pop_back();
    SSTL_TEST_ASSERT(0 && "deque::pop_back on empty deque must panic");
  }
  SSTL_TEST_EQ(g_family_panic_count, 2);
#elif SSTL_ON_ERROR == SSTL_UB
  sstl::string<1> s;
  SSTL_TEST_ASSERT(s.push_back('x'));
  SSTL_TEST_EQ(s.at(0), 'x');
  sstl::deque<int, 1> d;
  SSTL_TEST_ASSERT(d.push_back(4));
  SSTL_TEST_EQ(d.front(), 4);
#else
# error Unknown SSTL_ON_ERROR value
#endif
}

int main() {
  const sstl_test::test_case tests[] = {
    {"sequence_family_capacity_boundaries", sequence_family_capacity_boundaries},
    {"associative_family_capacity_boundaries", associative_family_capacity_boundaries},
    {"policy_family_invalid_access_matches_mode", policy_family_invalid_access_matches_mode}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
