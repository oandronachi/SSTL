// test_id: cpp03.string.mutation_members
// api_ids: [cpp.string.append_n, cpp.string.assign, cpp.string.insert, cpp.string.erase, cpp.string.replace, cpp.string.pop_back, cpp.string.resize, cpp.string.swap, cpp.string.operator_plus_equal, cpp.string.compare, cpp.string.ordering, cpp.string.try_front, cpp.string.try_back, cpp.string.find_first_of, cpp.string.find_first_not_of, cpp.string.find_last_of, cpp.string.find_last_not_of]
// req_ids: [REQ-03, REQ-05, REQ-06]
// patterns: [CAPACITY-BOUNDARY, STL-ORACLE-EQUIV, AUDIT-NOALLOC]
/*
 * Notes:
 *   - These checks pin the fixed-capacity string mutators that are easy to miss when only append/find are tested.
 *   - Failed growth must leave the previous C string intact so callers can recover in RETURN-policy builds.
 *   - Search-family assertions mirror the standard string laws without depending on C++11 string_view support.
 */
#include <sstl/string.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static void assign_insert_erase_replace_and_plus_equal_follow_string_model() {
  sstl_test::noalloc_guard guard;
  sstl::string<16> s;
  SSTL_TEST_ASSERT(s.assign("ace"));
  SSTL_TEST_ASSERT(s.insert(1, 'b'));
  SSTL_TEST_ASSERT(s.insert(3, "d"));
  SSTL_TEST_ASSERT(s == "abcde");
  s.erase(1, 2);
  SSTL_TEST_ASSERT(s == "ade");
  SSTL_TEST_ASSERT(s.replace(1, 1, "BCD"));
  SSTL_TEST_ASSERT(s == "aBCDe");
  SSTL_TEST_ASSERT(s += "fg");
  SSTL_TEST_ASSERT(s += 'h');
  SSTL_TEST_ASSERT(s == "aBCDefgh");
}

static void bounded_operations_fail_without_partial_mutation() {
  sstl_test::noalloc_guard guard;
  sstl::string<5> s("abc");
  SSTL_TEST_ASSERT(!s.append("def"));
  SSTL_TEST_ASSERT(s == "abc");
  SSTL_TEST_ASSERT(!s.insert(1, "wxyz"));
  SSTL_TEST_ASSERT(s == "abc");
  SSTL_TEST_ASSERT(!s.replace(1, 0, "wxyz"));
  SSTL_TEST_ASSERT(s == "abc");
  SSTL_TEST_ASSERT(s.append("de", 2));
  SSTL_TEST_ASSERT(s == "abcde");
  SSTL_TEST_ASSERT(!(s += 'f'));
  SSTL_TEST_ASSERT(s == "abcde");
}

static void self_aliasing_mutators_stage_source_before_writing() {
  sstl_test::noalloc_guard guard;
  sstl::string<16> s("abc");
  SSTL_TEST_ASSERT(s.assign(s));
  SSTL_TEST_ASSERT(s == "abc");

  sstl::string<16> subassign("abcdef");
  SSTL_TEST_ASSERT(subassign.assign(subassign, 2u, 3u));
  SSTL_TEST_ASSERT(subassign == "cde");

  sstl::string<16> insert_all("abc");
  SSTL_TEST_ASSERT(insert_all.insert(1u, insert_all));
  SSTL_TEST_ASSERT(insert_all == "aabcbc");

  sstl::string<16> insert_sub("abcdef");
  SSTL_TEST_ASSERT(insert_sub.insert(2u, insert_sub, 1u, 3u));
  SSTL_TEST_ASSERT(insert_sub == "abbcdcdef");

  sstl::string<16> replace_all("abcdef");
  SSTL_TEST_ASSERT(replace_all.replace(1u, 3u, replace_all));
  SSTL_TEST_ASSERT(replace_all == "aabcdefef");

  sstl::string<16> replace_sub("abcdef");
  SSTL_TEST_ASSERT(replace_sub.replace(1u, 3u, replace_sub.c_str() + 2u, 2u));
  SSTL_TEST_ASSERT(replace_sub == "acdef");
}

static void pop_back_resize_and_swap_follow_string_model() {
  sstl_test::noalloc_guard guard;
  sstl::string<8> s("abc");
  s.pop_back();
  SSTL_TEST_ASSERT(s == "ab");
  SSTL_TEST_EQ(s.size(), 2u);
  SSTL_TEST_EQ(s.c_str()[2], '\0');
  SSTL_TEST_ASSERT(s.resize(5u, 'x'));
  SSTL_TEST_ASSERT(s == "abxxx");
  SSTL_TEST_EQ(s.size(), 5u);
  SSTL_TEST_ASSERT(s.resize(2u));
  SSTL_TEST_ASSERT(s == "ab");
  SSTL_TEST_EQ(s.c_str()[2], '\0');
  SSTL_TEST_ASSERT(!s.resize(9u, 'z'));
  SSTL_TEST_ASSERT(s == "ab");
  sstl::string<8> other("Q");
  s.swap(other);
  SSTL_TEST_ASSERT(s == "Q");
  SSTL_TEST_ASSERT(other == "ab");
  sstl::swap(s, other);
  SSTL_TEST_ASSERT(s == "ab");
  SSTL_TEST_ASSERT(other == "Q");
  sstl::string<2> empty;
  empty.pop_back();
  SSTL_TEST_ASSERT(empty.empty());
  SSTL_TEST_EQ(empty.c_str()[0], '\0');
}

static void character_set_searches_match_expected_positions() {
  sstl_test::noalloc_guard guard;
  sstl::string<16> s("abca xyz");
  SSTL_TEST_EQ(s.find_first_of("xz"), 5u);
  SSTL_TEST_EQ(s.find_first_of('x'), 5u);
  SSTL_TEST_EQ(s.find_first_not_of("abc"), 4u);
  SSTL_TEST_EQ(s.find_first_not_of('a'), 1u);
  SSTL_TEST_EQ(s.find_last_of("abc"), 3u);
  SSTL_TEST_EQ(s.find_last_of('a'), 3u);
  SSTL_TEST_EQ(s.find_last_not_of("xyz"), 4u);
  SSTL_TEST_EQ(s.find_last_not_of('z'), 6u);
  SSTL_TEST_EQ(s.find_first_of("q"), sstl::npos);
  SSTL_TEST_EQ(s.find_first_of('q'), sstl::npos);
}

static void cross_capacity_compare_uses_current_text_only() {
  sstl_test::noalloc_guard guard;
  sstl::string<16> left("abc");
  sstl::string<4> equal("abc");
  sstl::string<8> greater("abd");
  sstl::string<8> shorter("ab");
  SSTL_TEST_EQ(left.compare(equal), 0);
  SSTL_TEST_ASSERT(left == equal);
  SSTL_TEST_ASSERT(!(left != equal));
  SSTL_TEST_ASSERT(left <= equal);
  SSTL_TEST_ASSERT(left >= equal);
  SSTL_TEST_ASSERT(left.compare(greater) < 0);
  SSTL_TEST_ASSERT(left < greater);
  SSTL_TEST_ASSERT(left <= greater);
  SSTL_TEST_ASSERT(greater > left);
  SSTL_TEST_ASSERT(greater >= left);
  SSTL_TEST_ASSERT(left != greater);
  SSTL_TEST_ASSERT(left.compare(shorter) > 0);
  SSTL_TEST_ASSERT(left > shorter);
  SSTL_TEST_ASSERT(shorter < left);
  SSTL_TEST_ASSERT(left != "abd");
  SSTL_TEST_ASSERT(left < "abd");
  SSTL_TEST_ASSERT(left <= "abc");
  SSTL_TEST_ASSERT(left > "abb");
  SSTL_TEST_ASSERT(left >= "abc");
  SSTL_TEST_ASSERT("abb" < left);
  SSTL_TEST_ASSERT("abc" <= left);
  SSTL_TEST_ASSERT("abd" > left);
  SSTL_TEST_ASSERT("abc" >= left);
  SSTL_TEST_ASSERT(left += equal);
  SSTL_TEST_ASSERT(left == "abcabc");
  SSTL_TEST_EQ(*left.try_front(), 'a');
  SSTL_TEST_EQ(*left.try_back(), 'c');
  sstl::string<4> empty;
  SSTL_TEST_ASSERT(empty.try_front() == 0);
  SSTL_TEST_ASSERT(empty.try_back() == 0);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"assign_insert_erase_replace_and_plus_equal_follow_string_model", assign_insert_erase_replace_and_plus_equal_follow_string_model},
    {"bounded_operations_fail_without_partial_mutation", bounded_operations_fail_without_partial_mutation},
    {"self_aliasing_mutators_stage_source_before_writing", self_aliasing_mutators_stage_source_before_writing},
    {"pop_back_resize_and_swap_follow_string_model", pop_back_resize_and_swap_follow_string_model},
    {"character_set_searches_match_expected_positions", character_set_searches_match_expected_positions},
    {"cross_capacity_compare_uses_current_text_only", cross_capacity_compare_uses_current_text_only}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
