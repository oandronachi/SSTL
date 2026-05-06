// test_id: cpp03.string_view.contracts
// api_ids: [cpp.string_view.ctors, cpp.string_view.find, cpp.string_view.rfind, cpp.string_view.find_first_of, cpp.string_view.find_first_not_of, cpp.string_view.find_last_of, cpp.string_view.find_last_not_of, cpp.string_view.compare, cpp.string_view.starts_with, cpp.string_view.ends_with, cpp.string_view.remove_prefix, cpp.string_view.remove_suffix]
// req_ids: [REQ-05, REQ-06]
// patterns: [STL-ORACLE-EQUIV, POLICY-MATRIX]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It treats string_view as a non-owning view: mutations adjust pointer/length only and must not allocate, copy, or own characters.
 *   - The constructor checks document the intended interop with SSTL array/vector storage.
 *   - Prefix/suffix operations are expected to be cheap view edits, not string modifications.
 */
#include <sstl/array.hpp>
#include <sstl/vector.hpp>
#include <sstl/string_view.hpp>
#include "test_harness.hpp"

static void string_view_observers_and_mutating_view_ops() {
  const char text[] = "embedded";
  sstl::string_view sv(text, 8);
  SSTL_TEST_EQ(sv.size(), 8u);
  SSTL_TEST_EQ(sv.at(2u), 'b');
  SSTL_TEST_EQ(sv.front(), 'e');
  SSTL_TEST_EQ(sv.back(), 'd');
  SSTL_TEST_EQ(*sv.begin(), 'e');
  SSTL_TEST_EQ(*(sv.end() - 1), 'd');
  SSTL_TEST_EQ(*sv.cbegin(), 'e');
  SSTL_TEST_EQ(*(sv.cend() - 1), 'd');
  SSTL_TEST_EQ(sv.find('b'), 2u);
  SSTL_TEST_EQ(sv.find('d', 5u), 5u);
  SSTL_TEST_EQ(sv.find("bed"), 2u);
  SSTL_TEST_EQ(sv.find("ed", 4u), 6u);
  SSTL_TEST_EQ(sv.rfind("ed"), 6u);
  SSTL_TEST_EQ(sv.rfind("em", 1u), 0u);
  SSTL_TEST_EQ(sv.find_first_of("xyzb"), 2u);
  SSTL_TEST_EQ(sv.find_first_of('d'), 4u);
  SSTL_TEST_EQ(sv.find_first_of(0), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_not_of("em"), 2u);
  SSTL_TEST_EQ(sv.find_first_not_of('e'), 1u);
  SSTL_TEST_EQ(sv.find_first_not_of(0), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_of("dm"), 7u);
  SSTL_TEST_EQ(sv.find_last_of('e'), 6u);
  SSTL_TEST_EQ(sv.find_last_of(0), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_not_of("ed"), 2u);
  SSTL_TEST_EQ(sv.find_last_not_of('d'), 6u);
  SSTL_TEST_EQ(sv.find_last_not_of(0), sstl::npos);
  SSTL_TEST_EQ(sv.compare("embedded"), 0);
  SSTL_TEST_ASSERT(sv.starts_with("em"));
  SSTL_TEST_ASSERT(sv.ends_with("ed"));
  sv.remove_prefix(2);
  SSTL_TEST_ASSERT(sv.starts_with("be"));
  sv.remove_suffix(2);
  SSTL_TEST_ASSERT(sv.ends_with("dd"));
}

static void string_view_constructs_from_static_containers() {
  sstl::array<char, 4> a;
  a[0] = 't'; a[1] = 'e'; a[2] = 's'; a[3] = 't';
  sstl::string_view av(a.data(), a.size());
  SSTL_TEST_EQ(av.size(), 4u);

  sstl::vector<char, 4> v;
  SSTL_TEST_ASSERT(v.push_back('v'));
  SSTL_TEST_ASSERT(v.push_back('i'));
  sstl::string_view vv(v.data(), v.size());
  SSTL_TEST_EQ(vv.compare("vi"), 0);
}

static void string_view_empty_null_substr_remains_empty() {
  const char* null_chars = 0;
  sstl::string_view sv(null_chars, 0);
  sstl::string_view sub = sv.substr();
  SSTL_TEST_ASSERT(sub.empty());
  SSTL_TEST_EQ(sub.size(), 0u);
  SSTL_TEST_ASSERT(sub.data() != 0);
}

static void string_view_search_edges_report_absence_cleanly() {
  const sstl::string_view sv("embedded", 8);
  const sstl::string_view empty("", 0);
  const sstl::string_view repeated("eee", 3);
  const char* no_chars = 0;
  SSTL_TEST_EQ(sv.find(0), sstl::npos);
  SSTL_TEST_EQ(sv.find('z'), sstl::npos);
  SSTL_TEST_EQ(sv.find('e', 99u), sstl::npos);
  SSTL_TEST_EQ(sv.find(static_cast<int>('m')), 1u);
  SSTL_TEST_EQ(sv.find("zz"), sstl::npos);
  SSTL_TEST_EQ(sv.find("bed", 0u, 3u), 2u);
  SSTL_TEST_EQ(sv.find("e", 1u, sstl::npos), sstl::npos);
  SSTL_TEST_EQ(sv.find("", 3u), 3u);
  SSTL_TEST_EQ(sv.find("", 99u), sstl::npos);
  SSTL_TEST_EQ(sv.rfind(0), sstl::npos);
  SSTL_TEST_EQ(sv.rfind("too-long-to-match"), sstl::npos);
  SSTL_TEST_EQ(sv.rfind("", sstl::npos), sv.size());
  SSTL_TEST_EQ(sv.rfind("", 2u), 2u);
  SSTL_TEST_EQ(sv.rfind("", 99u), sv.size());
  SSTL_TEST_EQ(sv.rfind("em", 0u), 0u);
  SSTL_TEST_EQ(sv.rfind("ed", 99u), 6u);
  SSTL_TEST_EQ(sv.rfind("ed", 2u), sstl::npos);

  SSTL_TEST_EQ(sv.find_first_of(no_chars), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_of(0), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_of("z"), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_of("e", 99u), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_of('z'), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_of(static_cast<int>('b')), 2u);
  SSTL_TEST_EQ(empty.find_first_of('e'), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_not_of(no_chars), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_not_of(0), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_not_of("embed"), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_not_of("e", 99u), sstl::npos);
  SSTL_TEST_EQ(repeated.find_first_not_of('e'), sstl::npos);
  SSTL_TEST_EQ(sv.find_first_not_of(static_cast<int>('e')), 1u);
  SSTL_TEST_EQ(empty.find_first_not_of('e'), sstl::npos);

  SSTL_TEST_EQ(sv.find_last_of(no_chars), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_of(0), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_of("z"), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_of("e", 4u), 3u);
  SSTL_TEST_EQ(sv.find_last_of("e", 99u), 6u);
  SSTL_TEST_EQ(sv.find_last_of('z'), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_of('e', 99u), 6u);
  SSTL_TEST_EQ(sv.find_last_of(static_cast<int>('b')), 2u);
  SSTL_TEST_EQ(empty.find_last_of('e'), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_not_of(no_chars), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_not_of(0), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_not_of("embed"), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_not_of("d", 4u), 3u);
  SSTL_TEST_EQ(sv.find_last_not_of("d", 99u), 6u);
  SSTL_TEST_EQ(repeated.find_last_not_of('e'), sstl::npos);
  SSTL_TEST_EQ(sv.find_last_not_of('d', 99u), 6u);
  SSTL_TEST_EQ(sv.find_last_not_of(static_cast<int>('d')), 6u);
  SSTL_TEST_EQ(empty.find_last_not_of('e'), sstl::npos);

  SSTL_TEST_ASSERT(sv.compare("z") < 0);
  SSTL_TEST_ASSERT(sv.compare("embeddedx") < 0);
  SSTL_TEST_ASSERT(sv.compare("embed") > 0);
  SSTL_TEST_ASSERT(!sv.starts_with("zz"));
  SSTL_TEST_ASSERT(!sv.starts_with("embeddedx"));
  SSTL_TEST_ASSERT(!sv.ends_with("zz"));
  SSTL_TEST_ASSERT(!sv.ends_with("xembedded"));
  {
    sstl::string_view clipped("abc", 3);
    clipped.remove_prefix(99u);
    SSTL_TEST_ASSERT(clipped.empty());
  }
  {
    sstl::string_view clipped("abc", 3);
    clipped.remove_suffix(99u);
    SSTL_TEST_ASSERT(clipped.empty());
  }
}

int main() {
  const sstl_test::test_case tests[] = {
    {"string_view_observers_and_mutating_view_ops", string_view_observers_and_mutating_view_ops},
    {"string_view_constructs_from_static_containers", string_view_constructs_from_static_containers},
    {"string_view_empty_null_substr_remains_empty", string_view_empty_null_substr_remains_empty},
    {"string_view_search_edges_report_absence_cleanly", string_view_search_edges_report_absence_cleanly}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
