/* test_id: c99.algorithm.contracts */
/* api_ids: [c.algorithm.contract] */
/* req_ids: [REQ-01, REQ-07, REQ-10] */
/* patterns: [C-API-PARITY, STL-COMPILE-PARITY, NO-HEAP-ALLOCATION] */
/*
 * Notes:
 *   - This test is intentionally broad because the C algorithm API is macro generated.
 *   - The test instantiates the generated API for integers and then walks every public algorithm group.
 *   - The integer family validates the full comparison, movement, set, binary-search, heap, and numeric surface.
 *   - A second struct family proves that the non-numeric algorithms are not limited to scalar arithmetic types.
 *   - No output buffer is allocated by the library; every destination array below is caller-owned automatic storage.
 */
#include <stdbool.h>
#include <stddef.h>

#include <sstl/c/sstl_algorithm.h>

#include "test_harness.h"

typedef struct stable_item {
  int key;
  int original_order;
} stable_item;

static int int_ptr_cmp(const void* a, const void* b) {
  const int* left = (const int*)a;
  const int* right = (const int*)b;
  return (*left > *right) - (*left < *right);
}

static int stable_item_cmp(const void* a, const void* b) {
  const stable_item* left = (const stable_item*)a;
  const stable_item* right = (const stable_item*)b;
  return (left->key > right->key) - (left->key < right->key);
}

static bool int_is_even(const void* value) { return (*(const int*)value % 2) == 0; }
static bool int_is_odd(const void* value) { return (*(const int*)value % 2) != 0; }
static bool int_is_positive(const void* value) { return *(const int*)value > 0; }
static bool int_less_than_four(const void* value) { return *(const int*)value < 4; }
static void int_times_two(void* dst, const void* src) { *(int*)dst = *(const int*)src * 2; }

SSTL_ALGORITHM_DECLARE(int_alg, int)
SSTL_ALGORITHM_DEFINE(int_alg, int, int_ptr_cmp)

SSTL_NUMERIC_ALGORITHM_DECLARE(int_num, int)
SSTL_NUMERIC_ALGORITHM_DEFINE(int_num, int)

SSTL_ALGORITHM_DECLARE(item_alg, stable_item)
SSTL_ALGORITHM_DEFINE(item_alg, stable_item, stable_item_cmp)

static void assert_range_equals(const int* actual, const int* expected, size_t n) {
  size_t i;
  for (i = 0u; i != n; ++i) {
    SSTL_C_ASSERT(actual[i] == expected[i]);
  }
}

static void test_non_modifying_algorithms(void) {
  /*
   * Non-mutating algorithms should behave like iterator algorithms: they return
   * the matching pointer, count matches, or report a boolean answer without
   * touching the source range.
   */
  int values[] = {1, 2, 3, 2, 4, 5};
  int same[] = {1, 2, 3, 2, 4, 5};
  int different[] = {1, 2, 99, 2, 4, 5};
  int needle[] = {3, 2, 4};
  int missing_needle[] = {2, 4, 9};
  int two = 2;
  int six = 6;

  SSTL_C_ASSERT(int_alg_find(values, values + 6, &two) == values + 1);
  SSTL_C_ASSERT(int_alg_find(values, values + 6, &six) == values + 6);
  SSTL_C_ASSERT(int_alg_find_if(values, values + 6, int_is_even) == values + 1);
  SSTL_C_ASSERT(int_alg_find_if_not(values, values + 6, int_less_than_four) == values + 4);
  SSTL_C_ASSERT(int_alg_all_of(values, values + 6, int_is_positive));
  SSTL_C_ASSERT(int_alg_any_of(values, values + 6, int_is_even));
  SSTL_C_ASSERT(int_alg_none_of(values, values + 6, int_is_odd) == false);
  SSTL_C_ASSERT(int_alg_count(values, values + 6, &two) == 2u);
  SSTL_C_ASSERT(int_alg_count_if(values, values + 6, int_is_even) == 3u);
  SSTL_C_ASSERT(int_alg_equal(values, values + 6, same));
  SSTL_C_ASSERT(!int_alg_equal(values, values + 6, different));
  SSTL_C_ASSERT(int_alg_mismatch_first(values, values + 6, different) == values + 2);
  SSTL_C_ASSERT(int_alg_mismatch_second(values, values + 6, different) == different + 2);
  SSTL_C_ASSERT(int_alg_search(values, values + 6, needle, needle + 3) == values + 2);
  SSTL_C_ASSERT(int_alg_search(values, values + 6, missing_needle, missing_needle + 3) == values + 6);
  SSTL_C_ASSERT(int_alg_lexicographical_compare(values, values + 3, different, different + 3));
  SSTL_C_ASSERT(int_alg_min_element(values, values + 6) == values);
  SSTL_C_ASSERT(int_alg_max_element(values, values + 6) == values + 5);
  SSTL_C_ASSERT(int_alg_minmax_first(values, values + 6) == values);
  SSTL_C_ASSERT(int_alg_minmax_second(values, values + 6) == values + 5);
}

static void test_copy_fill_transform_and_mutation_algorithms(void) {
  /*
   * These algorithms all write into ranges owned by the caller. The assertions
   * check both returned end pointers and the observable contents after each
   * operation, because either part can break independently.
   */
  int source[] = {1, 2, 3, 4, 5};
  int out[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  int expected_copy[] = {1, 2, 3, 4, 5};
  int expected_even_copy[] = {2, 4};
  int expected_transform[] = {2, 4, 6, 8, 10};
  int expected_backward[] = {1, 2, 1, 2, 3};
  int expected_rotate[] = {3, 4, 5, 1, 2};
  int expected_unique[] = {1, 2, 3, 4};
  int value_two = 2;
  int value_three = 3;
  int fill_value = 7;
  int backward[] = {1, 2, 3, 0, 0};
  int replace_values[] = {1, 2, 3, 2, 4};
  int remove_values[] = {1, 2, 2, 3, 4};
  int remove_if_values[] = {1, 2, 3, 4, 5};
  int reverse_values[] = {1, 2, 3, 4};
  int rotate_values[] = {1, 2, 3, 4, 5};
  int swap_a[] = {1, 2, 3};
  int swap_b[] = {4, 5, 6};
  int unique_values[] = {1, 1, 2, 2, 2, 3, 4, 4};
  int* new_end;

  SSTL_C_ASSERT(int_alg_copy(source, source + 5, out) == out + 5);
  assert_range_equals(out, expected_copy, 5u);
  SSTL_C_ASSERT(int_alg_copy_n(source, 3u, out) == out + 3);
  SSTL_C_ASSERT(int_alg_copy_if(source, source + 5, out, int_is_even) == out + 2);
  assert_range_equals(out, expected_even_copy, 2u);
  SSTL_C_ASSERT(int_alg_copy_backward(backward, backward + 3, backward + 5) == backward + 2);
  assert_range_equals(backward, expected_backward, 5u);

  int_alg_fill(out, out + 4, fill_value);
  SSTL_C_ASSERT(out[0] == 7 && out[3] == 7);
  SSTL_C_ASSERT(int_alg_fill_n(out, 3u, value_three) == out + 3);
  SSTL_C_ASSERT(out[0] == 3 && out[2] == 3 && out[3] == 7);
  SSTL_C_ASSERT(int_alg_transform(source, source + 5, out, int_times_two) == out + 5);
  assert_range_equals(out, expected_transform, 5u);

  int_alg_replace(replace_values, replace_values + 5, &value_two, 9);
  SSTL_C_ASSERT(replace_values[1] == 9 && replace_values[3] == 9);
  int_alg_replace_if(replace_values, replace_values + 5, int_is_odd, 8);
  SSTL_C_ASSERT(replace_values[0] == 8 && replace_values[2] == 8 && replace_values[4] == 4);

  new_end = int_alg_remove(remove_values, remove_values + 5, &value_two);
  SSTL_C_ASSERT(new_end == remove_values + 3);
  SSTL_C_ASSERT(remove_values[0] == 1 && remove_values[1] == 3 && remove_values[2] == 4);
  new_end = int_alg_remove_if(remove_if_values, remove_if_values + 5, int_is_even);
  SSTL_C_ASSERT(new_end == remove_if_values + 3);
  SSTL_C_ASSERT(remove_if_values[0] == 1 && remove_if_values[1] == 3 && remove_if_values[2] == 5);

  int_alg_iter_swap(reverse_values, reverse_values + 3);
  SSTL_C_ASSERT(reverse_values[0] == 4 && reverse_values[3] == 1);
  int_alg_reverse(reverse_values, reverse_values + 4);
  SSTL_C_ASSERT(reverse_values[0] == 1 && reverse_values[3] == 4);
  SSTL_C_ASSERT(int_alg_rotate(rotate_values, rotate_values + 2, rotate_values + 5) == rotate_values + 3);
  assert_range_equals(rotate_values, expected_rotate, 5u);
  SSTL_C_ASSERT(int_alg_swap_ranges(swap_a, swap_a + 3, swap_b) == swap_b + 3);
  SSTL_C_ASSERT(swap_a[0] == 4 && swap_b[0] == 1 && swap_a[2] == 6 && swap_b[2] == 3);
  new_end = int_alg_unique(unique_values, unique_values + 8);
  SSTL_C_ASSERT(new_end == unique_values + 4);
  assert_range_equals(unique_values, expected_unique, 4u);
}

static void test_ordering_heap_and_partition_algorithms(void) {
  /*
   * Ordering algorithms are validated by their postconditions instead of by
   * assuming a particular internal sorting strategy. The stable-sort struct
   * check is the exception: it confirms equal keys retain their input order.
   */
  int sortable[] = {5, 1, 4, 2, 3};
  int expected_sorted[] = {1, 2, 3, 4, 5};
  int partial[] = {9, 1, 8, 2, 7, 3};
  int nth[] = {4, 1, 5, 2, 3};
  int heap[] = {3, 1, 4, 2, 5};
  int partition_values[] = {1, 2, 3, 4, 5, 6};
  int stable_partition_values[] = {1, 2, 3, 4, 5, 6};
  stable_item items[] = {{2, 0}, {1, 1}, {2, 2}, {1, 3}, {2, 4}};
  stable_item recursive_items[] = {{3, 0}, {1, 1}, {2, 2}, {3, 3}, {1, 4}, {2, 5}, {3, 6}, {1, 7}};
  int* split;
  size_t i;

  SSTL_C_ASSERT(!int_alg_is_sorted(sortable, sortable + 5));
  SSTL_C_ASSERT(int_alg_is_sorted_until(sortable, sortable + 5) == sortable + 1);
  int_alg_sort(sortable, sortable + 5);
  assert_range_equals(sortable, expected_sorted, 5u);
  SSTL_C_ASSERT(int_alg_is_sorted(sortable, sortable + 5));

  item_alg_stable_sort(items, items + 5);
  SSTL_C_ASSERT(items[0].key == 1 && items[0].original_order == 1);
  SSTL_C_ASSERT(items[1].key == 1 && items[1].original_order == 3);
  SSTL_C_ASSERT(items[2].key == 2 && items[2].original_order == 0);
  SSTL_C_ASSERT(items[3].key == 2 && items[3].original_order == 2);
  SSTL_C_ASSERT(items[4].key == 2 && items[4].original_order == 4);
  item_alg_stable_sort(recursive_items, recursive_items + 8);
  SSTL_C_ASSERT(recursive_items[0].key == 1 && recursive_items[0].original_order == 1);
  SSTL_C_ASSERT(recursive_items[1].key == 1 && recursive_items[1].original_order == 4);
  SSTL_C_ASSERT(recursive_items[2].key == 1 && recursive_items[2].original_order == 7);
  SSTL_C_ASSERT(recursive_items[3].key == 2 && recursive_items[3].original_order == 2);
  SSTL_C_ASSERT(recursive_items[4].key == 2 && recursive_items[4].original_order == 5);
  SSTL_C_ASSERT(recursive_items[5].key == 3 && recursive_items[5].original_order == 0);
  SSTL_C_ASSERT(recursive_items[6].key == 3 && recursive_items[6].original_order == 3);
  SSTL_C_ASSERT(recursive_items[7].key == 3 && recursive_items[7].original_order == 6);

  int_alg_partial_sort(partial, partial + 3, partial + 6);
  SSTL_C_ASSERT(partial[0] == 1 && partial[1] == 2 && partial[2] == 3);
  int_alg_nth_element(nth, nth + 2, nth + 5);
  SSTL_C_ASSERT(nth[2] == 3);

  int_alg_make_heap(heap, heap + 4);
  SSTL_C_ASSERT(heap[0] == 4);
  int_alg_push_heap(heap, heap + 5);
  SSTL_C_ASSERT(heap[0] == 5);
  int_alg_pop_heap(heap, heap + 5);
  SSTL_C_ASSERT(heap[4] == 5);
  int_alg_sort_heap(heap, heap + 4);
  SSTL_C_ASSERT(heap[0] == 1 && heap[1] == 2 && heap[2] == 3 && heap[3] == 4);

  split = int_alg_partition(partition_values, partition_values + 6, int_is_even);
  for (i = 0u; i != (size_t)(split - partition_values); ++i) SSTL_C_ASSERT(int_is_even(&partition_values[i]));
  for (; i != 6u; ++i) SSTL_C_ASSERT(!int_is_even(&partition_values[i]));

  split = int_alg_stable_partition(stable_partition_values, stable_partition_values + 6, int_is_even);
  SSTL_C_ASSERT(split == stable_partition_values + 3);
  SSTL_C_ASSERT(stable_partition_values[0] == 2 && stable_partition_values[1] == 4 && stable_partition_values[2] == 6);
  SSTL_C_ASSERT(stable_partition_values[3] == 1 && stable_partition_values[4] == 3 && stable_partition_values[5] == 5);
}

static void test_merge_set_binary_and_numeric_algorithms(void) {
  /*
   * The sorted-range algorithms share the same comparator contract. This group
   * checks lower/upper/equal-range boundaries, membership queries, set algebra,
   * and the arithmetic-only numeric macro family.
   */
  int a[] = {1, 2, 4, 6};
  int b[] = {2, 3, 4, 5};
  int merged[8] = {0};
  int union_out[8] = {0};
  int intersection_out[4] = {0};
  int difference_out[4] = {0};
  int symmetric_out[8] = {0};
  int sorted[] = {1, 2, 2, 2, 3, 4};
  int full[] = {1, 1, 2, 3, 5};
  int subset_ok[] = {1, 2, 5};
  int subset_bad[] = {1, 2, 4};
  int numeric[] = {1, 2, 3, 4};
  int numeric_rhs[] = {5, 6, 7, 8};
  int numeric_out[4] = {0};
  int expected_merge[] = {1, 2, 2, 3, 4, 4, 5, 6};
  int expected_union[] = {1, 2, 3, 4, 5, 6};
  int expected_intersection[] = {2, 4};
  int expected_difference[] = {1, 6};
  int expected_symmetric[] = {1, 3, 5, 6};
  int expected_partial_sum[] = {1, 3, 6, 10};
  int expected_adjacent_difference[] = {1, 1, 1, 1};
  int two = 2;
  int four = 4;

  SSTL_C_ASSERT(int_alg_merge(a, a + 4, b, b + 4, merged) == merged + 8);
  assert_range_equals(merged, expected_merge, 8u);
  SSTL_C_ASSERT(int_alg_includes(full, full + 5, subset_ok, subset_ok + 3));
  SSTL_C_ASSERT(!int_alg_includes(full, full + 5, subset_bad, subset_bad + 3));
  SSTL_C_ASSERT(int_alg_set_union(a, a + 4, b, b + 4, union_out) == union_out + 6);
  assert_range_equals(union_out, expected_union, 6u);
  SSTL_C_ASSERT(int_alg_set_intersection(a, a + 4, b, b + 4, intersection_out) == intersection_out + 2);
  assert_range_equals(intersection_out, expected_intersection, 2u);
  SSTL_C_ASSERT(int_alg_set_difference(a, a + 4, b, b + 4, difference_out) == difference_out + 2);
  assert_range_equals(difference_out, expected_difference, 2u);
  SSTL_C_ASSERT(int_alg_set_symmetric_difference(a, a + 4, b, b + 4, symmetric_out) == symmetric_out + 4);
  assert_range_equals(symmetric_out, expected_symmetric, 4u);

  SSTL_C_ASSERT(int_alg_lower_bound(sorted, sorted + 6, &two) == sorted + 1);
  SSTL_C_ASSERT(int_alg_upper_bound(sorted, sorted + 6, &two) == sorted + 4);
  SSTL_C_ASSERT(int_alg_equal_range_first(sorted, sorted + 6, &two) == sorted + 1);
  SSTL_C_ASSERT(int_alg_equal_range_second(sorted, sorted + 6, &two) == sorted + 4);
  SSTL_C_ASSERT(int_alg_binary_search(sorted, sorted + 6, &four));

  SSTL_C_ASSERT(int_num_accumulate(numeric, numeric + 4, 0) == 10);
  SSTL_C_ASSERT(int_num_inner_product(numeric, numeric + 4, numeric_rhs, 0) == 70);
  SSTL_C_ASSERT(int_num_partial_sum(numeric, numeric + 4, numeric_out) == numeric_out + 4);
  assert_range_equals(numeric_out, expected_partial_sum, 4u);
  SSTL_C_ASSERT(int_num_adjacent_difference(numeric, numeric + 4, numeric_out) == numeric_out + 4);
  assert_range_equals(numeric_out, expected_adjacent_difference, 4u);
  int_num_iota(numeric_out, numeric_out + 4, 7);
  SSTL_C_ASSERT(numeric_out[0] == 7 && numeric_out[1] == 8 && numeric_out[2] == 9 && numeric_out[3] == 10);
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"non_modifying_algorithms", test_non_modifying_algorithms},
    {"copy_fill_transform_and_mutation_algorithms", test_copy_fill_transform_and_mutation_algorithms},
    {"ordering_heap_and_partition_algorithms", test_ordering_heap_and_partition_algorithms},
    {"merge_set_binary_and_numeric_algorithms", test_merge_set_binary_and_numeric_algorithms}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
