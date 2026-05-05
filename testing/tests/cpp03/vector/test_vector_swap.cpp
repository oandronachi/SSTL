// test_id: cpp03.vector.swap.elementwise
// api_ids: [cpp.vector.swap, cpp.vector.comparison, cpp.vector.begin, cpp.vector.data, cpp.vector.assign, cpp.vector.insert, cpp.vector.try_front, cpp.vector.try_back, cpp.vector.cross_capacity, cpp.vector.factories, cpp.deque.swap]
// req_ids: [REQ-05, REQ-06]
// patterns: [SWAP-SEMANTICS, ITER-INVALIDATION]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is a direct SSTL contract test; prefer changing the manifest and comments together when behavior changes.
 *   - The assertions are black-box: they verify public observable state rather than private representation.
 *   - If this test fails against an implementation, first compare the implementation API to the SSTL public contract.
 */
#include <sstl/vector.hpp>
#include <sstl/deque.hpp>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"
#include "tracked.hpp"

static void swap_exchanges_values_not_storage() {
  sstl_test::noalloc_guard guard;
  sstl::vector<int, 4> a;
  sstl::vector<int, 4> b;
  SSTL_TEST_ASSERT(a.push_back(1));
  SSTL_TEST_ASSERT(a.push_back(2));
  SSTL_TEST_ASSERT(b.push_back(9));
  int* a_data = a.data();
  int* b_data = b.data();
  a.swap(b);
  SSTL_TEST_ASSERT(a.data() == a_data);
  SSTL_TEST_ASSERT(b.data() == b_data);
  SSTL_TEST_EQ(a.size(), 1u);
  SSTL_TEST_EQ(b.size(), 2u);
  SSTL_TEST_EQ(a[0], 9);
  SSTL_TEST_EQ(b[0], 1);
  SSTL_TEST_EQ(b[1], 2);
}

static void vector_swap_uses_elementwise_tail_transfer() {
  sstl_test::noalloc_guard guard;
  sstl_test::tracked::reset();
  sstl::vector<sstl_test::tracked, 4> a;
  sstl::vector<sstl_test::tracked, 4> b;
  SSTL_TEST_ASSERT(a.push_back(sstl_test::tracked(1)));
  SSTL_TEST_ASSERT(a.push_back(sstl_test::tracked(2)));
  SSTL_TEST_ASSERT(b.push_back(sstl_test::tracked(9)));
  sstl_test::tracked::copy_ctor = 0;
  sstl_test::tracked::assign = 0;
  a.swap(b);
  SSTL_TEST_EQ(a.size(), 1u);
  SSTL_TEST_EQ(b.size(), 2u);
  SSTL_TEST_EQ(a[0].value, 9);
  SSTL_TEST_EQ(b[0].value, 1);
  SSTL_TEST_EQ(b[1].value, 2);
  SSTL_TEST_ASSERT(sstl_test::tracked::copy_ctor <= 2);
}

static void deque_swap_uses_elementwise_tail_transfer() {
  sstl_test::noalloc_guard guard;
  sstl_test::tracked::reset();
  sstl::deque<sstl_test::tracked, 4> a;
  sstl::deque<sstl_test::tracked, 4> b;
  SSTL_TEST_ASSERT(a.push_back(sstl_test::tracked(1)));
  SSTL_TEST_ASSERT(a.push_back(sstl_test::tracked(2)));
  SSTL_TEST_ASSERT(b.push_back(sstl_test::tracked(9)));
  sstl_test::tracked::copy_ctor = 0;
  sstl_test::tracked::assign = 0;
  a.swap(b);
  SSTL_TEST_EQ(a.size(), 1u);
  SSTL_TEST_EQ(b.size(), 2u);
  SSTL_TEST_EQ(a[0].value, 9);
  SSTL_TEST_EQ(b[0].value, 1);
  SSTL_TEST_EQ(b[1].value, 2);
  SSTL_TEST_ASSERT(sstl_test::tracked::copy_ctor <= 2);

  sstl::deque<int, 4> empty_deque;
  SSTL_TEST_ASSERT(empty_deque.try_front() == 0);
  SSTL_TEST_ASSERT(empty_deque.try_back() == 0);
  empty_deque.swap(empty_deque);

  sstl::deque<int, 4> short_deque;
  sstl::deque<int, 4> long_deque;
  SSTL_TEST_ASSERT(short_deque.push_back(1));
  SSTL_TEST_ASSERT(long_deque.push_back(7));
  SSTL_TEST_ASSERT(long_deque.push_back(8));
  SSTL_TEST_ASSERT(long_deque.push_back(9));
  short_deque.swap(long_deque);
  SSTL_TEST_EQ(short_deque.size(), 3u);
  SSTL_TEST_EQ(long_deque.size(), 1u);
  SSTL_TEST_EQ(short_deque[2], 9);
  SSTL_TEST_EQ(long_deque[0], 1);
}

static void vector_assign_insert_try_and_factories_cover_static_api() {
  sstl_test::noalloc_guard guard;
  sstl::vector<int, 6> v;
  SSTL_TEST_ASSERT(v.assign(3, 7));
  SSTL_TEST_EQ(v.size(), 3u);
  SSTL_TEST_EQ(v[0], 7);
  SSTL_TEST_EQ(*v.try_front(), 7);
  SSTL_TEST_EQ(*v.try_back(), 7);
  SSTL_TEST_ASSERT(v.insert(v.begin() + 1, 2, 9) != v.end());
  SSTL_TEST_EQ(v.size(), 5u);
  SSTL_TEST_EQ(v[0], 7);
  SSTL_TEST_EQ(v[1], 9);
  SSTL_TEST_EQ(v[2], 9);
  SSTL_TEST_EQ(v[3], 7);
  const int range_values[] = {1, 2};
  SSTL_TEST_ASSERT(v.assign(range_values, range_values + 2));
  sstl::vector<int, 6>::iterator inserted_tail = v.insert(v.end(), range_values, range_values + 2);
  SSTL_TEST_ASSERT(inserted_tail != v.end());
  SSTL_TEST_EQ(v.size(), 4u);
  SSTL_TEST_EQ(v[0], 1);
  SSTL_TEST_EQ(v[1], 2);
  SSTL_TEST_EQ(v[2], 1);
  SSTL_TEST_EQ(v[3], 2);
  SSTL_TEST_ASSERT(!v.assign(7, 1));
  SSTL_TEST_EQ(v.size(), 4u);
  SSTL_TEST_ASSERT(v.insert(v.begin(), 0u, 5) == v.begin());
  SSTL_TEST_ASSERT(v.insert(v.end(), 3u, 5) == v.end());
  SSTL_TEST_EQ(v.size(), 4u);
  const int too_many[] = {1, 2, 3, 4, 5, 6, 7};
  SSTL_TEST_ASSERT(!v.assign(too_many, too_many + 7));
  SSTL_TEST_ASSERT(!v.assign(too_many, 7u));
  SSTL_TEST_EQ(v.size(), 4u);

  sstl::vector<int, 6> assigned;
  assigned = v;
  SSTL_TEST_EQ(assigned.size(), 4u);
  SSTL_TEST_EQ(assigned[3], 2);

  sstl::vector<int, 6> range_shift;
  SSTL_TEST_ASSERT(range_shift.assign(5, 1));
  const int one_value[] = {9};
  SSTL_TEST_ASSERT(range_shift.insert(range_shift.begin() + 1, one_value, one_value + 1) != range_shift.end());
  SSTL_TEST_EQ(range_shift.size(), 6u);
  SSTL_TEST_EQ(range_shift[1], 9);
  SSTL_TEST_EQ(range_shift[5], 1);
  SSTL_TEST_ASSERT(range_shift.insert(range_shift.begin(), one_value, one_value) == range_shift.begin());
  SSTL_TEST_ASSERT(range_shift.insert(range_shift.begin(), one_value, one_value + 1) == range_shift.end());

  sstl::vector<int, 2> small;
  SSTL_TEST_ASSERT(small.push_back(4));
  SSTL_TEST_ASSERT(small.push_back(5));
  sstl::vector<int, 6> larger(small);
  SSTL_TEST_EQ(larger.size(), 2u);
  SSTL_TEST_EQ(larger[1], 5);

  sstl::vector<int, 6> made = sstl::make_vector<int, 6>(range_values, 2u);
  SSTL_TEST_EQ(made.size(), 2u);
  SSTL_TEST_EQ(made[1], 2);
  sstl::vector<int, 6> from_larger = sstl::from_array<int, 6>(range_values);
  SSTL_TEST_EQ(from_larger.size(), 2u);
  SSTL_TEST_EQ(from_larger[0], 1);
  sstl::vector<int, 2> from_exact = sstl::from_array(range_values);
  SSTL_TEST_EQ(from_exact.size(), 2u);
  SSTL_TEST_EQ(from_exact[1], 2);

  sstl::vector<int, 4> self_swap;
  SSTL_TEST_ASSERT(self_swap.push_back(1));
  self_swap.swap(self_swap);
  SSTL_TEST_EQ(self_swap.size(), 1u);

  sstl::vector<int, 4> short_vector;
  sstl::vector<int, 4> long_vector;
  SSTL_TEST_ASSERT(short_vector.push_back(1));
  SSTL_TEST_ASSERT(long_vector.push_back(7));
  SSTL_TEST_ASSERT(long_vector.push_back(8));
  SSTL_TEST_ASSERT(long_vector.push_back(9));
  short_vector.swap(long_vector);
  SSTL_TEST_EQ(short_vector.size(), 3u);
  SSTL_TEST_EQ(long_vector.size(), 1u);
  SSTL_TEST_EQ(short_vector[2], 9);
  SSTL_TEST_EQ(long_vector[0], 1);
}

static void vector_comparisons_match_lexicographic_stl_shape() {
  sstl_test::noalloc_guard guard;

  /*
    Vector comparisons are intentionally cross-capacity: capacity is a storage
    property, not part of the logical sequence. These assertions mirror the STL
    sequence rules: size first matters only after all common elements compare
    equivalent.
  */
  sstl::vector<int, 2> left;
  sstl::vector<int, 4> same;
  sstl::vector<int, 4> prefix;
  sstl::vector<int, 4> greater_tail;
  SSTL_TEST_ASSERT(left.push_back(1));
  SSTL_TEST_ASSERT(left.push_back(2));
  SSTL_TEST_ASSERT(same.push_back(1));
  SSTL_TEST_ASSERT(same.push_back(2));
  SSTL_TEST_ASSERT(prefix.push_back(1));
  SSTL_TEST_ASSERT(greater_tail.push_back(1));
  SSTL_TEST_ASSERT(greater_tail.push_back(3));

  SSTL_TEST_ASSERT(left == same);
  SSTL_TEST_ASSERT(!(left != same));
  SSTL_TEST_ASSERT(prefix < left);
  SSTL_TEST_ASSERT(prefix <= left);
  SSTL_TEST_ASSERT(left <= same);
  SSTL_TEST_ASSERT(greater_tail > left);
  SSTL_TEST_ASSERT(greater_tail >= left);
  SSTL_TEST_ASSERT(left >= same);
  SSTL_TEST_ASSERT(!(left < prefix));
}

static void non_member_swap_delegates_to_fixed_storage_members() {
  sstl_test::noalloc_guard guard;

  /*
    Unqualified `sstl::swap` should select the container overload for vector and
    deque rather than the generic copy-swap template. The data pointers remain
    stable, proving the overload delegates to the member operation.
  */
  sstl::vector<int, 3> left_vector;
  sstl::vector<int, 3> right_vector;
  SSTL_TEST_ASSERT(left_vector.push_back(1));
  SSTL_TEST_ASSERT(right_vector.push_back(8));
  SSTL_TEST_ASSERT(right_vector.push_back(9));
  int* left_data = left_vector.data();
  int* right_data = right_vector.data();
  sstl::swap(left_vector, right_vector);
  SSTL_TEST_ASSERT(left_vector.data() == left_data);
  SSTL_TEST_ASSERT(right_vector.data() == right_data);
  SSTL_TEST_EQ(left_vector.size(), 2u);
  SSTL_TEST_EQ(left_vector[1], 9);
  SSTL_TEST_EQ(right_vector.size(), 1u);
  SSTL_TEST_EQ(right_vector[0], 1);

  sstl::deque<int, 3> left_deque;
  sstl::deque<int, 3> right_deque;
  SSTL_TEST_ASSERT(left_deque.push_back(2));
  SSTL_TEST_ASSERT(right_deque.push_back(6));
  SSTL_TEST_ASSERT(right_deque.push_back(7));
  sstl::swap(left_deque, right_deque);
  SSTL_TEST_EQ(left_deque.size(), 2u);
  SSTL_TEST_EQ(left_deque[0], 6);
  SSTL_TEST_EQ(left_deque[1], 7);
  SSTL_TEST_EQ(right_deque.size(), 1u);
  SSTL_TEST_EQ(right_deque[0], 2);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"swap_exchanges_values_not_storage", swap_exchanges_values_not_storage},
    {"vector_swap_uses_elementwise_tail_transfer", vector_swap_uses_elementwise_tail_transfer},
    {"deque_swap_uses_elementwise_tail_transfer", deque_swap_uses_elementwise_tail_transfer},
    {"vector_assign_insert_try_and_factories_cover_static_api", vector_assign_insert_try_and_factories_cover_static_api},
    {"vector_comparisons_match_lexicographic_stl_shape", vector_comparisons_match_lexicographic_stl_shape},
    {"non_member_swap_delegates_to_fixed_storage_members", non_member_swap_delegates_to_fixed_storage_members}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
