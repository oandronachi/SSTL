// test_id: cpp03.coverage.gap.contracts
// api_ids: [cpp.string.push_back, cpp.string.pop_back, cpp.forward_list.contract, cpp.map.contract, cpp.vector.push_back, cpp.vector.try_insert, cpp.vector.erase, cpp.vector.resize, cpp.vector.comparison, cpp.deque.contract, cpp.deque.swap, cpp.list.splice, cpp.list.try_splice, cpp.set.contract, cpp.set.erase, cpp.flat_map.contract, cpp.flat_map.erase, cpp.flat_map.bounds, cpp.flat_set.contract, cpp.flat_set.erase, cpp.unordered_map.contract, cpp.unordered_map.erase, cpp.unordered_map.subscript, cpp.unordered_set.contract, cpp.unordered_set.erase, cpp.algorithm.comparator_overloads, cpp.function.contract]
// req_ids: [REQ-03, REQ-05, REQ-06, REQ-RUNTIME-COMPARISON]
// patterns: [CAPACITY-BOUNDARY, EDGE-BRANCH-COVERAGE, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This file is intentionally explicit about rare branches. Its job is not
 *     to re-prove the happy path; the other oracle tests already do that.
 *   - Each case below targets valid RETURN-policy behavior that LCOV showed was
 *     easy to miss: full fixed-capacity containers, empty removals, null-style
 *     iterator sentinels, failed searches, and red/black map rebalancing.
 *   - The tests keep the no-allocation guard active, so extra coverage does not
 *     come from using hosted containers or heap-backed helpers.
 *   - The assertions are still behavioral contracts. If a line becomes covered
 *     only by relying on undefined behavior, the test should be rewritten.
 */
#include <sstl/deque.hpp>
#include <sstl/algorithm.hpp>
#include <sstl/array.hpp>
#include <sstl/bitset.hpp>
#include <sstl/flat_map.hpp>
#include <sstl/flat_set.hpp>
#include <sstl/forward_list.hpp>
#include <sstl/functional.hpp>
#include <sstl/inplace_function.hpp>
#include <sstl/list.hpp>
#include <sstl/map.hpp>
#include <sstl/optional.hpp>
#include <sstl/priority_queue.hpp>
#include <sstl/set.hpp>
#include <sstl/span.hpp>
#include <sstl/string.hpp>
#include <sstl/string_view.hpp>
#include <sstl/unordered_map.hpp>
#include <sstl/unordered_set.hpp>
#include <sstl/variant.hpp>
#include <sstl/vector.hpp>

#include "noalloc_audit.hpp"
#include "test_harness.hpp"

static bool is_even_int(int value) {
  return (value % 2) == 0;
}

static bool is_negative_int(int value) {
  return value < 0;
}

static bool is_positive_int(int value) {
  return value > 0;
}

static bool is_large_int(int value) {
  return value > 100;
}

static bool always_true_int(int) {
  return true;
}

static bool equal_ints(int a, int b) {
  return a == b;
}

static int double_int(int value) {
  return value * 2;
}

static int add_two_ints(int a, int b) {
  return a + b;
}

static int add_three_ints(int a, int b, int c) {
  return a + b + c;
}

static bool same_mod_ten(int a, int b) {
  return (a % 10) == (b % 10);
}

struct constant_hash_int {
  unsigned operator()(int) const { return 0u; }
};

struct greater_int {
  bool operator()(int a, int b) const { return a > b; }
};

struct mutable_nullary {
  int value;
  mutable_nullary() : value(0) {}
  int operator()() { return ++value; }
};

struct mutable_unary {
  int value;
  mutable_unary() : value(0) {}
  int operator()(int a) { value += a; return value; }
};

struct mutable_binary {
  int value;
  mutable_binary() : value(0) {}
  int operator()(int a, int b) { value += a + b; return value; }
};

struct mutable_ternary {
  int value;
  mutable_ternary() : value(0) {}
  int operator()(int a, int b, int c) { value += a + b + c; return value; }
};

template <sstl::size_t N>
static void expect_forward_list_values(sstl::forward_list<int, N>& list,
                                       const int* expected,
                                       unsigned expected_count) {
  unsigned index = 0u;
  for (typename sstl::forward_list<int, N>::iterator it = list.begin(); it != list.end(); ++it) {
    SSTL_TEST_ASSERT(index < expected_count);
    SSTL_TEST_EQ(*it, expected[index]);
    ++index;
  }
  SSTL_TEST_EQ(index, expected_count);
  SSTL_TEST_EQ(list.size(), expected_count);
}

template <sstl::size_t N>
static void expect_list_values(sstl::list<int, N>& list,
                               const int* expected,
                               unsigned expected_count) {
  unsigned index = 0u;
  for (typename sstl::list<int, N>::iterator it = list.begin(); it != list.end(); ++it) {
    SSTL_TEST_ASSERT(index < expected_count);
    SSTL_TEST_EQ(*it, expected[index]);
    ++index;
  }
  SSTL_TEST_EQ(index, expected_count);
  SSTL_TEST_EQ(list.size(), expected_count);
}

template <sstl::size_t N>
static void expect_map_state(sstl::map<int, int, N>& map,
                             const bool* present,
                             unsigned max_key) {
  unsigned expected_size = 0u;
  unsigned iterated_size = 0u;
  for (unsigned key = 0u; key <= max_key; ++key) {
    if (present[key]) ++expected_size;
  }

  typename sstl::map<int, int, N>::iterator it = map.begin();
  for (unsigned key = 0u; key <= max_key; ++key) {
    if (!present[key]) continue;
    SSTL_TEST_ASSERT(it != map.end());
    SSTL_TEST_EQ(it->first, static_cast<int>(key));
    SSTL_TEST_EQ(it->second, static_cast<int>(key * 10u));
    ++it;
    ++iterated_size;
  }
  SSTL_TEST_ASSERT(it == map.end());
  SSTL_TEST_EQ(iterated_size, expected_size);
  SSTL_TEST_EQ(map.size(), expected_size);
}

static void insert_map_sequence(sstl::map<int, int, 16>& map,
                                bool* present,
                                const int* order,
                                unsigned count) {
  for (unsigned i = 0u; i != count; ++i) {
    const int key = order[i];
    sstl::pair<sstl::map<int, int, 16>::iterator, bool> inserted =
      map.insert(sstl::make_pair(key, key * 10));
    SSTL_TEST_ASSERT(inserted.second);
    SSTL_TEST_ASSERT(inserted.first != map.end());
    present[key] = true;
    expect_map_state(map, present, 15u);
  }
}

static void erase_map_sequence(sstl::map<int, int, 16>& map,
                               bool* present,
                               const int* order,
                               unsigned count) {
  for (unsigned i = 0u; i != count; ++i) {
    const int key = order[i];
    SSTL_TEST_EQ(map.erase(key), 1u);
    present[key] = false;
    expect_map_state(map, present, 15u);
  }
}

static void run_small_map_order_pair(const int* insert_order,
                                     const int* erase_order,
                                     unsigned count) {
  sstl::map<int, int, 5> map;
  bool present[6];
  for (unsigned i = 0u; i != 6u; ++i) present[i] = false;

  for (unsigned i = 0u; i != count; ++i) {
    const int key = insert_order[i];
    SSTL_TEST_ASSERT(map.insert(sstl::make_pair(key, key * 10)).second);
    present[key] = true;
  }
  expect_map_state(map, present, 5u);

  for (unsigned i = 0u; i != count; ++i) {
    const int key = erase_order[i];
    SSTL_TEST_EQ(map.erase(key), 1u);
    present[key] = false;
    expect_map_state(map, present, 5u);
  }
  SSTL_TEST_ASSERT(map.empty());
}

static void generate_small_map_erase_orders(const int* insert_order,
                                            int* erase_order,
                                            bool* erase_used,
                                            unsigned depth) {
  /*
    Red/black erase fix-up coverage is tree-shape sensitive. The five-key
    permutation matrix below is intentionally small enough to remain fast, but
    broad enough to visit left/right sibling cases that hand-picked examples
    easily miss. It stays deterministic and uses only stack arrays.
  */
  if (depth == 5u) {
    run_small_map_order_pair(insert_order, erase_order, 5u);
    return;
  }

  for (int key = 1; key <= 5; ++key) {
    if (!erase_used[key]) {
      erase_used[key] = true;
      erase_order[depth] = key;
      generate_small_map_erase_orders(insert_order, erase_order, erase_used, depth + 1u);
      erase_used[key] = false;
    }
  }
}

static void generate_small_map_insert_orders(int* insert_order,
                                             bool* insert_used,
                                             unsigned depth) {
  if (depth == 5u) {
    int erase_order[5];
    bool erase_used[6];
    for (unsigned i = 0u; i != 6u; ++i) erase_used[i] = false;
    generate_small_map_erase_orders(insert_order, erase_order, erase_used, 0u);
    return;
  }

  for (int key = 1; key <= 5; ++key) {
    if (!insert_used[key]) {
      insert_used[key] = true;
      insert_order[depth] = key;
      generate_small_map_insert_orders(insert_order, insert_used, depth + 1u);
      insert_used[key] = false;
    }
  }
}

static void array_span_and_string_weak_branch_edges_are_exercised() {
  sstl_test::noalloc_guard guard;

  /*
    Array's self-assignment branch is easy to miss because ordinary copy tests
    naturally use two different objects. The self-assignment case must be a
    no-op that preserves the existing elements.
  */
  sstl::array<int, 3> array_values;
  array_values.fill(7);
  array_values = array_values;
  SSTL_TEST_EQ(array_values[0], 7);
  SSTL_TEST_EQ(array_values[1], 7);
  SSTL_TEST_EQ(array_values[2], 7);
  {
    const sstl::array<int, 3>& const_array_values = array_values;
    SSTL_TEST_EQ(const_array_values.try_front()[0], 7);
    SSTL_TEST_EQ(const_array_values.try_back()[0], 7);
  }

  /*
    Span can safely validate invalid iterators when the view is a middle slice
    of a larger C array: the "before view" and "after view" probes are still
    pointers into the same underlying array object, so the relational checks are
    well-defined.
  */
  int raw_span_values[5] = {0, 1, 2, 3, 4};
  sstl::span<int> middle(raw_span_values + 1, raw_span_values + 4);
  SSTL_TEST_ASSERT(middle.is_valid_iterator(raw_span_values + 1));
  SSTL_TEST_ASSERT(middle.is_valid_iterator(raw_span_values + 4));
  SSTL_TEST_ASSERT(!middle.is_valid_iterator(raw_span_values));
  SSTL_TEST_ASSERT(!middle.is_valid_iterator(raw_span_values + 5));
  {
    sstl::span<int>::iterator wrapped_begin(raw_span_values + 1);
    sstl::span<int>::iterator wrapped_end(raw_span_values + 4);
    sstl::span<int>::iterator wrapped_before(raw_span_values);
    sstl::span<int>::iterator wrapped_after(raw_span_values + 5);
    SSTL_TEST_ASSERT(middle.is_valid_iterator(wrapped_begin));
    SSTL_TEST_ASSERT(middle.is_valid_iterator(wrapped_end));
    SSTL_TEST_ASSERT(!middle.is_valid_iterator(wrapped_before));
    SSTL_TEST_ASSERT(!middle.is_valid_iterator(wrapped_after));
  }
  {
    const sstl::span<int>& const_middle = middle;
    const int* before = raw_span_values;
    const int* begin = raw_span_values + 1;
    const int* end = raw_span_values + 4;
    const int* after = raw_span_values + 5;
    SSTL_TEST_ASSERT(const_middle.is_valid_iterator(begin));
    SSTL_TEST_ASSERT(const_middle.is_valid_iterator(end));
    SSTL_TEST_ASSERT(!const_middle.is_valid_iterator(before));
    SSTL_TEST_ASSERT(!const_middle.is_valid_iterator(after));
  }
  {
    sstl::span<int> empty_span;
    SSTL_TEST_ASSERT(empty_span.is_valid_iterator(static_cast<int*>(0)));
    SSTL_TEST_ASSERT(empty_span.is_valid_iterator(static_cast<const int*>(0)));
    SSTL_TEST_ASSERT(empty_span.is_valid_iterator(empty_span.begin()));
    SSTL_TEST_ASSERT(!empty_span.is_valid_iterator(raw_span_values));
    SSTL_TEST_ASSERT(!empty_span.is_valid_iterator(static_cast<const int*>(raw_span_values)));
    SSTL_TEST_ASSERT(!empty_span.is_valid_iterator(sstl::span<int>::iterator(raw_span_values)));
  }

  /*
    String branch coverage is mostly about the alternate public outcomes:
    capacity rejection because a string is already full, invalid positions,
    zero-length insertion decisions, over-long replacement counts, equal-length
    replacement, null character sets, and empty-search boundaries.
  */
  sstl::string<5> text("abc");
  const char* null_chars = 0;
  SSTL_TEST_ASSERT(!text.insert(99u, "x"));
  SSTL_TEST_EQ(text.find_first_of(null_chars), sstl::npos);
  SSTL_TEST_EQ(text.find_first_not_of(null_chars), sstl::npos);
  SSTL_TEST_EQ(text.find_last_of(null_chars), sstl::npos);
  SSTL_TEST_EQ(text.find_last_not_of(null_chars), sstl::npos);
  SSTL_TEST_EQ(text.find_first_of("a", text.size()), sstl::npos);
  SSTL_TEST_EQ(text.find_first_not_of("abc", text.size()), sstl::npos);
  SSTL_TEST_EQ(text.find_last_of("a", 1u), 0u);
  SSTL_TEST_EQ(text.find_last_not_of("c", 2u), 1u);

  sstl::string<3> full_text("abc");
  SSTL_TEST_ASSERT(!full_text.insert(1u, 'x'));
  SSTL_TEST_ASSERT(!full_text.insert(1u, "x"));
  SSTL_TEST_ASSERT(full_text == "abc");

  /*
    string hashes must describe the logical character range, not the whole
    fixed-capacity backing array. This specifically guards the pop_back tail:
    the removed byte remains in unused capacity, but equal strings must still
    hash the same when used as unordered-map keys.
  */
  sstl::string<4> hash_left("abx");
  sstl::string<4> hash_right("ab");
  SSTL_TEST_ASSERT(hash_left.pop_back());
  sstl::hash<sstl::string<4> > string_hash;
  SSTL_TEST_EQ(string_hash(hash_left), string_hash(hash_right));
  SSTL_TEST_ASSERT(hash_right.push_back('c'));
  SSTL_TEST_ASSERT(string_hash(hash_left) != string_hash(hash_right));

  sstl::string<8> replace_text("abc");
  SSTL_TEST_ASSERT(replace_text.replace(1u, 99u, "x"));
  SSTL_TEST_ASSERT(replace_text == "ax");
  SSTL_TEST_ASSERT(replace_text.assign("abc"));
  SSTL_TEST_ASSERT(replace_text.replace(1u, 1u, "Z"));
  SSTL_TEST_ASSERT(replace_text == "aZc");

  sstl::string<4> empty_text;
  SSTL_TEST_EQ(empty_text.find_first_of("a"), sstl::npos);
  SSTL_TEST_EQ(empty_text.find_first_not_of("a"), sstl::npos);
  SSTL_TEST_EQ(empty_text.find_last_of("a"), sstl::npos);
  SSTL_TEST_EQ(empty_text.find_last_not_of("a"), sstl::npos);
}

static void string_edge_paths_keep_text_recoverable() {
  sstl_test::noalloc_guard guard;

  /*
    The failed mutators below intentionally stay inside RETURN-policy contracts:
    they report false or leave the string untouched rather than forcing a panic.
  */
  sstl::string<5> s("abc");
  SSTL_TEST_ASSERT(!s.assign("abcdef"));
  SSTL_TEST_ASSERT(s == "abc");
  SSTL_TEST_ASSERT(!s.insert(99u, 'x'));
  SSTL_TEST_ASSERT(!s.insert(1u, "wxyz"));
  SSTL_TEST_ASSERT(s.insert(1u, 'x'));
  SSTL_TEST_ASSERT(s == "axbc");
  s.erase(1u, 1u);
  SSTL_TEST_ASSERT(s.insert(3u, ""));
  SSTL_TEST_ASSERT(!s.insert(99u, ""));
  s.erase(99u, 1u);
  SSTL_TEST_ASSERT(s == "abc");
  s.erase(1u, 99u);
  SSTL_TEST_ASSERT(s == "a");

  SSTL_TEST_ASSERT(s.assign("abcde"));
  SSTL_TEST_ASSERT(!s.replace(9u, 1u, "x"));
  SSTL_TEST_ASSERT(!s.replace(1u, 0u, "wxyz"));
  SSTL_TEST_ASSERT(s.replace(1u, 3u, "x"));
  SSTL_TEST_ASSERT(s == "axe");
  SSTL_TEST_ASSERT(s.replace(1u, 1u, "BCD"));
  SSTL_TEST_ASSERT(s == "aBCDe");

  SSTL_TEST_EQ(sstl::cstrlen(0), 0u);
  SSTL_TEST_EQ(sstl::cstrcmp("same", "same"), 0);
  SSTL_TEST_ASSERT(sstl::cstrcmp("b", "a") > 0);
  SSTL_TEST_ASSERT(s.compare("aBCDf") < 0);
  SSTL_TEST_ASSERT(s.try_at(1u) != 0);
  SSTL_TEST_ASSERT(s.try_at(99u) == 0);
  {
    const sstl::string<5>& const_s = s;
    SSTL_TEST_ASSERT(const_s.try_at(1u) != 0);
    SSTL_TEST_ASSERT(const_s.try_at(99u) == 0);
  }
  SSTL_TEST_EQ(s.find(0), sstl::npos);
  SSTL_TEST_EQ(s.find("a", 99u), sstl::npos);
  SSTL_TEST_EQ(s.find("", 2u), 2u);
  SSTL_TEST_EQ(s.find("zz"), sstl::npos);
  SSTL_TEST_EQ(s.find_first_of(0), sstl::npos);
  SSTL_TEST_EQ(s.find_first_of("BC"), 1u);
  SSTL_TEST_EQ(s.find_first_of("z"), sstl::npos);
  SSTL_TEST_EQ(s.find_first_of("a", 99u), sstl::npos);
  SSTL_TEST_EQ(s.find_first_not_of(0), sstl::npos);
  SSTL_TEST_EQ(s.find_first_not_of("a"), 1u);
  SSTL_TEST_EQ(s.find_first_not_of("aBCDe"), sstl::npos);
  SSTL_TEST_EQ(s.find_last_of(0), sstl::npos);
  SSTL_TEST_EQ(s.find_last_of("BC", 2u), 2u);
  SSTL_TEST_EQ(s.find_last_of("z"), sstl::npos);
  SSTL_TEST_EQ(s.find_last_not_of(0), sstl::npos);
  SSTL_TEST_EQ(s.find_last_not_of("e", 3u), 3u);
  SSTL_TEST_EQ(s.find_last_not_of("aBCDe"), sstl::npos);
  /*
    Character overloads need their own edge checks because C++03 also uses the
    integer literal 0 as the null pointer constant. The explicit int variables
    below prove that non-zero integer calls still route to character semantics,
    while the existing 0 checks above keep the legacy null-sentinel behavior.
  */
  const int int_b = 'B';
  const int int_x = 'x';
  const int int_e = 'e';
  SSTL_TEST_EQ(s.find_first_of('B'), 1u);
  SSTL_TEST_EQ(s.find_first_of(int_b), 1u);
  SSTL_TEST_EQ(s.find_first_of('B', 99u), sstl::npos);
  SSTL_TEST_EQ(s.find_first_not_of('a'), 1u);
  SSTL_TEST_EQ(s.find_first_not_of(int_x), 0u);
  SSTL_TEST_EQ(s.find_first_not_of('a', 99u), sstl::npos);
  SSTL_TEST_EQ(s.find_last_of('B'), 1u);
  SSTL_TEST_EQ(s.find_last_of(int_e), 4u);
  SSTL_TEST_EQ(s.find_last_of('z'), sstl::npos);
  SSTL_TEST_EQ(s.find_last_not_of('e'), 3u);
  SSTL_TEST_EQ(s.find_last_not_of(int_e), 3u);
  SSTL_TEST_EQ(s.find_last_not_of('x', 99u), 4u);
  sstl::string<4> same_chars("aaaa");
  SSTL_TEST_EQ(same_chars.find_first_not_of('a'), sstl::npos);
  SSTL_TEST_EQ(same_chars.find_last_not_of('a'), sstl::npos);
  sstl::string<4> empty_chars;
  SSTL_TEST_EQ(empty_chars.find_last_of('a'), sstl::npos);
  SSTL_TEST_EQ(empty_chars.find_last_not_of('a'), sstl::npos);
  SSTL_TEST_EQ(s.rfind(""), s.size());
  SSTL_TEST_EQ(s.rfind("aBCDex"), sstl::npos);
  SSTL_TEST_EQ(s.rfind("zz"), sstl::npos);

  sstl::string<5> sub = s.substr(2u, 10u);
  SSTL_TEST_ASSERT(sub == "CDe");
}

static void forward_list_transfer_sort_and_failure_paths_are_exercised() {
  sstl_test::noalloc_guard guard;

  /*
    Inserting after end() is a valid failed insertion in RETURN mode. It also
    proves the temporary node is destroyed and the slot is made reusable.
  */
  sstl::forward_list<int, 3> invalid_insert;
  SSTL_TEST_ASSERT(invalid_insert.insert_after(invalid_insert.end(), 7) == invalid_insert.end());
  SSTL_TEST_ASSERT(invalid_insert.empty());
  SSTL_TEST_ASSERT(invalid_insert.insert_after(invalid_insert.before_begin(), 1) != invalid_insert.end());
  SSTL_TEST_ASSERT(invalid_insert.insert_after(invalid_insert.before_begin(), 2) != invalid_insert.end());
  SSTL_TEST_ASSERT(invalid_insert.insert_after(invalid_insert.before_begin(), 3) != invalid_insert.end());
  SSTL_TEST_ASSERT(invalid_insert.insert_after(invalid_insert.before_begin(), 4) == invalid_insert.end());

  sstl::forward_list<int, 8> list;
  sstl::forward_list<int, 3> donor;
  sstl::forward_list<int, 2> too_large;
  SSTL_TEST_ASSERT(list.insert_after(list.before_begin(), 4) != list.end());
  SSTL_TEST_ASSERT(list.insert_after(list.before_begin(), 1) != list.end());
  SSTL_TEST_ASSERT(donor.insert_after(donor.before_begin(), 7) != donor.end());
  SSTL_TEST_ASSERT(donor.insert_after(donor.before_begin(), 5) != donor.end());
  SSTL_TEST_ASSERT(donor.insert_after(donor.before_begin(), 3) != donor.end());
  SSTL_TEST_ASSERT(too_large.insert_after(too_large.before_begin(), 9) != too_large.end());
  SSTL_TEST_ASSERT(too_large.insert_after(too_large.before_begin(), 8) != too_large.end());

  SSTL_TEST_ASSERT(list.splice_after(list.before_begin(), donor));
  SSTL_TEST_ASSERT(donor.empty());
  SSTL_TEST_ASSERT(!invalid_insert.splice_after(invalid_insert.before_begin(), too_large));

  {
    sstl::forward_list<int, 8> same_capacity_target;
    sstl::forward_list<int, 8> same_capacity_source;
    SSTL_TEST_ASSERT(same_capacity_target.insert_after(same_capacity_target.before_begin(), 1) != same_capacity_target.end());
    SSTL_TEST_ASSERT(same_capacity_source.insert_after(same_capacity_source.before_begin(), 3) != same_capacity_source.end());
    SSTL_TEST_ASSERT(same_capacity_source.insert_after(same_capacity_source.before_begin(), 2) != same_capacity_source.end());
    SSTL_TEST_ASSERT(same_capacity_target.splice_after(same_capacity_target.before_begin(), same_capacity_source));
    SSTL_TEST_ASSERT(same_capacity_source.empty());
  }

  /*
    Same-capacity whole-list splice has an optimized path that relinks public
    cases without going through the templated copy path. These probes cover the
    benign empty-source return and the capacity failure before any nodes move.
  */
  {
    sstl::forward_list<int, 4> empty_source_target;
    sstl::forward_list<int, 4> empty_source;
    SSTL_TEST_ASSERT(empty_source_target.insert_after(empty_source_target.before_begin(), 1) != empty_source_target.end());
    SSTL_TEST_ASSERT(empty_source_target.splice_after(empty_source_target.before_begin(), empty_source));
    {
      const int expected[] = {1};
      expect_forward_list_values(empty_source_target, expected, 1u);
    }
  }
  {
    sstl::forward_list<int, 2> full_same_capacity_target;
    sstl::forward_list<int, 2> same_capacity_source;
    SSTL_TEST_ASSERT(full_same_capacity_target.insert_after(full_same_capacity_target.before_begin(), 2) != full_same_capacity_target.end());
    SSTL_TEST_ASSERT(full_same_capacity_target.insert_after(full_same_capacity_target.before_begin(), 1) != full_same_capacity_target.end());
    SSTL_TEST_ASSERT(same_capacity_source.insert_after(same_capacity_source.before_begin(), 3) != same_capacity_source.end());
    SSTL_TEST_ASSERT(!full_same_capacity_target.splice_after(full_same_capacity_target.before_begin(), same_capacity_source));
    SSTL_TEST_EQ(same_capacity_source.size(), 1u);
  }

  {
    sstl::forward_list<int, 2> iterator_edges;
    SSTL_TEST_ASSERT(iterator_edges.insert_after(iterator_edges.before_begin(), 1) != iterator_edges.end());
    SSTL_TEST_ASSERT(iterator_edges.before_begin() != iterator_edges.end());
    SSTL_TEST_ASSERT(iterator_edges.erase_after(iterator_edges.end()) == iterator_edges.end());
    sstl::forward_list<int, 2>::iterator tail = iterator_edges.begin();
    SSTL_TEST_ASSERT(iterator_edges.erase_after(tail) == iterator_edges.end());
    sstl::forward_list<int, 2>::iterator mutable_end = iterator_edges.end();
    ++mutable_end;
    SSTL_TEST_ASSERT(mutable_end == iterator_edges.end());
    const sstl::forward_list<int, 2>& const_iterator_edges = iterator_edges;
    SSTL_TEST_ASSERT(const_iterator_edges.before_begin() != const_iterator_edges.end());
    sstl::forward_list<int, 2>::const_iterator const_begin = const_iterator_edges.begin();
    ++const_begin;
    SSTL_TEST_ASSERT(const_begin == const_iterator_edges.end());
    sstl::forward_list<int, 2>::const_iterator const_end = const_iterator_edges.end();
    ++const_end;
    SSTL_TEST_ASSERT(const_end == const_iterator_edges.end());
  }

  list.remove(99);
  list.remove(5);
  list.remove_if(is_even_int);
  SSTL_TEST_ASSERT(list.insert_after(list.before_begin(), 3) != list.end());
  SSTL_TEST_ASSERT(list.insert_after(list.before_begin(), 3) != list.end());
  list.unique(equal_ints);

  sstl::forward_list<int, 8> unsorted;
  SSTL_TEST_ASSERT(unsorted.insert_after(unsorted.before_begin(), 2) != unsorted.end());
  SSTL_TEST_ASSERT(unsorted.insert_after(unsorted.before_begin(), 5) != unsorted.end());
  SSTL_TEST_ASSERT(unsorted.insert_after(unsorted.before_begin(), 1) != unsorted.end());
  SSTL_TEST_ASSERT(unsorted.insert_after(unsorted.before_begin(), 4) != unsorted.end());
  SSTL_TEST_ASSERT(unsorted.insert_after(unsorted.before_begin(), 3) != unsorted.end());
  unsorted.sort();
  {
    const int expected[] = {1, 2, 3, 4, 5};
    expect_forward_list_values(unsorted, expected, 5u);
  }

  sstl::forward_list<int, 10> merge_target;
  sstl::forward_list<int, 4> merge_source;
  SSTL_TEST_ASSERT(merge_target.insert_after(merge_target.before_begin(), 4) != merge_target.end());
  SSTL_TEST_ASSERT(merge_target.insert_after(merge_target.before_begin(), 1) != merge_target.end());
  merge_target.sort();
  SSTL_TEST_ASSERT(merge_source.insert_after(merge_source.before_begin(), 5) != merge_source.end());
  SSTL_TEST_ASSERT(merge_source.insert_after(merge_source.before_begin(), 3) != merge_source.end());
  SSTL_TEST_ASSERT(merge_source.insert_after(merge_source.before_begin(), 2) != merge_source.end());
  merge_source.sort();
  SSTL_TEST_ASSERT(merge_target.merge(merge_source));
  SSTL_TEST_ASSERT(merge_source.empty());
  {
    const int expected[] = {1, 2, 3, 4, 5};
    expect_forward_list_values(merge_target, expected, 5u);
  }

  sstl::forward_list<int, 2> full;
  sstl::forward_list<int, 1> one;
  SSTL_TEST_ASSERT(full.insert_after(full.before_begin(), 2) != full.end());
  SSTL_TEST_ASSERT(full.insert_after(full.before_begin(), 1) != full.end());
  SSTL_TEST_ASSERT(one.insert_after(one.before_begin(), 3) != one.end());
  SSTL_TEST_ASSERT(!full.merge(one));
  SSTL_TEST_ASSERT(!full.splice_after(full.before_begin(), one, one.before_begin()));
  SSTL_TEST_ASSERT(full.splice_after(full.before_begin(), full));

  {
    sstl::forward_list<int, 2> same_merge_full_target;
    sstl::forward_list<int, 2> same_merge_source;
    SSTL_TEST_ASSERT(same_merge_full_target.insert_after(same_merge_full_target.before_begin(), 2) != same_merge_full_target.end());
    SSTL_TEST_ASSERT(same_merge_full_target.insert_after(same_merge_full_target.before_begin(), 1) != same_merge_full_target.end());
    SSTL_TEST_ASSERT(same_merge_source.insert_after(same_merge_source.before_begin(), 3) != same_merge_source.end());
    SSTL_TEST_ASSERT(!same_merge_full_target.merge(same_merge_source));
    SSTL_TEST_EQ(same_merge_source.size(), 1u);
  }

  {
    sstl::forward_list<int, 1> full_target;
    sstl::forward_list<int, 1> same_capacity_source;
    SSTL_TEST_ASSERT(full_target.insert_after(full_target.before_begin(), 1) != full_target.end());
    SSTL_TEST_ASSERT(same_capacity_source.insert_after(same_capacity_source.before_begin(), 2) != same_capacity_source.end());
    SSTL_TEST_ASSERT(!full_target.splice_after(full_target.before_begin(), same_capacity_source, same_capacity_source.before_begin()));
  }

  {
    sstl::forward_list<int, 4> single_target;
    sstl::forward_list<int, 4> single_source;
    SSTL_TEST_ASSERT(single_target.insert_after(single_target.before_begin(), 4) != single_target.end());
    SSTL_TEST_ASSERT(single_target.insert_after(single_target.before_begin(), 1) != single_target.end());
    SSTL_TEST_ASSERT(single_source.insert_after(single_source.before_begin(), 3) != single_source.end());
    SSTL_TEST_ASSERT(single_source.insert_after(single_source.before_begin(), 2) != single_source.end());
    sstl::forward_list<int, 4>::iterator target_pos = single_target.begin();
    sstl::forward_list<int, 4>::iterator source_before_moving = single_source.begin();
    SSTL_TEST_ASSERT(single_target.splice_after(target_pos, single_source, source_before_moving));
    {
      const int expected_target[] = {1, 3, 4};
      const int expected_source[] = {2};
      expect_forward_list_values(single_target, expected_target, 3u);
      expect_forward_list_values(single_source, expected_source, 1u);
    }
    sstl::forward_list<int, 4> invalid_position_source;
    SSTL_TEST_ASSERT(invalid_position_source.insert_after(invalid_position_source.before_begin(), 9) != invalid_position_source.end());
    SSTL_TEST_ASSERT(!single_target.splice_after(single_target.end(), invalid_position_source, invalid_position_source.before_begin()));
  }

  {
    sstl::forward_list<int, 4> templated_invalid_target;
    sstl::forward_list<int, 2> templated_invalid_source;
    SSTL_TEST_ASSERT(templated_invalid_source.insert_after(templated_invalid_source.before_begin(), 9) != templated_invalid_source.end());
    SSTL_TEST_ASSERT(!templated_invalid_target.splice_after(templated_invalid_target.end(), templated_invalid_source, templated_invalid_source.before_begin()));
    SSTL_TEST_EQ(templated_invalid_source.size(), 1u);
  }

  {
    sstl::forward_list<int, 4> templated_single_target;
    sstl::forward_list<int, 2> templated_single_source;
    SSTL_TEST_ASSERT(templated_single_target.insert_after(templated_single_target.before_begin(), 0) != templated_single_target.end());
    SSTL_TEST_ASSERT(templated_single_source.insert_after(templated_single_source.before_begin(), 2) != templated_single_source.end());
    SSTL_TEST_ASSERT(templated_single_source.insert_after(templated_single_source.before_begin(), 1) != templated_single_source.end());
    SSTL_TEST_ASSERT(templated_single_target.splice_after(templated_single_target.before_begin(), templated_single_source, templated_single_source.begin()));
    {
      const int expected_target[] = {2, 0};
      const int expected_source[] = {1};
      expect_forward_list_values(templated_single_target, expected_target, 2u);
      expect_forward_list_values(templated_single_source, expected_source, 1u);
    }
    SSTL_TEST_ASSERT(templated_single_target.splice_after(templated_single_target.before_begin(), templated_single_source, templated_single_source.end()));
    SSTL_TEST_EQ(templated_single_source.size(), 1u);
  }

  {
    sstl::forward_list<int, 4> self_move;
    SSTL_TEST_ASSERT(self_move.insert_after(self_move.before_begin(), 3) != self_move.end());
    SSTL_TEST_ASSERT(self_move.insert_after(self_move.before_begin(), 2) != self_move.end());
    SSTL_TEST_ASSERT(self_move.insert_after(self_move.before_begin(), 1) != self_move.end());
    sstl::forward_list<int, 4>::iterator before_second = self_move.begin();
    SSTL_TEST_ASSERT(self_move.splice_after(self_move.before_begin(), self_move, before_second));
    {
      const int expected[] = {2, 1, 3};
      expect_forward_list_values(self_move, expected, 3u);
    }
    SSTL_TEST_ASSERT(self_move.splice_after(self_move.before_begin(), self_move, self_move.end()));
  }

  {
    sstl::forward_list<int, 4> self_noops;
    SSTL_TEST_ASSERT(self_noops.insert_after(self_noops.before_begin(), 3) != self_noops.end());
    SSTL_TEST_ASSERT(self_noops.insert_after(self_noops.before_begin(), 2) != self_noops.end());
    SSTL_TEST_ASSERT(self_noops.insert_after(self_noops.before_begin(), 1) != self_noops.end());
    sstl::forward_list<int, 4>::iterator before_head = self_noops.before_begin();
    sstl::forward_list<int, 4>::iterator head = self_noops.begin();
    SSTL_TEST_ASSERT(self_noops.splice_after(head, self_noops, before_head));
    sstl::forward_list<int, 4>::iterator before_second = self_noops.begin();
    SSTL_TEST_ASSERT(self_noops.splice_after(before_second, self_noops, before_second));
    {
      const int expected[] = {1, 2, 3};
      expect_forward_list_values(self_noops, expected, 3u);
    }
  }

  {
    sstl::forward_list<int, 4> self_move_to_normal_position;
    SSTL_TEST_ASSERT(self_move_to_normal_position.insert_after(self_move_to_normal_position.before_begin(), 3) != self_move_to_normal_position.end());
    SSTL_TEST_ASSERT(self_move_to_normal_position.insert_after(self_move_to_normal_position.before_begin(), 2) != self_move_to_normal_position.end());
    SSTL_TEST_ASSERT(self_move_to_normal_position.insert_after(self_move_to_normal_position.before_begin(), 1) != self_move_to_normal_position.end());
    sstl::forward_list<int, 4>::iterator tail_pos = self_move_to_normal_position.begin();
    ++tail_pos;
    ++tail_pos;
    SSTL_TEST_ASSERT(self_move_to_normal_position.splice_after(tail_pos, self_move_to_normal_position, self_move_to_normal_position.before_begin()));
    {
      const int expected[] = {2, 3, 1};
      expect_forward_list_values(self_move_to_normal_position, expected, 3u);
    }
  }

  {
    sstl::forward_list<int, 2> trivial_sort;
    trivial_sort.unique();
    trivial_sort.sort();
    SSTL_TEST_ASSERT(trivial_sort.insert_after(trivial_sort.before_begin(), 1) != trivial_sort.end());
    trivial_sort.sort();
  }

  {
    sstl::forward_list<int, 4> merge_self;
    SSTL_TEST_ASSERT(merge_self.insert_after(merge_self.before_begin(), 2) != merge_self.end());
    SSTL_TEST_ASSERT(merge_self.insert_after(merge_self.before_begin(), 1) != merge_self.end());
    merge_self.sort();
    SSTL_TEST_ASSERT(merge_self.merge(merge_self));
    {
      const int expected[] = {1, 2};
      expect_forward_list_values(merge_self, expected, 2u);
    }
  }
}

static void map_insert_erase_rebalancing_and_capacity_edges_are_exercised() {
  sstl_test::noalloc_guard guard;

  /*
    These insertion/deletion orders are deliberately different. Red/black erase
    rebalancing is shape-sensitive, so a single neat tree leaves many valid
    sibling-color and rotation cases unvisited.
  */
  {
    sstl::map<int, int, 16> map;
    bool present[16];
    for (unsigned i = 0u; i != 16u; ++i) present[i] = false;
    const int inserts[] = {8, 4, 12, 2, 6, 10, 14, 1, 3, 5, 7, 9, 11, 13, 15};
    const int erases[] = {1, 3, 5, 7, 9, 11, 13, 15, 2, 4, 6, 8, 10, 12, 14};
    insert_map_sequence(map, present, inserts, sizeof(inserts) / sizeof(inserts[0]));
    sstl::pair<sstl::map<int, int, 16>::iterator, bool> duplicate =
      map.insert(sstl::make_pair(8, -1));
    SSTL_TEST_ASSERT(!duplicate.second);
    SSTL_TEST_ASSERT(duplicate.first != map.end());
    SSTL_TEST_EQ(duplicate.first->second, 80);
    erase_map_sequence(map, present, erases, sizeof(erases) / sizeof(erases[0]));
    SSTL_TEST_EQ(map.erase(99), 0u);
    SSTL_TEST_ASSERT(map.erase(map.end()) == map.end());
  }

  {
    sstl::map<int, int, 16> map;
    bool present[16];
    for (unsigned i = 0u; i != 16u; ++i) present[i] = false;
    const int inserts[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    const int erases[] = {6, 7, 4, 5, 2, 3, 10, 11, 8, 9, 1, 12};
    insert_map_sequence(map, present, inserts, sizeof(inserts) / sizeof(inserts[0]));
    erase_map_sequence(map, present, erases, sizeof(erases) / sizeof(erases[0]));
  }

  {
    sstl::map<int, int, 16> map;
    bool present[16];
    for (unsigned i = 0u; i != 16u; ++i) present[i] = false;
    const int inserts[] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4};
    const int erases[] = {10, 11, 14, 15, 12, 13, 6, 7, 8, 9, 4, 5};
    insert_map_sequence(map, present, inserts, sizeof(inserts) / sizeof(inserts[0]));
    erase_map_sequence(map, present, erases, sizeof(erases) / sizeof(erases[0]));
  }

  {
    sstl::map<int, int, 1> one;
    SSTL_TEST_ASSERT(one.insert(sstl::make_pair(1, 10)).second);
    sstl::pair<sstl::map<int, int, 1>::iterator, bool> full =
      one.insert(sstl::make_pair(2, 20));
    SSTL_TEST_ASSERT(!full.second);
    SSTL_TEST_ASSERT(full.first == one.end());
    int& overflow = one[2];
    overflow = 99;
    SSTL_TEST_EQ(one.size(), 1u);
    SSTL_TEST_ASSERT(one.find(2) == one.end());
    SSTL_TEST_EQ(one[1], 10);
  }

  {
    /*
      Ordered maps recycle fixed node slots. This probe keeps an iterator to a
      slot, erases that key, then inserts a new key that may reuse the same
      physical slot. is_valid_iterator() and equality must both reject the stale
      iterator so callers cannot accidentally treat the new key as the old one.
    */
    sstl::map<int, int, 3> reuse;
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(1, 10)).second);
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(2, 20)).second);
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(3, 30)).second);
    sstl::map<int, int, 3>::iterator advanced_end = reuse.end();
    ++advanced_end;
    SSTL_TEST_ASSERT(advanced_end == reuse.end());
    sstl::map<int, int, 3>::iterator last = reuse.end();
    sstl::map<int, int, 3>::iterator old_end = last--;
    SSTL_TEST_ASSERT(old_end == reuse.end());
    SSTL_TEST_EQ(last->first, 3);
    sstl::map<int, int, 3>::iterator predecessor = reuse.find(3);
    sstl::map<int, int, 3>::iterator old_predecessor = predecessor--;
    SSTL_TEST_EQ(old_predecessor->first, 3);
    SSTL_TEST_EQ(predecessor->first, 2);
    sstl::map<int, int, 3>::iterator stale = reuse.find(1);
    SSTL_TEST_EQ(reuse.erase(1), 1u);
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(4, 40)).second);
    SSTL_TEST_ASSERT(!reuse.is_valid_iterator(stale));
    SSTL_TEST_ASSERT(stale != reuse.find(4));
    {
      sstl::map<int, int, 3>::iterator null_iterator;
      ++null_iterator;
      --null_iterator;
      SSTL_TEST_ASSERT(!reuse.is_valid_iterator(null_iterator));
      SSTL_TEST_ASSERT(!reuse.is_valid_iterator(sstl::map<int, int, 3>::iterator(&reuse, 99)));
      SSTL_TEST_ASSERT(!reuse.is_valid_iterator(sstl::map<int, int, 3>::iterator(&reuse, -2)));
    }
    {
      const sstl::map<int, int, 3>& const_reuse = reuse;
      sstl::map<int, int, 3>::const_iterator const_stale = const_reuse.find(2);
      SSTL_TEST_EQ(reuse.erase(2), 1u);
      SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(5, 50)).second);
      SSTL_TEST_ASSERT(!const_reuse.is_valid_iterator(const_stale));
      SSTL_TEST_ASSERT(const_stale != const_reuse.find(5));
      SSTL_TEST_ASSERT(const_reuse.find(99) == const_reuse.end());
      sstl::map<int, int, 3>::const_iterator const_advanced_end = const_reuse.end();
      ++const_advanced_end;
      SSTL_TEST_ASSERT(const_advanced_end == const_reuse.end());
      sstl::map<int, int, 3>::const_iterator const_last = const_reuse.end();
      sstl::map<int, int, 3>::const_iterator const_old_end = const_last--;
      SSTL_TEST_ASSERT(const_old_end == const_reuse.end());
      SSTL_TEST_EQ(const_last->first, 5);
      sstl::map<int, int, 3>::const_iterator const_predecessor = const_reuse.find(5);
      sstl::map<int, int, 3>::const_iterator const_old_predecessor = const_predecessor--;
      SSTL_TEST_EQ(const_old_predecessor->first, 5);
      SSTL_TEST_EQ(const_predecessor->first, 4);
      sstl::map<int, int, 3>::const_iterator null_const_iterator;
      ++null_const_iterator;
      --null_const_iterator;
      SSTL_TEST_ASSERT(!const_reuse.is_valid_iterator(null_const_iterator));
      SSTL_TEST_ASSERT(!const_reuse.is_valid_iterator(sstl::map<int, int, 3>::const_iterator(&const_reuse, 99)));
      SSTL_TEST_ASSERT(!const_reuse.is_valid_iterator(sstl::map<int, int, 3>::const_iterator(&const_reuse, -2)));
    }
  }
}

static void map_permutation_rebalance_matrix_exercises_small_trees() {
  sstl_test::noalloc_guard guard;

  int insert_order[5];
  bool insert_used[6];
  for (unsigned i = 0u; i != 6u; ++i) insert_used[i] = false;
  generate_small_map_insert_orders(insert_order, insert_used, 0u);

  /*
    These two explicit insert orders are the classic left-right and right-left
    insertion rotations. The permutation matrix above also reaches them, but
    keeping the named probes here makes the branch's purpose obvious to a
    future reader scanning failures.
  */
  {
    sstl::map<int, int, 3> lr;
    SSTL_TEST_ASSERT(lr.insert(sstl::make_pair(3, 30)).second);
    SSTL_TEST_ASSERT(lr.insert(sstl::make_pair(1, 10)).second);
    SSTL_TEST_ASSERT(lr.insert(sstl::make_pair(2, 20)).second);
    SSTL_TEST_EQ(lr.begin()->first, 1);
  }
  {
    sstl::map<int, int, 3> rl;
    SSTL_TEST_ASSERT(rl.insert(sstl::make_pair(1, 10)).second);
    SSTL_TEST_ASSERT(rl.insert(sstl::make_pair(3, 30)).second);
    SSTL_TEST_ASSERT(rl.insert(sstl::make_pair(2, 20)).second);
    SSTL_TEST_EQ(rl.begin()->first, 1);
  }
}

static void flat_ordered_and_unordered_sets_report_end_on_missing_or_full() {
  sstl_test::noalloc_guard guard;

  sstl::set<int, 1> ordered;
  SSTL_TEST_ASSERT(ordered.insert(1).second);
  SSTL_TEST_ASSERT(!ordered.insert(1).second);
  SSTL_TEST_ASSERT(!ordered.insert(2).second);
  SSTL_TEST_ASSERT(ordered.find(1) != ordered.end());
  SSTL_TEST_ASSERT(ordered.find(9) == ordered.end());
  SSTL_TEST_EQ(ordered.erase(9), 0u);
  {
    sstl::set<int, 1>::iterator it = ordered.end();
    sstl::set<int, 1>::iterator old = it--;
    SSTL_TEST_ASSERT(old == ordered.end());
    SSTL_TEST_EQ(*it, 1);
    const sstl::set<int, 1>& const_ordered = ordered;
    sstl::set<int, 1>::const_iterator const_it = const_ordered.end();
    sstl::set<int, 1>::const_iterator const_old = const_it--;
    SSTL_TEST_ASSERT(const_old == const_ordered.end());
    SSTL_TEST_EQ(*const_it, 1);
  }
#if SSTL_ON_ERROR == SSTL_RETURN
  /*
    Invalid iterator erase is a policy-routed public outcome in RETURN mode.
    Panic-mode builds intentionally skip this probe because the same call would
    correctly enter the application panic hook.
  */
  SSTL_TEST_ASSERT(ordered.erase(ordered.end()) == ordered.end());
#endif

  sstl::set<int, 0> zero_ordered;
  SSTL_TEST_ASSERT(!zero_ordered.insert(1).second);
  SSTL_TEST_ASSERT(zero_ordered.find(1) == zero_ordered.end());

  sstl::flat_set<int, 1> flat;
  SSTL_TEST_ASSERT(flat.insert(1).second);
  SSTL_TEST_ASSERT(!flat.insert(1).second);
  SSTL_TEST_ASSERT(!flat.insert(2).second);
  SSTL_TEST_ASSERT(flat.find(1) != flat.end());
  SSTL_TEST_ASSERT(flat.find(9) == flat.end());
  SSTL_TEST_EQ(flat.erase(9), 0u);
#if SSTL_ON_ERROR == SSTL_RETURN
  /*
    The flat_set wrapper has its own iterator guard before it delegates into
    flat_map. This keeps that wrapper branch visible to coverage without
    relying on undefined iterator ownership.
  */
  SSTL_TEST_ASSERT(flat.erase(flat.end()) == flat.end());
#endif

  sstl::unordered_set<int, 1, 3> unordered;
  SSTL_TEST_ASSERT(unordered.insert(1).second);
  SSTL_TEST_ASSERT(!unordered.insert(1).second);
  SSTL_TEST_ASSERT(!unordered.insert(2).second);
  SSTL_TEST_ASSERT(unordered.find(1) != unordered.end());
  SSTL_TEST_ASSERT(unordered.find(9) == unordered.end());
  SSTL_TEST_EQ(unordered.bucket_count(), 3u);
  SSTL_TEST_ASSERT(unordered.full());
  SSTL_TEST_EQ(unordered.erase(9), 0u);
#if SSTL_ON_ERROR == SSTL_RETURN
  /*
    unordered_set validates both the wrapped iterator epoch and the end
    sentinel. Separate same-container and foreign-container probes keep both
    sides of the short-circuit guard observable in RETURN-policy coverage.
  */
  SSTL_TEST_ASSERT(unordered.erase(unordered.end()) == unordered.end());
  {
    sstl::unordered_set<int, 1, 3> other_unordered;
    SSTL_TEST_ASSERT(unordered.erase(other_unordered.end()) == unordered.end());
  }
#endif

  sstl::flat_set<int, 0> zero_flat;
  SSTL_TEST_ASSERT(!zero_flat.insert(1).second);
  SSTL_TEST_ASSERT(zero_flat.find(1) == zero_flat.end());

  sstl::unordered_set<int, 1, 0> zero_bucket_unordered;
  SSTL_TEST_ASSERT(!zero_bucket_unordered.insert(1).second);
  SSTL_TEST_ASSERT(zero_bucket_unordered.find(1) == zero_bucket_unordered.end());
  SSTL_TEST_EQ(zero_bucket_unordered.bucket_count(), 0u);
}

static void flat_and_hash_maps_exercise_direct_contracts() {
  sstl_test::noalloc_guard guard;

  /*
    Set wrappers cover the public set surface, but they hide several map-only
    branches: sorted insertion positions, operator[], iterator validity epochs,
    hash collisions, zero-bucket configurations, and clear() reset behavior.
  */
  sstl::flat_map<int, int, 3> bracket_flat;
  bracket_flat[5] = 50;
  SSTL_TEST_EQ(bracket_flat.find(5)->second, 50);
  {
    sstl::flat_map<int, int, 1> full_subscript;
    full_subscript[1] = 10;
    int& overflow = full_subscript[2];
    overflow = 99;
    SSTL_TEST_EQ(full_subscript.size(), 1u);
    SSTL_TEST_ASSERT(full_subscript.find(2) == full_subscript.end());
    SSTL_TEST_EQ(full_subscript[1], 10);
  }

  sstl::flat_map<int, int, 3> flat;
  SSTL_TEST_ASSERT(flat.empty());
  SSTL_TEST_ASSERT(flat.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(!flat.insert(sstl::make_pair(2, 99)).second);
  SSTL_TEST_ASSERT(flat.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_EQ(flat.begin()->first, 1);
  SSTL_TEST_EQ(flat[2], 20);
  flat[2] = 22;
  SSTL_TEST_EQ(flat.find(2)->second, 22);
  {
    sstl::flat_map<int, int, 3>::iterator mutable_post = flat.begin();
    sstl::flat_map<int, int, 3>::iterator mutable_before = mutable_post++;
    SSTL_TEST_EQ(mutable_before->first, 1);
    SSTL_TEST_EQ(mutable_post->first, 2);
  }
  flat.revalidate_iterators();
  sstl::flat_map<int, int, 3>::iterator old_begin = flat.begin();
  SSTL_TEST_ASSERT(flat.is_valid_iterator(old_begin));
  SSTL_TEST_ASSERT(flat.insert(sstl::make_pair(3, 30)).second);
  SSTL_TEST_ASSERT(!flat.is_valid_iterator(old_begin));
  SSTL_TEST_ASSERT(flat.is_valid_iterator(flat.begin()));
  SSTL_TEST_ASSERT(flat.is_valid_iterator(flat.end()));
  {
    sstl::flat_map<int, int, 3>::iterator null_iterator;
    sstl::flat_map<int, int, 3>::iterator beyond_end = flat.end();
    ++beyond_end;
    SSTL_TEST_ASSERT(!flat.is_valid_iterator(null_iterator));
    SSTL_TEST_ASSERT(!flat.is_valid_iterator(beyond_end));
  }
  SSTL_TEST_ASSERT(!flat.insert(sstl::make_pair(4, 40)).second);
  const sstl::flat_map<int, int, 3>& const_flat = flat;
  SSTL_TEST_ASSERT(const_flat.find(3) != const_flat.end());
  SSTL_TEST_ASSERT(const_flat.find(0) == const_flat.end());
  SSTL_TEST_ASSERT(const_flat.find(9) == const_flat.end());
  SSTL_TEST_ASSERT(const_flat.begin() != const_flat.end());
  SSTL_TEST_ASSERT(const_flat.begin() == const_flat.find(1));
  {
    sstl::flat_map<int, int, 3>::const_iterator const_post = const_flat.begin();
    sstl::flat_map<int, int, 3>::const_iterator const_before = const_post++;
    SSTL_TEST_EQ(const_before->first, 1);
    SSTL_TEST_EQ(const_post->first, 2);
  }
  SSTL_TEST_ASSERT(const_flat.is_valid_iterator(const_flat.begin()));
  SSTL_TEST_ASSERT(const_flat.is_valid_iterator(const_flat.end()));
  {
    sstl::flat_map<int, int, 3>::const_iterator null_const_iterator;
    sstl::flat_map<int, int, 3>::const_iterator beyond_const_end = const_flat.end();
    ++beyond_const_end;
    SSTL_TEST_ASSERT(!const_flat.is_valid_iterator(null_const_iterator));
    SSTL_TEST_ASSERT(!const_flat.is_valid_iterator(beyond_const_end));
  }
  SSTL_TEST_EQ(const_flat.lower_bound(0)->first, 1);
  SSTL_TEST_ASSERT(const_flat.upper_bound(3) == const_flat.end());
  SSTL_TEST_ASSERT(const_flat.equal_range(9).first == const_flat.end());
  SSTL_TEST_EQ(flat.erase(9), 0u);
#if SSTL_ON_ERROR == SSTL_RETURN
  /*
    A stale generation and the end sentinel both route through the public
    invalid-iterator policy branch. Fresh iterators above remain valid, proving
    that the generation model invalidates only old iterators.
  */
  SSTL_TEST_ASSERT(flat.erase(old_begin) == flat.end());
  SSTL_TEST_ASSERT(flat.erase(flat.end()) == flat.end());
#endif

  sstl::unordered_map<int, int, 3, 1, constant_hash_int> hashed;
  SSTL_TEST_ASSERT(hashed.begin() == hashed.end());
  SSTL_TEST_ASSERT(hashed.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(hashed.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(hashed.insert(sstl::make_pair(3, 30)).second);
  SSTL_TEST_ASSERT(!hashed.insert(sstl::make_pair(2, 99)).second);
  SSTL_TEST_ASSERT(!hashed.insert(sstl::make_pair(4, 40)).second);
  SSTL_TEST_ASSERT(hashed.find(1) != hashed.end());
  SSTL_TEST_ASSERT(hashed.find(99) == hashed.end());
  SSTL_TEST_ASSERT(hashed.load_factor() > 2.9f);
  {
    const sstl::unordered_map<int, int, 3, 1, constant_hash_int>& const_hashed = hashed;
    SSTL_TEST_ASSERT(const_hashed.begin() != const_hashed.end());
    SSTL_TEST_ASSERT(const_hashed.begin() == const_hashed.find(1));
    SSTL_TEST_ASSERT(const_hashed.find(99) == const_hashed.end());
    SSTL_TEST_ASSERT(hashed.is_valid_iterator(hashed.begin()));
    SSTL_TEST_ASSERT(hashed.is_valid_iterator(hashed.end()));
    SSTL_TEST_ASSERT(const_hashed.is_valid_iterator(const_hashed.begin()));
    SSTL_TEST_ASSERT(const_hashed.is_valid_iterator(const_hashed.end()));
  }
  {
    /*
      All keys collide through constant_hash_int. Erasing key 2 removes a
      middle bucket-chain node, while erasing key 3 removes the current head.
      Those two cases exercise both unlink directions without heap allocation.
    */
#if SSTL_ON_ERROR == SSTL_RETURN
    sstl::unordered_map<int, int, 3, 1, constant_hash_int>::iterator stale = hashed.find(1);
#endif
    sstl::unordered_map<int, int, 3, 1, constant_hash_int>::iterator after_middle = hashed.erase(hashed.find(2));
    SSTL_TEST_ASSERT(after_middle != hashed.end());
    SSTL_TEST_EQ(after_middle->first, 3);
    SSTL_TEST_ASSERT(hashed.find(2) == hashed.end());
    SSTL_TEST_EQ(hashed.erase(3), 1u);
    SSTL_TEST_EQ(hashed.erase(99), 0u);
    SSTL_TEST_EQ(hashed.erase(1), 1u);
#if SSTL_ON_ERROR == SSTL_RETURN
    SSTL_TEST_ASSERT(hashed.erase(stale) == hashed.end());
    SSTL_TEST_ASSERT(hashed.erase(hashed.end()) == hashed.end());
    {
      sstl::unordered_map<int, int, 3, 1, constant_hash_int> other_hashed;
      SSTL_TEST_ASSERT(!hashed.is_valid_iterator(other_hashed.end()));
      SSTL_TEST_ASSERT(hashed.erase(other_hashed.end()) == hashed.end());
    }
#endif
  }
  hashed.clear();
  SSTL_TEST_ASSERT(hashed.empty());
  SSTL_TEST_ASSERT(hashed.begin() == hashed.end());

  {
    sstl::unordered_map<int, int, 3, 5> subscripted;
    subscripted[4] = 40;
    SSTL_TEST_ASSERT(subscripted.find(4) != subscripted.end());
    SSTL_TEST_EQ(subscripted.find(4)->second, 40);
    SSTL_TEST_EQ(subscripted[4], 40);
    SSTL_TEST_EQ(subscripted[5], 0);
    subscripted[5] = 50;
    SSTL_TEST_EQ(subscripted.find(5)->second, 50);
  }

  {
    sstl::unordered_map<int, int, 1, 3> full_subscript;
    full_subscript[1] = 10;
    int& overflow = full_subscript[2];
    overflow = 99;
    SSTL_TEST_EQ(full_subscript.size(), 1u);
    SSTL_TEST_ASSERT(full_subscript.find(2) == full_subscript.end());
    SSTL_TEST_EQ(full_subscript[1], 10);
  }

  {
    /*
      The unordered-map node pool is free-list backed. Erasing slots 0 and 1
      then inserting two new keys should reuse the most recently released slot
      first. Iteration walks physical slots, so the observable order distinguishes
      LIFO free-list reuse from a fresh linear scan.
    */
    sstl::unordered_map<int, int, 3, 5> reuse;
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(1, 10)).second);
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(2, 20)).second);
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(3, 30)).second);
    sstl::unordered_map<int, int, 3, 5>::iterator stale_reuse = reuse.find(1);
    SSTL_TEST_EQ(reuse.erase(1), 1u);
    SSTL_TEST_EQ(reuse.erase(2), 1u);
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(4, 40)).second);
    SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(5, 50)).second);
    SSTL_TEST_ASSERT(!reuse.is_valid_iterator(stale_reuse));
    SSTL_TEST_ASSERT(stale_reuse != reuse.find(5));
    sstl::unordered_map<int, int, 3, 5>::iterator reuse_it = reuse.begin();
    SSTL_TEST_ASSERT(reuse_it != reuse.end());
    SSTL_TEST_EQ(reuse_it->first, 5);
    ++reuse_it;
    SSTL_TEST_ASSERT(reuse_it != reuse.end());
    SSTL_TEST_EQ(reuse_it->first, 4);
    ++reuse_it;
    SSTL_TEST_ASSERT(reuse_it != reuse.end());
    SSTL_TEST_EQ(reuse_it->first, 3);
    {
      sstl::unordered_map<int, int, 3, 5>::iterator null_iterator;
      ++null_iterator;
      SSTL_TEST_ASSERT(!reuse.is_valid_iterator(null_iterator));
      SSTL_TEST_ASSERT(!reuse.is_valid_iterator(sstl::unordered_map<int, int, 3, 5>::iterator(&reuse, 4u)));
    }
    {
      const sstl::unordered_map<int, int, 3, 5>& const_reuse = reuse;
      sstl::unordered_map<int, int, 3, 5>::const_iterator const_stale = const_reuse.find(3);
      SSTL_TEST_EQ(reuse.erase(3), 1u);
      SSTL_TEST_ASSERT(reuse.insert(sstl::make_pair(6, 60)).second);
      SSTL_TEST_ASSERT(!const_reuse.is_valid_iterator(const_stale));
      SSTL_TEST_ASSERT(const_stale != const_reuse.find(6));
      sstl::unordered_map<int, int, 3, 5>::const_iterator null_const_iterator;
      ++null_const_iterator;
      SSTL_TEST_ASSERT(!const_reuse.is_valid_iterator(null_const_iterator));
      SSTL_TEST_ASSERT(!const_reuse.is_valid_iterator(sstl::unordered_map<int, int, 3, 5>::const_iterator(&const_reuse, 4u)));
    }
  }

  sstl::unordered_map<int, int, 1, 0> zero_bucket;
  SSTL_TEST_EQ(zero_bucket.bucket_count(), 0u);
  SSTL_TEST_ASSERT(zero_bucket.load_factor() == 0.0f);
  SSTL_TEST_ASSERT(zero_bucket.find(1) == zero_bucket.end());
  SSTL_TEST_ASSERT(!zero_bucket.insert(sstl::make_pair(1, 10)).second);
}

static void vector_and_deque_return_policy_edges_are_exercised() {
  sstl_test::noalloc_guard guard;

  sstl::vector<int, 2> values;
  SSTL_TEST_ASSERT(!values.try_pop_back(0));
  values.pop_back();
  values = values;
  SSTL_TEST_ASSERT(values.push_back(1));
  SSTL_TEST_ASSERT(values.push_back(2));
  SSTL_TEST_EQ(values.at(0), 1);
  values.at(0) = 3;
  SSTL_TEST_EQ(values.front(), 3);
  SSTL_TEST_EQ(values.back(), 2);
  SSTL_TEST_ASSERT(values.insert(values.begin(), 9) == values.end());
  SSTL_TEST_ASSERT(values.erase(values.end()) == values.end());
  SSTL_TEST_ASSERT(values.try_pop_back(0));
  {
    sstl::vector<int, 4> inserts;
    int one_range[1] = {3};
    int empty_range[1] = {0};
    SSTL_TEST_ASSERT(inserts.push_back(1));
    SSTL_TEST_ASSERT(inserts.push_back(4));
    sstl::vector<int, 4>::iterator count_insert = inserts.try_insert(inserts.begin() + 1, 1u, 2);
    SSTL_TEST_ASSERT(count_insert != inserts.end());
    SSTL_TEST_EQ(inserts[0], 1);
    SSTL_TEST_EQ(inserts[1], 2);
    SSTL_TEST_EQ(inserts[2], 4);
    SSTL_TEST_ASSERT(inserts.try_insert(inserts.begin(), empty_range, empty_range) == inserts.begin());
    sstl::vector<int, 4>::iterator range_insert = inserts.try_insert(inserts.begin() + 2, one_range, one_range + 1);
    SSTL_TEST_ASSERT(range_insert != inserts.end());
    SSTL_TEST_EQ(inserts[0], 1);
    SSTL_TEST_EQ(inserts[1], 2);
    SSTL_TEST_EQ(inserts[2], 3);
    SSTL_TEST_EQ(inserts[3], 4);
    SSTL_TEST_ASSERT(inserts.try_insert(inserts.begin(), 1u, 9) == inserts.end());
    SSTL_TEST_ASSERT(inserts.erase(inserts.begin() + 1, inserts.begin() + 3) == inserts.begin() + 1);
    SSTL_TEST_EQ(inserts.size(), 2u);
    SSTL_TEST_EQ(inserts[0], 1);
    SSTL_TEST_EQ(inserts[1], 4);
    SSTL_TEST_ASSERT(inserts.erase(inserts.begin(), inserts.begin()) == inserts.begin());
  }
  {
    sstl::vector<int, 4> resized;
    SSTL_TEST_ASSERT(resized.resize(3u, 7));
    SSTL_TEST_EQ(resized.size(), 3u);
    SSTL_TEST_EQ(resized[0], 7);
    SSTL_TEST_EQ(resized[2], 7);
    SSTL_TEST_ASSERT(resized.resize(1u, 9));
    SSTL_TEST_EQ(resized.size(), 1u);
    SSTL_TEST_EQ(resized[0], 7);
    SSTL_TEST_ASSERT(!resized.resize(5u, 1));
  }
  {
    sstl::vector<int, 2> tail_insert;
    int one_range[1] = {8};
    SSTL_TEST_ASSERT(tail_insert.push_back(7));
    sstl::vector<int, 2>::iterator tail_result = tail_insert.try_insert(tail_insert.end(), one_range, one_range + 1);
    SSTL_TEST_ASSERT(tail_result != tail_insert.end());
    SSTL_TEST_EQ(tail_insert[0], 7);
    SSTL_TEST_EQ(tail_insert[1], 8);
  }
  {
    sstl::vector<int, 2> left;
    sstl::vector<int, 2> same_left;
    sstl::vector<int, 2> different_value;
    sstl::vector<int, 1> different_size;
    SSTL_TEST_ASSERT(left.push_back(5));
    SSTL_TEST_ASSERT(left.push_back(6));
    SSTL_TEST_ASSERT(same_left.push_back(5));
    SSTL_TEST_ASSERT(same_left.push_back(6));
    SSTL_TEST_ASSERT(different_value.push_back(5));
    SSTL_TEST_ASSERT(different_value.push_back(7));
    SSTL_TEST_ASSERT(different_size.push_back(5));
    SSTL_TEST_ASSERT(left == same_left);
    SSTL_TEST_ASSERT(!(left == different_value));
    SSTL_TEST_ASSERT(!(left == different_size));
  }
  sstl::vector<int, 2> other;
  SSTL_TEST_ASSERT(other.push_back(7));
  values.swap(other);
  SSTL_TEST_EQ(values.size(), 1u);
  SSTL_TEST_EQ(values[0], 7);
  {
    const sstl::vector<int, 2>& const_values = values;
    SSTL_TEST_EQ(const_values.at(0), 7);
    SSTL_TEST_EQ(const_values.front(), 7);
    SSTL_TEST_EQ(const_values.back(), 7);
    SSTL_TEST_ASSERT(const_values.try_at(0) != 0);
    SSTL_TEST_ASSERT(const_values.try_at(99u) == 0);
  }

  sstl::deque<int, 2> q;
  q.pop_back();
  q.pop_front();
  SSTL_TEST_ASSERT(q.push_back(1));
  sstl::deque<int, 2>::iterator old_begin = q.begin();
  SSTL_TEST_ASSERT(q.push_front(0));
  SSTL_TEST_ASSERT(!q.is_valid_iterator(old_begin));
  SSTL_TEST_ASSERT(q.is_valid_iterator(q.begin()));
  SSTL_TEST_ASSERT(!q.push_front(9));
  {
    const sstl::deque<int, 2>& const_q = q;
    sstl::deque<int, 2>::const_iterator const_old_begin = const_q.begin();
    SSTL_TEST_ASSERT(const_q.front() == 0);
    SSTL_TEST_ASSERT(const_q.back() == 1);
    SSTL_TEST_ASSERT(const_q.is_valid_iterator(const_old_begin));
  }
  {
    int out = -1;
    SSTL_TEST_ASSERT(q.try_pop_back(&out));
    SSTL_TEST_EQ(out, 1);
    SSTL_TEST_ASSERT(q.try_pop_front(&out));
    SSTL_TEST_EQ(out, 0);
  }
  SSTL_TEST_ASSERT(!q.try_pop_front(0));
  {
    sstl::deque<int, 2> null_out_q;
    SSTL_TEST_ASSERT(null_out_q.push_back(1));
    SSTL_TEST_ASSERT(null_out_q.try_pop_back(0));
    SSTL_TEST_ASSERT(null_out_q.push_back(2));
    SSTL_TEST_ASSERT(null_out_q.try_pop_front(0));
  }
  {
    sstl::deque<int, 2> other_q;
    SSTL_TEST_ASSERT(!q.is_valid_iterator(other_q.begin()));
  }
  {
    sstl::deque<int, 2> const_epoch_q;
    SSTL_TEST_ASSERT(const_epoch_q.push_back(1));
    const sstl::deque<int, 2>& const_epoch_view = const_epoch_q;
    sstl::deque<int, 2>::const_iterator stale_const = const_epoch_view.begin();
    SSTL_TEST_ASSERT(const_epoch_q.try_pop_back(0));
    SSTL_TEST_ASSERT(!const_epoch_view.is_valid_iterator(stale_const));
    const sstl::deque<int, 2>& other_const_view = q;
    SSTL_TEST_ASSERT(!other_const_view.is_valid_iterator(const_epoch_view.begin()));
  }
  {
    sstl::deque<int, 6> editable;
    SSTL_TEST_ASSERT(editable.push_back(1));
    SSTL_TEST_ASSERT(editable.push_back(3));
    SSTL_TEST_ASSERT(editable.push_back(4));
    SSTL_TEST_ASSERT(editable.insert(editable.begin() + 1, 2) != editable.end());
    SSTL_TEST_ASSERT(editable.insert(editable.begin(), 0) == editable.begin());
    sstl::deque<int, 6>::iterator appended = editable.insert(editable.end(), 5);
    SSTL_TEST_ASSERT(appended != editable.end());
    SSTL_TEST_EQ(editable[0], 0);
    SSTL_TEST_EQ(editable[2], 2);
    SSTL_TEST_EQ(editable[4], 4);
    SSTL_TEST_ASSERT(editable.insert(editable.begin(), 99) == editable.end());
    SSTL_TEST_ASSERT(editable.erase(editable.begin() + 2) != editable.end());
    SSTL_TEST_EQ(editable[2], 3);
    SSTL_TEST_ASSERT(editable.erase(editable.begin()) == editable.begin());
    SSTL_TEST_EQ(editable[0], 1);
    sstl::deque<int, 6>::iterator after_tail_erase = editable.erase(editable.end() - 1);
    SSTL_TEST_ASSERT(after_tail_erase == editable.end());
    SSTL_TEST_ASSERT(editable.resize(2u));
    SSTL_TEST_EQ(editable.size(), 2u);
    SSTL_TEST_ASSERT(editable.resize(3u));
    SSTL_TEST_EQ(editable.size(), 3u);
    SSTL_TEST_ASSERT(editable.resize(4u, 8));
    SSTL_TEST_EQ(editable[2], 0);
    SSTL_TEST_EQ(editable[3], 8);
    SSTL_TEST_ASSERT(editable.resize(1u, 9));
    SSTL_TEST_EQ(editable.size(), 1u);
    SSTL_TEST_ASSERT(!editable.resize(7u));
    SSTL_TEST_ASSERT(!editable.resize(7u, 1));
    SSTL_TEST_ASSERT(editable.erase(editable.end()) == editable.end());
  }
}

static void list_splice_merge_sort_and_empty_edges_are_exercised() {
  sstl_test::noalloc_guard guard;

  /*
    These list checks mirror the forward_list edge coverage, but the doubly
    linked implementation has its own splice and merge branches. We cover both
    whole-list transfers and single-node transfers, including cross-capacity
    overloads that are distinct template instantiations.
  */
  sstl::list<int, 2> small;
  SSTL_TEST_ASSERT(!small.try_pop_front(0));
  SSTL_TEST_ASSERT(!small.try_pop_back(0));
  SSTL_TEST_ASSERT(small.erase(small.end()) == small.end());
  SSTL_TEST_ASSERT(small.push_front(2));
  SSTL_TEST_ASSERT(small.push_back(3));
  SSTL_TEST_ASSERT(!small.push_front(1));
  SSTL_TEST_ASSERT(small.insert(small.begin(), 1) == small.end());
  SSTL_TEST_ASSERT(small.try_pop_front(0));
  SSTL_TEST_ASSERT(small.try_pop_back(0));
  SSTL_TEST_ASSERT(small.empty());
  SSTL_TEST_ASSERT(small.try_insert(small.end(), 4) != small.end());
  SSTL_TEST_ASSERT(small.try_pop_back(0));
  {
    sstl::list<int, 2>::iterator mutable_end = small.end();
    ++mutable_end;
    SSTL_TEST_ASSERT(mutable_end == small.end());
    sstl::list<int, 2>::iterator mutable_before_end = small.end();
    --mutable_before_end;
    SSTL_TEST_ASSERT(mutable_before_end == small.end());
    const sstl::list<int, 2>& const_small = small;
    sstl::list<int, 2>::const_iterator const_end = const_small.end();
    ++const_end;
    SSTL_TEST_ASSERT(const_end == const_small.end());
  }

  sstl::list<int, 8> same_dst;
  sstl::list<int, 8> same_src;
  SSTL_TEST_ASSERT(same_dst.push_back(1));
  SSTL_TEST_ASSERT(same_src.push_back(2));
  SSTL_TEST_ASSERT(same_src.push_back(3));
  SSTL_TEST_ASSERT(same_dst.splice(same_dst.end(), same_src));
  SSTL_TEST_ASSERT(same_src.empty());
  {
    const int expected[] = {1, 2, 3};
    expect_list_values(same_dst, expected, 3u);
  }
  {
    sstl::list<int, 8> empty_same_source;
    SSTL_TEST_ASSERT(same_dst.splice(same_dst.end(), empty_same_source));
  }
  {
    sstl::list<int, 2> full_same_target;
    sstl::list<int, 2> same_capacity_source;
    SSTL_TEST_ASSERT(full_same_target.push_back(1));
    SSTL_TEST_ASSERT(full_same_target.push_back(2));
    SSTL_TEST_ASSERT(same_capacity_source.push_back(3));
    SSTL_TEST_ASSERT(!full_same_target.splice(full_same_target.end(), same_capacity_source));
    SSTL_TEST_EQ(same_capacity_source.size(), 1u);
  }

  sstl::list<int, 8> templated_dst;
  sstl::list<int, 2> templated_src;
  SSTL_TEST_ASSERT(templated_dst.push_back(1));
  SSTL_TEST_ASSERT(templated_src.push_back(4));
  SSTL_TEST_ASSERT(templated_src.push_front(2));
  SSTL_TEST_ASSERT(templated_dst.splice(templated_dst.end(), templated_src));
  SSTL_TEST_ASSERT(templated_src.empty());
  {
    const int expected[] = {1, 2, 4};
    expect_list_values(templated_dst, expected, 3u);
  }
  {
    sstl::list<int, 1> full_templated_target;
    sstl::list<int, 2> templated_capacity_source;
    SSTL_TEST_ASSERT(full_templated_target.push_back(1));
    SSTL_TEST_ASSERT(templated_capacity_source.push_back(2));
    SSTL_TEST_ASSERT(!full_templated_target.splice(full_templated_target.end(), templated_capacity_source));
    SSTL_TEST_EQ(templated_capacity_source.size(), 1u);
  }

  {
    sstl::list<int, 8> range_dst;
    sstl::list<int, 8> range_src;
    SSTL_TEST_ASSERT(range_dst.push_back(1));
    SSTL_TEST_ASSERT(range_dst.push_back(5));
    SSTL_TEST_ASSERT(range_src.push_back(2));
    SSTL_TEST_ASSERT(range_src.push_back(3));
    SSTL_TEST_ASSERT(range_src.push_back(4));
    sstl::list<int, 8>::iterator before_five = range_dst.begin();
    ++before_five;
    SSTL_TEST_ASSERT(range_dst.splice(before_five, range_src, range_src.begin(), range_src.end()));
    SSTL_TEST_ASSERT(range_src.empty());
    {
      const int expected[] = {1, 2, 3, 4, 5};
      expect_list_values(range_dst, expected, 5u);
    }
  }

  {
    sstl::list<int, 8> same_range;
    SSTL_TEST_ASSERT(same_range.push_back(1));
    SSTL_TEST_ASSERT(same_range.push_back(2));
    SSTL_TEST_ASSERT(same_range.push_back(3));
    SSTL_TEST_ASSERT(same_range.push_back(4));
    SSTL_TEST_ASSERT(same_range.push_back(5));
    sstl::list<int, 8>::iterator first = same_range.begin();
    ++first;
    sstl::list<int, 8>::iterator last = first;
    ++last;
    ++last;
    SSTL_TEST_ASSERT(same_range.splice(first, same_range, first, last));
    SSTL_TEST_ASSERT(same_range.splice(same_range.end(), same_range, first, last));
    {
      const int expected[] = {1, 4, 5, 2, 3};
      expect_list_values(same_range, expected, 5u);
    }
  }

  {
    sstl::list<int, 8> head_range_to_tail;
    SSTL_TEST_ASSERT(head_range_to_tail.push_back(1));
    SSTL_TEST_ASSERT(head_range_to_tail.push_back(2));
    SSTL_TEST_ASSERT(head_range_to_tail.push_back(3));
    SSTL_TEST_ASSERT(head_range_to_tail.push_back(4));
    sstl::list<int, 8>::iterator first = head_range_to_tail.begin();
    sstl::list<int, 8>::iterator last = first;
    ++last;
    ++last;
    SSTL_TEST_ASSERT(head_range_to_tail.splice(head_range_to_tail.end(),
                                              head_range_to_tail,
                                              first,
                                              last));
    {
      const int expected[] = {3, 4, 1, 2};
      expect_list_values(head_range_to_tail, expected, 4u);
    }
  }

  {
    sstl::list<int, 8> tail_range_to_head;
    SSTL_TEST_ASSERT(tail_range_to_head.push_back(1));
    SSTL_TEST_ASSERT(tail_range_to_head.push_back(2));
    SSTL_TEST_ASSERT(tail_range_to_head.push_back(3));
    SSTL_TEST_ASSERT(tail_range_to_head.push_back(4));
    sstl::list<int, 8>::iterator first = tail_range_to_head.begin();
    ++first;
    ++first;
    SSTL_TEST_ASSERT(tail_range_to_head.splice(tail_range_to_head.begin(),
                                              tail_range_to_head,
                                              first,
                                              tail_range_to_head.end()));
    {
      const int expected[] = {3, 4, 1, 2};
      expect_list_values(tail_range_to_head, expected, 4u);
    }
  }

  {
    sstl::list<int, 8> middle_range_before_middle;
    SSTL_TEST_ASSERT(middle_range_before_middle.push_back(1));
    SSTL_TEST_ASSERT(middle_range_before_middle.push_back(2));
    SSTL_TEST_ASSERT(middle_range_before_middle.push_back(3));
    SSTL_TEST_ASSERT(middle_range_before_middle.push_back(4));
    SSTL_TEST_ASSERT(middle_range_before_middle.push_back(5));
    sstl::list<int, 8>::iterator pos = middle_range_before_middle.begin();
    ++pos;
    sstl::list<int, 8>::iterator first = pos;
    ++first;
    ++first;
    sstl::list<int, 8>::iterator last = first;
    ++last;
    SSTL_TEST_ASSERT(middle_range_before_middle.splice(pos,
                                                       middle_range_before_middle,
                                                       first,
                                                       last));
    {
      const int expected[] = {1, 4, 2, 3, 5};
      expect_list_values(middle_range_before_middle, expected, 5u);
    }
  }

  {
    sstl::list<int, 8> templated_range_dst;
    sstl::list<int, 3> templated_range_src;
    SSTL_TEST_ASSERT(templated_range_dst.push_back(1));
    SSTL_TEST_ASSERT(templated_range_src.push_back(6));
    SSTL_TEST_ASSERT(templated_range_src.push_back(7));
    SSTL_TEST_ASSERT(templated_range_src.push_back(8));
    SSTL_TEST_ASSERT(templated_range_dst.splice(templated_range_dst.end(),
                                               templated_range_src,
                                               templated_range_src.begin(),
                                               templated_range_src.end()));
    SSTL_TEST_ASSERT(templated_range_src.empty());
    {
      const int expected[] = {1, 6, 7, 8};
      expect_list_values(templated_range_dst, expected, 4u);
    }
  }

  {
    sstl::list<int, 2> full_range_target;
    sstl::list<int, 4> range_capacity_source;
    SSTL_TEST_ASSERT(full_range_target.push_back(1));
    SSTL_TEST_ASSERT(full_range_target.push_back(2));
    SSTL_TEST_ASSERT(range_capacity_source.push_back(3));
    SSTL_TEST_ASSERT(range_capacity_source.push_back(4));
    SSTL_TEST_ASSERT(!full_range_target.splice(full_range_target.end(),
                                               range_capacity_source,
                                               range_capacity_source.begin(),
                                               range_capacity_source.end()));
    SSTL_TEST_EQ(range_capacity_source.size(), 2u);
  }

  sstl::list<int, 4> single_dst;
  sstl::list<int, 4> single_src;
  SSTL_TEST_ASSERT(single_dst.push_back(1));
  SSTL_TEST_ASSERT(single_src.push_back(9));
  SSTL_TEST_ASSERT(single_dst.splice(single_dst.end(), single_src, single_src.begin()));
  SSTL_TEST_ASSERT(single_src.empty());
  {
    const int expected[] = {1, 9};
    expect_list_values(single_dst, expected, 2u);
  }
  SSTL_TEST_ASSERT(single_dst.splice(single_dst.begin(), single_dst, single_dst.begin()));
  {
    sstl::list<int, 4> same_single_noops;
    SSTL_TEST_ASSERT(same_single_noops.push_back(1));
    SSTL_TEST_ASSERT(same_single_noops.push_back(2));
    sstl::list<int, 4>::iterator first = same_single_noops.begin();
    sstl::list<int, 4>::iterator second = first;
    ++second;
    SSTL_TEST_ASSERT(same_single_noops.splice(first, same_single_noops, same_single_noops.end()));
    SSTL_TEST_ASSERT(same_single_noops.splice(first, same_single_noops, first));
    SSTL_TEST_ASSERT(same_single_noops.splice(second, same_single_noops, first));
    {
      const int expected[] = {1, 2};
      expect_list_values(same_single_noops, expected, 2u);
    }
  }

  {
    sstl::list<int, 4> relink_self;
    SSTL_TEST_ASSERT(relink_self.push_back(1));
    SSTL_TEST_ASSERT(relink_self.push_back(2));
    SSTL_TEST_ASSERT(relink_self.push_back(3));
    sstl::list<int, 4>::iterator second = relink_self.begin();
    ++second;
    SSTL_TEST_ASSERT(relink_self.splice(relink_self.begin(), relink_self, second));
    {
      const int expected[] = {2, 1, 3};
      expect_list_values(relink_self, expected, 3u);
    }
    SSTL_TEST_ASSERT(relink_self.splice(relink_self.begin(), relink_self));
  }

  sstl::list<int, 4> cross_single_dst;
  sstl::list<int, 2> cross_single_src;
  SSTL_TEST_ASSERT(cross_single_dst.push_back(1));
  SSTL_TEST_ASSERT(cross_single_src.push_back(7));
  SSTL_TEST_ASSERT(cross_single_dst.splice(cross_single_dst.end(), cross_single_src, cross_single_src.begin()));
  SSTL_TEST_ASSERT(cross_single_src.empty());

  {
    sstl::list<int, 4> templated_single_end_target;
    sstl::list<int, 2> templated_single_end_source;
    SSTL_TEST_ASSERT(templated_single_end_target.splice(templated_single_end_target.end(), templated_single_end_source, templated_single_end_source.end()));
    sstl::list<int, 1> full_templated_single_target;
    sstl::list<int, 2> templated_single_source;
    SSTL_TEST_ASSERT(full_templated_single_target.push_back(1));
    SSTL_TEST_ASSERT(templated_single_source.push_back(2));
    SSTL_TEST_ASSERT(!full_templated_single_target.splice(full_templated_single_target.end(), templated_single_source, templated_single_source.begin()));
    SSTL_TEST_EQ(templated_single_source.size(), 1u);
  }

  sstl::list<int, 1> full;
  sstl::list<int, 1> one;
  SSTL_TEST_ASSERT(full.push_back(1));
  SSTL_TEST_ASSERT(one.push_back(2));
  SSTL_TEST_ASSERT(!full.splice(full.end(), one));
  SSTL_TEST_ASSERT(!full.splice(full.end(), one, one.begin()));
  SSTL_TEST_ASSERT(!full.merge(one));

  sstl::list<int, 8> unsorted;
  SSTL_TEST_ASSERT(unsorted.push_back(4));
  SSTL_TEST_ASSERT(unsorted.push_back(1));
  SSTL_TEST_ASSERT(unsorted.push_back(3));
  SSTL_TEST_ASSERT(unsorted.push_back(2));
  unsorted.sort();
  {
    const int expected[] = {1, 2, 3, 4};
    expect_list_values(unsorted, expected, 4u);
  }

  sstl::list<int, 8> merge_dst;
  sstl::list<int, 8> merge_src;
  SSTL_TEST_ASSERT(merge_dst.push_back(1));
  SSTL_TEST_ASSERT(merge_dst.push_back(4));
  SSTL_TEST_ASSERT(merge_src.push_back(2));
  SSTL_TEST_ASSERT(merge_src.push_back(3));
  SSTL_TEST_ASSERT(merge_dst.merge(merge_src));
  SSTL_TEST_ASSERT(merge_src.empty());
  {
    const int expected[] = {1, 2, 3, 4};
    expect_list_values(merge_dst, expected, 4u);
  }
  {
    sstl::list<int, 8> merge_before_head_dst;
    sstl::list<int, 8> merge_before_head_src;
    SSTL_TEST_ASSERT(merge_before_head_dst.push_back(5));
    SSTL_TEST_ASSERT(merge_before_head_src.push_back(1));
    SSTL_TEST_ASSERT(merge_before_head_dst.merge(merge_before_head_src));
    {
      const int expected[] = {1, 5};
      expect_list_values(merge_before_head_dst, expected, 2u);
    }
  }

  sstl::list<int, 8> merge_template_dst;
  sstl::list<int, 2> merge_template_src;
  SSTL_TEST_ASSERT(merge_template_dst.push_back(1));
  SSTL_TEST_ASSERT(merge_template_dst.push_back(5));
  SSTL_TEST_ASSERT(merge_template_src.push_back(2));
  SSTL_TEST_ASSERT(merge_template_src.push_back(4));
  SSTL_TEST_ASSERT(merge_template_dst.merge(merge_template_src));
  SSTL_TEST_ASSERT(merge_template_src.empty());
  {
    const int expected[] = {1, 2, 4, 5};
    expect_list_values(merge_template_dst, expected, 4u);
  }
  {
    sstl::list<int, 8> templated_merge_before_head_dst;
    sstl::list<int, 2> templated_merge_before_head_src;
    SSTL_TEST_ASSERT(templated_merge_before_head_dst.push_back(5));
    SSTL_TEST_ASSERT(templated_merge_before_head_src.push_back(1));
    SSTL_TEST_ASSERT(templated_merge_before_head_dst.merge(templated_merge_before_head_src));
    {
      const int expected[] = {1, 5};
      expect_list_values(templated_merge_before_head_dst, expected, 2u);
    }
  }

  {
    sstl::list<int, 8> list_ops;
    SSTL_TEST_ASSERT(list_ops.push_back(1));
    SSTL_TEST_ASSERT(list_ops.push_back(2));
    SSTL_TEST_ASSERT(list_ops.push_back(2));
    SSTL_TEST_ASSERT(list_ops.push_back(3));
    SSTL_TEST_ASSERT(list_ops.push_back(4));
    list_ops.reverse();
    {
      const int expected[] = {4, 3, 2, 2, 1};
      expect_list_values(list_ops, expected, 5u);
    }
    list_ops.remove(99);
    list_ops.remove(3);
    list_ops.remove_if(is_even_int);
    {
      const int expected[] = {1};
      expect_list_values(list_ops, expected, 1u);
    }
    SSTL_TEST_ASSERT(list_ops.push_back(1));
    SSTL_TEST_ASSERT(list_ops.push_back(2));
    list_ops.unique();
    {
      const int expected[] = {1, 2};
      expect_list_values(list_ops, expected, 2u);
    }
  }

  {
    sstl::list<int, 2> trivial_sort;
    trivial_sort.sort();
    trivial_sort.unique();
    SSTL_TEST_ASSERT(trivial_sort.push_back(1));
    trivial_sort.sort();
    {
      const int expected[] = {1};
      expect_list_values(trivial_sort, expected, 1u);
    }
    SSTL_TEST_ASSERT(trivial_sort.splice(trivial_sort.end(), trivial_sort));
    SSTL_TEST_ASSERT(trivial_sort.merge(trivial_sort));
    sstl::list<int, 1> empty_for_transfer;
    SSTL_TEST_ASSERT(trivial_sort.splice(trivial_sort.end(), empty_for_transfer));
  }
}

static void algorithm_edge_paths_cover_short_ranges_and_branch_outcomes() {
  sstl_test::noalloc_guard guard;

  /*
    The oracle tests prove parity for normal-sized examples. This test adds the
    branchy edges that coverage tools are good at finding: empty ranges, early
    returns, partial matches, false predicate exits, and range tails.
  */
  int values[] = {-2, 0, 3, 3, 5};
  SSTL_TEST_EQ(sstl::find(values, values + 5, 3) - values, 2);
  SSTL_TEST_ASSERT(sstl::find(values, values + 5, 9) == values + 5);
  SSTL_TEST_EQ(sstl::find_if(values, values + 5, is_positive_int) - values, 2);
  SSTL_TEST_ASSERT(sstl::find_if(values, values + 5, is_large_int) == values + 5);
  SSTL_TEST_EQ(sstl::find_if_not(values, values + 5, is_negative_int) - values, 1);
  SSTL_TEST_ASSERT(sstl::find_if_not(values, values + 5, always_true_int) == values + 5);
  SSTL_TEST_ASSERT(!sstl::all_of(values, values + 5, is_positive_int));
  {
    int positive_values[] = {1, 2, 3};
    SSTL_TEST_ASSERT(sstl::all_of(positive_values, positive_values + 3, is_positive_int));
  }
  SSTL_TEST_ASSERT(sstl::any_of(values, values + 5, is_positive_int));
  SSTL_TEST_ASSERT(!sstl::any_of(values, values + 5, is_large_int));
  SSTL_TEST_ASSERT(!sstl::none_of(values, values + 5, is_positive_int));
  SSTL_TEST_EQ(sstl::count(values, values + 5, 3), 2u);
  SSTL_TEST_EQ(sstl::count_if(values, values + 5, is_positive_int), 3u);

  int expected_equal[] = {-2, 0, 3, 99, 5};
  SSTL_TEST_ASSERT(!sstl::equal(values, values + 5, expected_equal));
  sstl::pair<int*, int*> mismatch = sstl::mismatch(values, values + 5, expected_equal);
  SSTL_TEST_EQ(mismatch.first - values, 3);
  {
    int mod_left[] = {1, 12, 23};
    int mod_right[] = {11, 2, 13};
    int mod_bad[] = {11, 2, 14};
    SSTL_TEST_ASSERT(sstl::equal(mod_left, mod_left + 3, mod_right, same_mod_ten));
    SSTL_TEST_ASSERT(!sstl::equal(mod_left, mod_left + 3, mod_bad, same_mod_ten));
    sstl::pair<int*, int*> pred_mismatch = sstl::mismatch(mod_left, mod_left + 3, mod_bad, same_mod_ten);
    SSTL_TEST_EQ(pred_mismatch.first - mod_left, 2);
    pred_mismatch = sstl::mismatch(mod_left, mod_left + 3, mod_right, same_mod_ten);
    SSTL_TEST_ASSERT(pred_mismatch.first == mod_left + 3);
  }

  int needle[] = {3, 5};
  int empty_needle[] = {0};
  SSTL_TEST_ASSERT(sstl::search(values, values + 5, empty_needle, empty_needle) == values);
  SSTL_TEST_EQ(sstl::search(values, values + 5, needle, needle + 2) - values, 3);
  int absent_needle[] = {3, 6};
  SSTL_TEST_ASSERT(sstl::search(values, values + 5, absent_needle, absent_needle + 2) == values + 5);

  int lex_a[] = {1, 2};
  int lex_b[] = {1, 3};
  int lex_prefix[] = {1};
  SSTL_TEST_ASSERT(sstl::lexicographical_compare(lex_a, lex_a + 2, lex_b, lex_b + 2));
  SSTL_TEST_ASSERT(!sstl::lexicographical_compare(lex_b, lex_b + 2, lex_a, lex_a + 2));
  SSTL_TEST_ASSERT(sstl::lexicographical_compare(lex_prefix, lex_prefix + 1, lex_a, lex_a + 2));
  {
    int desc_a[] = {3, 2};
    int desc_b[] = {3, 1};
    int desc_equal[] = {3, 2};
    int desc_prefix[] = {3};
    SSTL_TEST_ASSERT(sstl::lexicographical_compare(desc_a, desc_a + 2, desc_b, desc_b + 2, greater_int()));
    SSTL_TEST_ASSERT(!sstl::lexicographical_compare(desc_b, desc_b + 2, desc_a, desc_a + 2, greater_int()));
    SSTL_TEST_ASSERT(!sstl::lexicographical_compare(desc_equal, desc_equal + 2, desc_a, desc_a + 2, greater_int()));
    SSTL_TEST_ASSERT(sstl::lexicographical_compare(desc_prefix, desc_prefix + 1, desc_a, desc_a + 2, greater_int()));
  }

  int mixed[] = {5, 1, 7, 0};
  SSTL_TEST_ASSERT(sstl::min_element(mixed + 4, mixed + 4) == mixed + 4);
  SSTL_TEST_ASSERT(sstl::max_element(mixed + 4, mixed + 4) == mixed + 4);
  SSTL_TEST_ASSERT(sstl::min_element(mixed + 4, mixed + 4, greater_int()) == mixed + 4);
  SSTL_TEST_ASSERT(sstl::max_element(mixed + 4, mixed + 4, greater_int()) == mixed + 4);
  SSTL_TEST_EQ(*sstl::min_element(mixed, mixed + 4), 0);
  SSTL_TEST_EQ(*sstl::max_element(mixed, mixed + 4), 7);
  SSTL_TEST_EQ(*sstl::min_element(mixed, mixed + 4, greater_int()), 7);
  SSTL_TEST_EQ(*sstl::max_element(mixed, mixed + 4, greater_int()), 0);

  int copied[5] = {0, 0, 0, 0, 0};
  int copied_if[5] = {0, 0, 0, 0, 0};
  SSTL_TEST_ASSERT(sstl::copy(values, values + 5, copied) == copied + 5);
  SSTL_TEST_ASSERT(sstl::copy_if(values, values + 5, copied_if, is_positive_int) == copied_if + 3);
  SSTL_TEST_EQ(copied_if[0], 3);
  SSTL_TEST_EQ(copied_if[2], 5);

  int transformed[5] = {0, 0, 0, 0, 0};
  SSTL_TEST_ASSERT(sstl::transform(values, values + 5, transformed, double_int) == transformed + 5);
  SSTL_TEST_EQ(transformed[0], -4);
  SSTL_TEST_EQ(transformed[4], 10);

  int replace_values[] = {1, 2, 3, 4};
  sstl::replace(replace_values, replace_values + 4, 2, 9);
  sstl::replace_if(replace_values, replace_values + 4, is_even_int, 8);
  SSTL_TEST_EQ(replace_values[1], 9);
  SSTL_TEST_EQ(replace_values[3], 8);

  int remove_values[] = {1, 2, 3, 2, 4};
  SSTL_TEST_ASSERT(sstl::remove(remove_values, remove_values + 5, 2) == remove_values + 3);
  int remove_if_values[] = {1, 2, 3, 4};
  SSTL_TEST_ASSERT(sstl::remove_if(remove_if_values, remove_if_values + 4, is_even_int) == remove_if_values + 2);

  int reverse_one[] = {1};
  int reverse_odd[] = {1, 2, 3};
  sstl::reverse(reverse_one, reverse_one + 1);
  sstl::reverse(reverse_odd, reverse_odd + 3);
  SSTL_TEST_EQ(reverse_odd[0], 3);
  SSTL_TEST_EQ(reverse_odd[2], 1);

  int rotate_values[] = {1, 2, 3, 4};
  SSTL_TEST_ASSERT(sstl::rotate(rotate_values, rotate_values + 1, rotate_values + 4) == rotate_values + 3);
  SSTL_TEST_EQ(rotate_values[0], 2);
  SSTL_TEST_EQ(rotate_values[3], 1);

  int unique_values[] = {1, 1, 2, 2, 3};
  SSTL_TEST_ASSERT(sstl::unique(unique_values, unique_values + 5) == unique_values + 3);
  SSTL_TEST_EQ(unique_values[0], 1);
  SSTL_TEST_EQ(unique_values[2], 3);
  SSTL_TEST_ASSERT(sstl::unique(unique_values, unique_values) == unique_values);
  {
    int pred_unique[] = {1, 11, 2, 12, 3};
    SSTL_TEST_ASSERT(sstl::unique(pred_unique, pred_unique + 5, same_mod_ten) == pred_unique + 3);
    SSTL_TEST_EQ(pred_unique[0], 1);
    SSTL_TEST_EQ(pred_unique[1], 2);
    SSTL_TEST_EQ(pred_unique[2], 3);
    SSTL_TEST_ASSERT(sstl::unique(pred_unique, pred_unique, same_mod_ten) == pred_unique);
  }

  int sort_values[] = {4, 1, 3, 2};
  sstl::sort(sort_values, sort_values + 4);
  SSTL_TEST_ASSERT(sstl::is_sorted(sort_values, sort_values + 4));

  int stable_values[] = {2, 1, 2, 0};
  sstl::stable_sort(stable_values, stable_values + 4);
  SSTL_TEST_ASSERT(sstl::is_sorted(stable_values, stable_values + 4));
  {
    int lower_bound_values[] = {1, 3, 5};
    int upper_bound_values[] = {1, 3, 5};
    SSTL_TEST_EQ(sstl::stable_lower_bound(lower_bound_values, lower_bound_values + 3, 0, sstl::less<int>()) - lower_bound_values, 0);
    SSTL_TEST_EQ(sstl::stable_upper_bound(upper_bound_values, upper_bound_values + 3, 0, sstl::less<int>()) - upper_bound_values, 0);
  }

  int partial_values[] = {5, 4, 3, 2, 1};
  sstl::partial_sort(partial_values, partial_values, partial_values + 5);
  sstl::partial_sort(partial_values, partial_values + 3, partial_values + 5);
  SSTL_TEST_EQ(partial_values[0], 1);
  SSTL_TEST_EQ(partial_values[2], 3);

  int nth_values[] = {9, 1, 8, 2, 7};
  sstl::nth_element(nth_values, nth_values + 2, nth_values + 5);
  SSTL_TEST_EQ(nth_values[2], 7);
  {
    int nth_exit_values[] = {2, 1, 3};
    sstl::nth_element(nth_exit_values, nth_exit_values + 1, nth_exit_values + 3);
    SSTL_TEST_EQ(nth_exit_values[1], 2);
  }

  int heap_values[] = {3, 1, 4, 2};
  sstl::make_heap(heap_values, heap_values + 4);
  SSTL_TEST_EQ(heap_values[0], 4);
  int push_heap_values[] = {4, 3, 2, 1, 5};
  sstl::push_heap(push_heap_values, push_heap_values + 5);
  SSTL_TEST_EQ(push_heap_values[0], 5);
  {
    int already_heap[] = {5, 3, 4, 1, 2};
    sstl::push_heap(already_heap, already_heap + 5);
    SSTL_TEST_EQ(already_heap[0], 5);
    sstl::push_heap(already_heap, already_heap + 1);
    sstl::pop_heap(already_heap, already_heap + 1);
  }
  sstl::pop_heap(push_heap_values, push_heap_values + 5);
  SSTL_TEST_EQ(push_heap_values[4], 5);
  sstl::sort_heap(push_heap_values, push_heap_values + 4);
  SSTL_TEST_ASSERT(sstl::is_sorted(push_heap_values, push_heap_values + 4));

  int partition_values[] = {1, 2, 3, 4};
  SSTL_TEST_ASSERT(sstl::partition(partition_values, partition_values + 4, is_even_int) == partition_values + 2);
  int stable_partition_values[] = {1, 2, 3, 4};
  SSTL_TEST_ASSERT(sstl::stable_partition(stable_partition_values, stable_partition_values + 4, is_even_int) == stable_partition_values + 2);

  int set_a[] = {1, 3, 5};
  int set_b[] = {2, 3, 4};
  int set_out[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  SSTL_TEST_ASSERT(!sstl::includes(set_a, set_a + 3, set_b, set_b + 3));
  SSTL_TEST_ASSERT(sstl::includes(set_a, set_a + 3, set_a + 1, set_a + 2));
  SSTL_TEST_ASSERT(sstl::set_union(set_a, set_a + 3, set_b, set_b + 3, set_out) == set_out + 5);
  SSTL_TEST_ASSERT(sstl::set_intersection(set_a, set_a + 3, set_b, set_b + 3, set_out) == set_out + 1);
  SSTL_TEST_ASSERT(sstl::set_difference(set_a, set_a + 3, set_b, set_b + 3, set_out) == set_out + 2);
  SSTL_TEST_ASSERT(sstl::set_symmetric_difference(set_a, set_a + 3, set_b, set_b + 3, set_out) == set_out + 4);
  {
    int desc_a[] = {5, 3, 1};
    int desc_b[] = {4, 3, 2};
    int desc_out[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    SSTL_TEST_ASSERT(!sstl::includes(desc_a, desc_a + 3, desc_b, desc_b + 3, greater_int()));
    SSTL_TEST_ASSERT(sstl::includes(desc_a, desc_a + 3, desc_a + 1, desc_a + 2, greater_int()));
    SSTL_TEST_ASSERT(sstl::merge(desc_a, desc_a + 3, desc_b, desc_b + 3, desc_out, greater_int()) == desc_out + 6);
    SSTL_TEST_ASSERT(sstl::set_union(desc_a, desc_a + 3, desc_b, desc_b + 3, desc_out, greater_int()) == desc_out + 5);
    SSTL_TEST_ASSERT(sstl::set_intersection(desc_a, desc_a + 3, desc_b, desc_b + 3, desc_out, greater_int()) == desc_out + 1);
    SSTL_TEST_ASSERT(sstl::set_difference(desc_a, desc_a + 3, desc_b, desc_b + 3, desc_out, greater_int()) == desc_out + 2);
    SSTL_TEST_ASSERT(sstl::set_difference(desc_a, desc_a, desc_b, desc_b + 3, desc_out, greater_int()) == desc_out);
    SSTL_TEST_ASSERT(sstl::set_symmetric_difference(desc_a, desc_a + 3, desc_b, desc_b + 3, desc_out, greater_int()) == desc_out + 4);
  }

  int bounds[] = {1, 2, 2, 4};
  SSTL_TEST_EQ(sstl::lower_bound(bounds, bounds + 4, 2) - bounds, 1);
  SSTL_TEST_EQ(sstl::upper_bound(bounds, bounds + 4, 2) - bounds, 3);
  SSTL_TEST_EQ(sstl::equal_range(bounds, bounds + 4, 2).second - bounds, 3);
  SSTL_TEST_ASSERT(sstl::binary_search(bounds, bounds + 4, 4));
  SSTL_TEST_ASSERT(!sstl::binary_search(bounds, bounds + 4, 3));
  {
    int desc_bounds[] = {4, 2, 2, 1};
    SSTL_TEST_ASSERT(sstl::binary_search(desc_bounds, desc_bounds + 4, 2, greater_int()));
    SSTL_TEST_ASSERT(!sstl::binary_search(desc_bounds, desc_bounds + 4, 3, greater_int()));
  }

  int numeric_out[4] = {0, 0, 0, 0};
  SSTL_TEST_EQ(sstl::accumulate(bounds, bounds + 4, 0), 9);
  SSTL_TEST_EQ(sstl::inner_product(bounds, bounds + 4, bounds, 0), 25);
  SSTL_TEST_ASSERT(sstl::partial_sum(bounds, bounds, numeric_out) == numeric_out);
  SSTL_TEST_ASSERT(sstl::partial_sum(bounds, bounds + 4, numeric_out) == numeric_out + 4);
  SSTL_TEST_EQ(numeric_out[3], 9);
  SSTL_TEST_ASSERT(sstl::adjacent_difference(bounds, bounds, numeric_out) == numeric_out);
  SSTL_TEST_ASSERT(sstl::adjacent_difference(bounds, bounds + 4, numeric_out) == numeric_out + 4);
  SSTL_TEST_EQ(numeric_out[0], 1);
  SSTL_TEST_EQ(numeric_out[2], 0);
  sstl::iota(numeric_out, numeric_out + 4, 7);
  SSTL_TEST_EQ(numeric_out[0], 7);
  SSTL_TEST_EQ(numeric_out[3], 10);
}

struct variant_visit_sum {
  int* out;
  explicit variant_visit_sum(int* p) : out(p) {}
  void operator()(int value) const { *out += value; }
  void operator()(char value) const { *out += static_cast<int>(value); }
  void operator()(long value) const { *out += static_cast<int>(value); }
  void operator()(short value) const { *out += static_cast<int>(value); }
};

static void small_utility_headers_get_edge_branch_exercise() {
  sstl_test::noalloc_guard guard;

  sstl::logical_and<bool> land;
  sstl::logical_or<bool> lor;
  SSTL_TEST_ASSERT(land(true, true));
  SSTL_TEST_ASSERT(!land(false, true));
  SSTL_TEST_ASSERT(!land(true, false));
  SSTL_TEST_ASSERT(lor(false, true));
  SSTL_TEST_ASSERT(lor(true, false));
  SSTL_TEST_ASSERT(!lor(false, false));

  sstl::optional<int> empty_optional;
  sstl::optional<int> empty_copy_source;
  sstl::optional<int> copied_empty_optional(empty_copy_source);
  sstl::optional<int> present_optional(7);
  sstl::optional<int> copied_optional(present_optional);
  SSTL_TEST_ASSERT(empty_optional == sstl::nullopt);
  SSTL_TEST_ASSERT(copied_empty_optional == sstl::nullopt);
  SSTL_TEST_ASSERT(present_optional != sstl::nullopt);
  SSTL_TEST_ASSERT(copied_optional == present_optional);
  SSTL_TEST_ASSERT(empty_optional.try_value() == 0);
  SSTL_TEST_ASSERT(present_optional.try_value() != 0);
  SSTL_TEST_EQ(present_optional.value_or(9), 7);
  SSTL_TEST_EQ(empty_optional.value_or(9), 9);
  empty_optional = present_optional;
  SSTL_TEST_ASSERT(empty_optional == 7);
  present_optional = 8;
  SSTL_TEST_EQ(present_optional.value(), 8);
  present_optional = sstl::optional<int>();
  SSTL_TEST_ASSERT(present_optional == sstl::nullopt);
  empty_optional = sstl::nullopt;
  SSTL_TEST_ASSERT(sstl::nullopt == empty_optional);
  empty_optional.emplace(11);
  SSTL_TEST_EQ(empty_optional.value(), 11);
  {
    const sstl::optional<int>& const_present = empty_optional;
    const sstl::optional<int>& const_empty = present_optional;
    SSTL_TEST_ASSERT(const_present.try_value() != 0);
    SSTL_TEST_ASSERT(const_empty.try_value() == 0);
    SSTL_TEST_EQ(const_present.value(), 11);
    SSTL_TEST_ASSERT(!(const_present == const_empty));
    SSTL_TEST_ASSERT(const_empty == sstl::optional<int>());
    SSTL_TEST_ASSERT(!(sstl::optional<int>(1) == sstl::optional<int>(2)));
    SSTL_TEST_ASSERT(!(const_empty == 11));
    SSTL_TEST_ASSERT(const_present != 99);
  }

  sstl::function1<int, int> fn1(double_int);
  sstl::function1<int, int> fn1_copy(fn1);
  sstl::function1<int, int> fn1_assigned;
  fn1_assigned = fn1_copy;
  fn1_assigned = fn1_assigned;
  SSTL_TEST_ASSERT(fn1_assigned);
  SSTL_TEST_EQ(fn1_assigned(4), 8);
  {
    sstl::function1<int, int> empty_fn1;
    sstl::function1<int, int> copied_empty_fn1(empty_fn1);
    fn1_assigned = copied_empty_fn1;
    SSTL_TEST_ASSERT(!fn1_assigned);
  }
  {
    mutable_nullary zero_source;
    sstl::function0<int> mutable_zero(zero_source);
    const sstl::function0<int>& const_mutable_zero = mutable_zero;
    SSTL_TEST_EQ(const_mutable_zero(), 1);
    SSTL_TEST_EQ(const_mutable_zero(), 2);

    mutable_unary one_source;
    sstl::function1<int, int> mutable_one(one_source);
    const sstl::function1<int, int>& const_mutable_one = mutable_one;
    SSTL_TEST_EQ(const_mutable_one(3), 3);
    SSTL_TEST_EQ(const_mutable_one(4), 7);
  }

  sstl::function2<int, int, int> fn2(add_two_ints);
  sstl::function2<int, int, int> fn2_copy(fn2);
  sstl::function2<int, int, int> fn2_assigned;
  fn2_assigned = fn2_copy;
  fn2_assigned = fn2_assigned;
  SSTL_TEST_ASSERT(fn2_assigned);
  SSTL_TEST_EQ(fn2_assigned(3, 4), 7);
  {
    sstl::function2<int, int, int> empty_fn2;
    sstl::function2<int, int, int> copied_empty_fn2(empty_fn2);
    fn2_assigned = copied_empty_fn2;
    SSTL_TEST_ASSERT(!fn2_assigned);
  }
  {
    mutable_binary two_source;
    sstl::function2<int, int, int> mutable_two(two_source);
    const sstl::function2<int, int, int>& const_mutable_two = mutable_two;
    SSTL_TEST_EQ(const_mutable_two(1, 2), 3);
    SSTL_TEST_EQ(const_mutable_two(3, 4), 10);
  }

  sstl::function3<int, int, int, int> fn3(add_three_ints);
  sstl::function3<int, int, int, int> fn3_copy(fn3);
  sstl::function3<int, int, int, int> fn3_assigned;
  fn3_assigned = fn3_copy;
  fn3_assigned = fn3_assigned;
  SSTL_TEST_ASSERT(fn3_assigned);
  SSTL_TEST_EQ(fn3_assigned(1, 2, 3), 6);
  {
    sstl::function3<int, int, int, int> empty_fn3;
    sstl::function3<int, int, int, int> copied_empty_fn3(empty_fn3);
    fn3_assigned = copied_empty_fn3;
    SSTL_TEST_ASSERT(!fn3_assigned);
  }
  {
    mutable_ternary three_source;
    sstl::function3<int, int, int, int> mutable_three(three_source);
    const sstl::function3<int, int, int, int>& const_mutable_three = mutable_three;
    SSTL_TEST_EQ(const_mutable_three(1, 2, 3), 6);
    SSTL_TEST_EQ(const_mutable_three(4, 5, 6), 21);
  }

  sstl::bitset<8> bits;
  bits.set(1).set(3);
  sstl::bitset<8> inverted = ~bits;
  SSTL_TEST_ASSERT(inverted.test(0));
  SSTL_TEST_ASSERT(!inverted.test(1));
  bits <<= 8u;
  SSTL_TEST_ASSERT(bits.none());

  sstl::priority_queue<int, 2> heap;
  heap.pop();
  SSTL_TEST_ASSERT(heap.push(2));
  SSTL_TEST_ASSERT(heap.push(5));
  SSTL_TEST_ASSERT(!heap.push(9));
  SSTL_TEST_EQ(heap.top(), 5);
  heap.pop();
  SSTL_TEST_EQ(heap.top(), 2);
  heap.pop();
  SSTL_TEST_ASSERT(heap.empty());

  sstl::string_view view("abcabc", 6u);
  SSTL_TEST_EQ(view.find("zz"), sstl::npos);
  SSTL_TEST_EQ(view.rfind("zz"), sstl::npos);
  SSTL_TEST_EQ(view.rfind("abcabcx"), sstl::npos);
  SSTL_TEST_EQ(view.compare("abcabc"), 0);
  SSTL_TEST_ASSERT(view.compare("abcabcd") < 0);
  SSTL_TEST_ASSERT(view.compare("abd") < 0);
  SSTL_TEST_ASSERT(view.compare("ab") > 0);
  SSTL_TEST_ASSERT(view.starts_with("abc"));
  SSTL_TEST_ASSERT(!view.starts_with("abd"));
  SSTL_TEST_ASSERT(!view.starts_with("abcd"));
  SSTL_TEST_ASSERT(!view.starts_with("abcdefg"));
  SSTL_TEST_ASSERT(view.ends_with("abc"));
  SSTL_TEST_ASSERT(!view.ends_with("abd"));
  SSTL_TEST_ASSERT(!view.ends_with("zabc"));
  SSTL_TEST_ASSERT(!view.ends_with("abcdefg"));
  view.remove_prefix(99u);
  SSTL_TEST_ASSERT(view.empty());
  sstl::string_view suffix("abc", 3u);
  suffix.remove_suffix(99u);
  SSTL_TEST_ASSERT(suffix.empty());

  sstl::variant2<int, char> v2(7);
  SSTL_TEST_ASSERT(sstl::get_if<0>(&v2) != 0);
  SSTL_TEST_ASSERT(sstl::get_if<1>(&v2) == 0);
  SSTL_TEST_ASSERT(sstl::get_if<0>(static_cast<sstl::variant2<int, char>*>(0)) == 0);

  int visited = 0;
  sstl::variant4<int, char, long, short> v4(1);
  SSTL_TEST_EQ(v4.get0(), 1);
  v4.visit(variant_visit_sum(&visited));
  v4 = static_cast<char>(2);
  SSTL_TEST_EQ(v4.get1(), static_cast<char>(2));
  v4.visit(variant_visit_sum(&visited));
  v4 = static_cast<long>(3);
  SSTL_TEST_EQ(v4.get2(), static_cast<long>(3));
  v4.visit(variant_visit_sum(&visited));
  v4 = static_cast<short>(4);
  SSTL_TEST_EQ(v4.get3(), static_cast<short>(4));
  v4.visit(variant_visit_sum(&visited));
  SSTL_TEST_EQ(visited, 10);
  SSTL_TEST_ASSERT(sstl::holds_alternative<short>(v4));
}

static void documented_swap_overloads_exchange_fixed_storage_containers() {
  sstl_test::noalloc_guard guard;

  /*
    These calls intentionally use unqualified swap. That keeps argument
    dependent lookup in play and proves that the container-specific overloads
    are visible to ordinary generic code without falling back to a hosted STL
    dependency.
  */
  sstl::list<int, 4> left_list;
  sstl::list<int, 4> right_list;
  SSTL_TEST_ASSERT(left_list.push_back(1));
  SSTL_TEST_ASSERT(left_list.push_back(2));
  SSTL_TEST_ASSERT(right_list.push_back(7));
  swap(left_list, right_list);
  {
    const int expected_left[] = {7};
    const int expected_right[] = {1, 2};
    expect_list_values(left_list, expected_left, 1u);
    expect_list_values(right_list, expected_right, 2u);
  }

  sstl::forward_list<int, 4> left_forward;
  sstl::forward_list<int, 4> right_forward;
  sstl::forward_list<int, 4>::iterator left_tail = left_forward.before_begin();
  left_tail = left_forward.insert_after(left_tail, 3);
  left_tail = left_forward.insert_after(left_tail, 4);
  (void)right_forward.insert_after(right_forward.before_begin(), 9);
  swap(left_forward, right_forward);
  {
    const int expected_left[] = {9};
    const int expected_right[] = {3, 4};
    expect_forward_list_values(left_forward, expected_left, 1u);
    expect_forward_list_values(right_forward, expected_right, 2u);
  }

  sstl::map<int, int, 4> left_map;
  sstl::map<int, int, 4> right_map;
  SSTL_TEST_ASSERT(left_map.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(left_map.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(right_map.insert(sstl::make_pair(8, 80)).second);
  swap(left_map, right_map);
  SSTL_TEST_EQ(left_map.size(), 1u);
  SSTL_TEST_EQ(left_map.find(8)->second, 80);
  SSTL_TEST_EQ(right_map.size(), 2u);
  SSTL_TEST_EQ(right_map.find(1)->second, 10);
  SSTL_TEST_EQ(right_map.find(2)->second, 20);
  swap(left_map, left_map);
  SSTL_TEST_EQ(left_map.size(), 1u);
  SSTL_TEST_EQ(left_map.find(8)->second, 80);
  left_map = left_map;
  SSTL_TEST_EQ(left_map.size(), 1u);
  SSTL_TEST_EQ(left_map.find(8)->second, 80);

  sstl::set<int, 4> left_set;
  sstl::set<int, 4> right_set;
  SSTL_TEST_ASSERT(left_set.insert(5).second);
  SSTL_TEST_ASSERT(left_set.insert(6).second);
  SSTL_TEST_ASSERT(right_set.insert(1).second);
  swap(left_set, right_set);
  SSTL_TEST_EQ(left_set.size(), 1u);
  SSTL_TEST_ASSERT(left_set.find(1) != left_set.end());
  SSTL_TEST_EQ(right_set.size(), 2u);
  SSTL_TEST_ASSERT(right_set.find(5) != right_set.end());
  SSTL_TEST_ASSERT(right_set.find(6) != right_set.end());

  sstl::flat_map<int, int, 4> left_flat_map;
  sstl::flat_map<int, int, 4> right_flat_map;
  SSTL_TEST_ASSERT(left_flat_map.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(left_flat_map.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(right_flat_map.insert(sstl::make_pair(9, 90)).second);
  swap(left_flat_map, right_flat_map);
  SSTL_TEST_EQ(left_flat_map.size(), 1u);
  SSTL_TEST_EQ(left_flat_map.find(9)->second, 90);
  SSTL_TEST_EQ(right_flat_map.size(), 2u);
  SSTL_TEST_EQ(right_flat_map.find(1)->second, 10);
  SSTL_TEST_EQ(right_flat_map.find(2)->second, 20);

  sstl::flat_set<int, 4> left_flat_set;
  sstl::flat_set<int, 4> right_flat_set;
  SSTL_TEST_ASSERT(left_flat_set.insert(3).second);
  SSTL_TEST_ASSERT(left_flat_set.insert(4).second);
  SSTL_TEST_ASSERT(right_flat_set.insert(8).second);
  swap(left_flat_set, right_flat_set);
  SSTL_TEST_EQ(left_flat_set.size(), 1u);
  SSTL_TEST_ASSERT(left_flat_set.find(8) != left_flat_set.end());
  SSTL_TEST_EQ(right_flat_set.size(), 2u);
  SSTL_TEST_ASSERT(right_flat_set.find(3) != right_flat_set.end());
  SSTL_TEST_ASSERT(right_flat_set.find(4) != right_flat_set.end());

  sstl::unordered_map<int, int, 4, 5> left_hash_map;
  sstl::unordered_map<int, int, 4, 5> right_hash_map;
  SSTL_TEST_ASSERT(left_hash_map.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(left_hash_map.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(right_hash_map.insert(sstl::make_pair(7, 70)).second);
  swap(left_hash_map, right_hash_map);
  SSTL_TEST_EQ(left_hash_map.size(), 1u);
  SSTL_TEST_EQ(left_hash_map.find(7)->second, 70);
  SSTL_TEST_EQ(right_hash_map.size(), 2u);
  SSTL_TEST_EQ(right_hash_map.find(1)->second, 10);
  SSTL_TEST_EQ(right_hash_map.find(2)->second, 20);

  sstl::unordered_set<int, 4, 5> left_hash_set;
  sstl::unordered_set<int, 4, 5> right_hash_set;
  SSTL_TEST_ASSERT(left_hash_set.insert(1).second);
  SSTL_TEST_ASSERT(left_hash_set.insert(2).second);
  SSTL_TEST_ASSERT(right_hash_set.insert(7).second);
  swap(left_hash_set, right_hash_set);
  SSTL_TEST_EQ(left_hash_set.size(), 1u);
  SSTL_TEST_ASSERT(left_hash_set.find(7) != left_hash_set.end());
  SSTL_TEST_EQ(right_hash_set.size(), 2u);
  SSTL_TEST_ASSERT(right_hash_set.find(1) != right_hash_set.end());
  SSTL_TEST_ASSERT(right_hash_set.find(2) != right_hash_set.end());
}

int main() {
  const sstl_test::test_case tests[] = {
    {"array_span_and_string_weak_branch_edges_are_exercised", array_span_and_string_weak_branch_edges_are_exercised},
    {"string_edge_paths_keep_text_recoverable", string_edge_paths_keep_text_recoverable},
    {"forward_list_transfer_sort_and_failure_paths_are_exercised", forward_list_transfer_sort_and_failure_paths_are_exercised},
    {"map_insert_erase_rebalancing_and_capacity_edges_are_exercised", map_insert_erase_rebalancing_and_capacity_edges_are_exercised},
    {"map_permutation_rebalance_matrix_exercises_small_trees", map_permutation_rebalance_matrix_exercises_small_trees},
    {"flat_ordered_and_unordered_sets_report_end_on_missing_or_full", flat_ordered_and_unordered_sets_report_end_on_missing_or_full},
    {"flat_and_hash_maps_exercise_direct_contracts", flat_and_hash_maps_exercise_direct_contracts},
    {"vector_and_deque_return_policy_edges_are_exercised", vector_and_deque_return_policy_edges_are_exercised},
    {"list_splice_merge_sort_and_empty_edges_are_exercised", list_splice_merge_sort_and_empty_edges_are_exercised},
    {"algorithm_edge_paths_cover_short_ranges_and_branch_outcomes", algorithm_edge_paths_cover_short_ranges_and_branch_outcomes},
    {"small_utility_headers_get_edge_branch_exercise", small_utility_headers_get_edge_branch_exercise},
    {"documented_swap_overloads_exchange_fixed_storage_containers", documented_swap_overloads_exchange_fixed_storage_containers}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
