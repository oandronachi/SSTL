// test_id: cpp03.string.find_compare.oracle
// api_ids: [cpp.string.compare, cpp.string.find, cpp.string.rfind]
// req_ids: [REQ-05]
// patterns: [STL-ORACLE-EQUIV]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is a direct SSTL contract test; prefer changing the manifest and comments together when behavior changes.
 *   - The assertions are black-box: they verify public observable state rather than private representation.
 *   - If this test fails against an implementation, first compare the implementation API to the SSTL public contract.
 */
#include <sstl/string.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void find_compare_and_substr_follow_string_model() {
  sstl_test::noalloc_guard guard;
  sstl::string<16> s;
  const int int_b = 'b';
  SSTL_TEST_ASSERT(s.append("ababa"));
  SSTL_TEST_EQ(s.find("ba"), 1u);
  SSTL_TEST_EQ(s.find('a'), 0u);
  SSTL_TEST_EQ(s.find('a', 1u), 2u);
  SSTL_TEST_EQ(s.find(int_b, 2u), 3u);
  SSTL_TEST_EQ(s.find('z'), sstl::npos);
  SSTL_TEST_EQ(s.find(0), sstl::npos);
  SSTL_TEST_EQ(s.rfind("ba"), 3u);
  SSTL_TEST_EQ(s.rfind("ba", 2u), 1u);
  SSTL_TEST_EQ(s.rfind("ba", 0u), sstl::npos);
  SSTL_TEST_EQ(s.rfind('a'), 4u);
  SSTL_TEST_EQ(s.rfind('a', 3u), 2u);
  SSTL_TEST_EQ(s.rfind(int_b, 2u), 1u);
  SSTL_TEST_EQ(s.rfind('z'), sstl::npos);
  SSTL_TEST_EQ(s.rfind(0), sstl::npos);
  SSTL_TEST_EQ(s.rfind("", 99u), s.size());
  SSTL_TEST_EQ(s.compare("ababa"), 0);
  sstl::string<16> sub = s.substr(1, 3);
  SSTL_TEST_ASSERT(sub == "bab");
}

int main() {
  const sstl_test::test_case tests[] = {
    {"find_compare_and_substr_follow_string_model", find_compare_and_substr_follow_string_model}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
