/* test_id: c99.list.extended_ops */
/* api_ids: [c.list.insert, c.list.pop_back, c.list.erase, c.list.remove, c.list.remove_if, c.list.unique, c.list.sort, c.list.merge, c.forward_list.before_begin, c.forward_list.insert_after, c.forward_list.erase_after, c.forward_list.splice_after, c.forward_list.remove, c.forward_list.remove_if, c.forward_list.unique, c.forward_list.sort, c.forward_list.merge] */
/* req_ids: [REQ-03, REQ-05, REQ-06, REQ-07] */
/* patterns: [AUDIT-NOALLOC, CAPACITY-BOUNDARY, C-API-PARITY] */
/*
 * Notes:
 *   - The C list macro preserves index insertion/erase for callers while storing elements in a node pool.
 *   - The C forward_list exposes after-position iterators over its own node pool.
 *   - Function-pointer predicates/comparators give C callers a generic way to remove and order arbitrary element types.
 *   - Merge is destination-owned and all-or-nothing under capacity pressure.
 */
#include <sstl/c/sstl_list.h>
#include <sstl/c/sstl_forward_list.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_LIST_DECLARE(c_ext_list, int, 6)
SSTL_LIST_DEFINE(c_ext_list, int, 6)
SSTL_FORWARD_LIST_DECLARE(c_ext_forward_list, int, 6)
SSTL_FORWARD_LIST_DEFINE(c_ext_forward_list, int, 6)

static bool c_is_even(const int* value) { return (*value % 2) == 0; }
static int c_counting_compare_calls = 0;
static int c_int_compare(const void* a, const void* b) {
  const int* left = (const int*)a;
  const int* right = (const int*)b;
  return (*left > *right) - (*left < *right);
}
static int c_counting_int_compare(const void* a, const void* b) {
  ++c_counting_compare_calls;
  return c_int_compare(a, b);
}

static int c_ext_list_value_at(c_ext_list* list, size_t index) {
  c_ext_list_iterator it = c_ext_list_begin(list);
  size_t walked = 0u;
  int* value = 0;
  while (walked != index && it != c_ext_list_end(list)) {
    it = c_ext_list_next(list, it);
    ++walked;
  }
  value = c_ext_list_at(list, it);
  SSTL_C_ASSERT(value != 0);
  return *value;
}

static int c_ext_forward_list_value_at(c_ext_forward_list* list, size_t index) {
  c_ext_forward_list_iterator it = c_ext_forward_list_begin(list);
  size_t walked = 0u;
  int* value = 0;
  while (walked != index && it != c_ext_forward_list_end(list)) {
    it = c_ext_forward_list_next(list, it);
    ++walked;
  }
  value = c_ext_forward_list_at(list, it);
  SSTL_C_ASSERT(value != 0);
  return *value;
}

static void list_insert_erase_remove_unique_sort_and_merge_work(void) {
  c_ext_list left;
  c_ext_list right;
  int out = 0;
  sstl_c_noalloc_begin();
  c_ext_list_init(&left);
  c_ext_list_init(&right);
  SSTL_C_ASSERT(c_ext_list_push_back(&left, 3));
  SSTL_C_ASSERT(c_ext_list_insert(&left, 0u, 1));
  SSTL_C_ASSERT(c_ext_list_insert(&left, 1u, 2));
  SSTL_C_ASSERT(c_ext_list_erase(&left, 1u, &out));
  SSTL_C_EQ(out, 2);
  SSTL_C_ASSERT(c_ext_list_push_back(&left, 3));
  c_ext_list_unique(&left);
  SSTL_C_EQ(c_ext_list_size(&left), 2u);
  SSTL_C_ASSERT(c_ext_list_push_back(&left, 2));
  SSTL_C_ASSERT(c_ext_list_pop_back(&left, &out));
  SSTL_C_EQ(out, 2);
  SSTL_C_ASSERT(c_ext_list_push_back(&left, 2));
  c_ext_list_remove_if(&left, c_is_even);
  c_ext_list_sort(&left, c_int_compare);
  SSTL_C_EQ(c_ext_list_value_at(&left, 0u), 1);
  SSTL_C_EQ(c_ext_list_value_at(&left, 1u), 3);
  SSTL_C_ASSERT(c_ext_list_push_back(&right, 2));
  SSTL_C_ASSERT(c_ext_list_push_back(&right, 4));
  c_counting_compare_calls = 0;
  SSTL_C_ASSERT(c_ext_list_merge(&left, &right, c_counting_int_compare));
  SSTL_C_ASSERT(c_counting_compare_calls <= 3);
  SSTL_C_EQ(c_ext_list_size(&right), 0u);
  SSTL_C_EQ(c_ext_list_value_at(&left, 0u), 1);
  SSTL_C_EQ(c_ext_list_value_at(&left, 1u), 2);
  SSTL_C_EQ(c_ext_list_value_at(&left, 2u), 3);
  SSTL_C_EQ(c_ext_list_value_at(&left, 3u), 4);
  c_ext_list_remove(&left, 3);
  SSTL_C_EQ(c_ext_list_size(&left), 3u);
  sstl_c_noalloc_end();
}

static void list_splice_one_and_range_work(void) {
  c_ext_list left;
  c_ext_list donor;
  c_ext_list_iterator before_four;
  sstl_c_noalloc_begin();
  c_ext_list_init(&left);
  c_ext_list_init(&donor);
  SSTL_C_ASSERT(c_ext_list_push_back(&left, 1));
  SSTL_C_ASSERT(c_ext_list_push_back(&left, 4));
  SSTL_C_ASSERT(c_ext_list_push_back(&donor, 2));
  SSTL_C_ASSERT(c_ext_list_push_back(&donor, 3));
  SSTL_C_ASSERT(c_ext_list_push_back(&donor, 5));
  before_four = c_ext_list_next(&left, c_ext_list_begin(&left));
  SSTL_C_ASSERT(c_ext_list_try_splice_one(&left, before_four, &donor, c_ext_list_begin(&donor)));
  SSTL_C_EQ(c_ext_list_value_at(&left, 0u), 1);
  SSTL_C_EQ(c_ext_list_value_at(&left, 1u), 2);
  SSTL_C_EQ(c_ext_list_value_at(&left, 2u), 4);
  SSTL_C_ASSERT(c_ext_list_splice_range(&left, c_ext_list_end(&left), &donor, c_ext_list_begin(&donor), c_ext_list_end(&donor)));
  SSTL_C_ASSERT(c_ext_list_empty(&donor));
  SSTL_C_EQ(c_ext_list_size(&left), 5u);
  SSTL_C_EQ(c_ext_list_value_at(&left, 3u), 3);
  SSTL_C_EQ(c_ext_list_value_at(&left, 4u), 5);
  sstl_c_noalloc_end();
}

static void list_node_iterators_survive_front_mutations(void) {
  c_ext_list list;
  c_ext_list_iterator second;
  int* second_ptr = 0;
  int out = 0;
  sstl_c_noalloc_begin();
  c_ext_list_init(&list);
  SSTL_C_ASSERT(c_ext_list_push_back(&list, 1));
  SSTL_C_ASSERT(c_ext_list_push_back(&list, 2));
  second = c_ext_list_next(&list, c_ext_list_begin(&list));
  second_ptr = c_ext_list_at(&list, second);
  SSTL_C_ASSERT(second_ptr != 0);
  SSTL_C_EQ(*second_ptr, 2);
  SSTL_C_ASSERT(c_ext_list_insert(&list, 0u, 0));
  SSTL_C_EQ(*c_ext_list_at(&list, second), 2);
  SSTL_C_ASSERT(c_ext_list_erase(&list, 0u, &out));
  SSTL_C_EQ(out, 0);
  SSTL_C_ASSERT(c_ext_list_at(&list, second) == second_ptr);
  SSTL_C_EQ(*second_ptr, 2);
  sstl_c_noalloc_end();
}

static void forward_list_after_position_surface_is_distinct(void) {
  c_ext_forward_list left;
  c_ext_forward_list right;
  c_ext_forward_list donor;
  c_ext_forward_list_iterator before;
  c_ext_forward_list_iterator pos;
  int out = 0;
  sstl_c_noalloc_begin();
  c_ext_forward_list_init(&left);
  c_ext_forward_list_init(&right);
  c_ext_forward_list_init(&donor);
  before = c_ext_forward_list_before_begin(&left);
  SSTL_C_ASSERT(c_ext_forward_list_insert_after(&left, before, 3) != c_ext_forward_list_end(&left));
  pos = c_ext_forward_list_insert_after(&left, before, 1);
  SSTL_C_ASSERT(pos != c_ext_forward_list_end(&left));
  SSTL_C_ASSERT(c_ext_forward_list_insert_after(&left, pos, 1) != c_ext_forward_list_end(&left));
  c_ext_forward_list_sort(&left, c_int_compare);
  c_ext_forward_list_unique(&left);
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 0u), 1);
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 1u), 3);
  pos = c_ext_forward_list_begin(&left);
  SSTL_C_ASSERT(c_ext_forward_list_insert_after(&left, pos, 2) != c_ext_forward_list_end(&left));
  SSTL_C_ASSERT(c_ext_forward_list_erase_after(&left, pos, &out) == c_ext_forward_list_next(&left, pos));
  SSTL_C_EQ(out, 2);
  SSTL_C_ASSERT(c_ext_forward_list_push_front(&right, 2));
  SSTL_C_ASSERT(c_ext_forward_list_merge(&left, &right, c_int_compare));
  SSTL_C_ASSERT(c_ext_forward_list_empty(&right));
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 0u), 1);
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 1u), 2);
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 2u), 3);
  c_ext_forward_list_remove_if(&left, c_is_even);
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 0u), 1);
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 1u), 3);
  SSTL_C_ASSERT(c_ext_forward_list_push_front(&donor, 5));
  SSTL_C_ASSERT(c_ext_forward_list_push_front(&donor, 4));
  SSTL_C_ASSERT(c_ext_forward_list_splice_after(&left, c_ext_forward_list_before_begin(&left), &donor));
  SSTL_C_ASSERT(c_ext_forward_list_empty(&donor));
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 0u), 4);
  SSTL_C_EQ(c_ext_forward_list_value_at(&left, 1u), 5);
  c_ext_forward_list_remove(&left, 5);
  SSTL_C_EQ(*c_ext_forward_list_front(&left), 4);
  sstl_c_noalloc_end();
}

static void forward_list_node_iterators_survive_head_mutations(void) {
  c_ext_forward_list list;
  c_ext_forward_list_iterator first;
  c_ext_forward_list_iterator second;
  int* second_ptr = 0;
  int out = 0;
  sstl_c_noalloc_begin();
  c_ext_forward_list_init(&list);
  SSTL_C_ASSERT(c_ext_forward_list_push_front(&list, 2));
  SSTL_C_ASSERT(c_ext_forward_list_push_front(&list, 1));
  first = c_ext_forward_list_begin(&list);
  second = c_ext_forward_list_next(&list, first);
  second_ptr = c_ext_forward_list_at(&list, second);
  SSTL_C_ASSERT(second_ptr != 0);
  SSTL_C_EQ(*second_ptr, 2);
  SSTL_C_ASSERT(c_ext_forward_list_insert_after(&list, c_ext_forward_list_before_begin(&list), 0) != c_ext_forward_list_end(&list));
  SSTL_C_EQ(*c_ext_forward_list_at(&list, second), 2);
  SSTL_C_ASSERT(c_ext_forward_list_erase_after(&list, c_ext_forward_list_before_begin(&list), &out) == first);
  SSTL_C_EQ(out, 0);
  SSTL_C_ASSERT(c_ext_forward_list_at(&list, second) == second_ptr);
  SSTL_C_EQ(*second_ptr, 2);
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"list_insert_erase_remove_unique_sort_and_merge_work", list_insert_erase_remove_unique_sort_and_merge_work},
    {"list_splice_one_and_range_work", list_splice_one_and_range_work},
    {"list_node_iterators_survive_front_mutations", list_node_iterators_survive_front_mutations},
    {"forward_list_after_position_surface_is_distinct", forward_list_after_position_surface_is_distinct},
    {"forward_list_node_iterators_survive_head_mutations", forward_list_node_iterators_survive_head_mutations}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
