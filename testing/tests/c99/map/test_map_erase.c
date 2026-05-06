/* test_id: c99.map.erase */
/* api_ids: [c.map.contract, c.map.erase, c.set.contract, c.set.erase] */
/* req_ids: [REQ-05, REQ-06, REQ-07] */
/* patterns: [AUDIT-NOALLOC, C-API-PARITY] */
/*
 * Notes:
 *   - C map erase is key-based because the generated C API has no iterator object.
 *   - The return value mirrors associative-container erase: zero when absent, one when removed.
 *   - Remaining entries must still be findable after the fixed node tree is rebalanced.
 *   - Public begin/next/at checks prove ordered traversal rather than insertion-order storage.
 *   - The larger-capacity case erases leaves, one-child nodes, and two-child nodes so the
 *     in-place red/black erase path cannot quietly fall back to stack-backed rebuilding.
 */
#include <sstl/c/sstl_map.h>
#include <sstl/c/sstl_set.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_MAP_DECLARE(c_ext_map, int, int, 4, sstl_cmp_int_v)
SSTL_MAP_DEFINE(c_ext_map, int, int, 4, sstl_cmp_int_v)
SSTL_MAP_DECLARE(c_ext_map8, int, int, 8, sstl_cmp_int_v)
SSTL_MAP_DEFINE(c_ext_map8, int, int, 8, sstl_cmp_int_v)
SSTL_SET_DECLARE(c_ext_set, int, 4, sstl_cmp_int_v)
SSTL_SET_DEFINE(c_ext_set, int, 4, sstl_cmp_int_v)
SSTL_SET_DECLARE(c_ext_set8, int, 8, sstl_cmp_int_v)
SSTL_SET_DEFINE(c_ext_set8, int, 8, sstl_cmp_int_v)

static void assert_next_map_entry(c_ext_map* m, c_ext_map_iterator* it, int key, int value) {
  c_ext_map_pair* entry = c_ext_map_at(m, *it);
  SSTL_C_ASSERT(entry != 0);
  SSTL_C_EQ(entry->key, key);
  SSTL_C_EQ(entry->value, value);
  *it = c_ext_map_next(m, *it);
}

static void assert_next_set_value(c_ext_set* s, c_ext_set_iterator* it, int key) {
  int* value = c_ext_set_at(s, *it);
  SSTL_C_ASSERT(value != 0);
  SSTL_C_EQ(*value, key);
  *it = c_ext_set_next(s, *it);
}

static void assert_next_map8_entry(c_ext_map8* m, c_ext_map8_iterator* it, int key, int value) {
  c_ext_map8_pair* entry = c_ext_map8_at(m, *it);
  SSTL_C_ASSERT(entry != 0);
  SSTL_C_EQ(entry->key, key);
  SSTL_C_EQ(entry->value, value);
  *it = c_ext_map8_next(m, *it);
}

static void assert_next_set8_value(c_ext_set8* s, c_ext_set8_iterator* it, int key) {
  int* value = c_ext_set8_at(s, *it);
  SSTL_C_ASSERT(value != 0);
  SSTL_C_EQ(*value, key);
  *it = c_ext_set8_next(s, *it);
}

static void erase_by_key_reports_count_and_preserves_remaining_entries(void) {
  c_ext_map m;
  sstl_c_noalloc_begin();
  c_ext_map_init(&m);
  SSTL_C_ASSERT(c_ext_map_insert(&m, 1, 10));
  SSTL_C_ASSERT(c_ext_map_insert(&m, 2, 20));
  SSTL_C_ASSERT(c_ext_map_insert(&m, 3, 30));
  SSTL_C_EQ(c_ext_map_erase(&m, 2), 1u);
  SSTL_C_EQ(c_ext_map_erase(&m, 2), 0u);
  SSTL_C_ASSERT(c_ext_map_find(&m, 2) == 0);
  SSTL_C_EQ(*c_ext_map_find(&m, 1), 10);
  SSTL_C_EQ(*c_ext_map_find(&m, 3), 30);
  SSTL_C_EQ(c_ext_map_size(&m), 2u);
  sstl_c_noalloc_end();
}

static void map_iteration_is_sorted_by_key(void) {
  c_ext_map m;
  c_ext_map_iterator it;
  sstl_c_noalloc_begin();
  c_ext_map_init(&m);
  SSTL_C_ASSERT(c_ext_map_insert(&m, 3, 30));
  SSTL_C_ASSERT(c_ext_map_insert(&m, 1, 10));
  SSTL_C_ASSERT(c_ext_map_insert(&m, 4, 40));
  SSTL_C_ASSERT(c_ext_map_insert(&m, 2, 20));
  SSTL_C_ASSERT(!c_ext_map_insert(&m, 2, 200));
  SSTL_C_ASSERT(c_ext_map_full(&m));
  it = c_ext_map_begin(&m);
  assert_next_map_entry(&m, &it, 1, 10);
  assert_next_map_entry(&m, &it, 2, 20);
  assert_next_map_entry(&m, &it, 3, 30);
  assert_next_map_entry(&m, &it, 4, 40);
  SSTL_C_EQ(it, c_ext_map_end(&m));
  sstl_c_noalloc_end();
}

static void set_iteration_is_sorted_and_erase_preserves_tree_search(void) {
  c_ext_set s;
  c_ext_set_iterator it;
  sstl_c_noalloc_begin();
  c_ext_set_init(&s);
  SSTL_C_ASSERT(c_ext_set_insert(&s, 4));
  SSTL_C_ASSERT(c_ext_set_insert(&s, 1));
  SSTL_C_ASSERT(c_ext_set_insert(&s, 3));
  SSTL_C_ASSERT(c_ext_set_insert(&s, 2));
  SSTL_C_ASSERT(!c_ext_set_insert(&s, 2));
  SSTL_C_EQ(c_ext_set_size(&s), 4u);
  SSTL_C_EQ(c_ext_set_erase(&s, 3), 1u);
  SSTL_C_EQ(c_ext_set_erase(&s, 3), 0u);
  SSTL_C_ASSERT(c_ext_set_find(&s, 3) == 0);
  SSTL_C_ASSERT(c_ext_set_find(&s, 4) != 0);
  it = c_ext_set_begin(&s);
  assert_next_set_value(&s, &it, 1);
  assert_next_set_value(&s, &it, 2);
  assert_next_set_value(&s, &it, 4);
  SSTL_C_EQ(it, c_ext_set_end(&s));
  sstl_c_noalloc_end();
}

static void public_iterators_to_successor_survive_two_child_erase(void) {
  c_ext_map8 m;
  c_ext_map8_iterator map_it;
  c_ext_map8_pair* entry;
  c_ext_set8 s;
  c_ext_set8_iterator set_it;
  int* value;

  sstl_c_noalloc_begin();
  c_ext_map8_init(&m);
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 2, 20));
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 1, 10));
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 3, 30));
  map_it = c_ext_map8_lower_bound(&m, 3);
  SSTL_C_EQ(c_ext_map8_erase(&m, 2), 1u);
  entry = c_ext_map8_at(&m, map_it);
  SSTL_C_ASSERT(entry != 0);
  SSTL_C_EQ(entry->key, 3);
  SSTL_C_EQ(entry->value, 30);

  c_ext_set8_init(&s);
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 2));
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 1));
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 3));
  set_it = c_ext_set8_lower_bound(&s, 3);
  SSTL_C_EQ(c_ext_set8_erase(&s, 2), 1u);
  value = c_ext_set8_at(&s, set_it);
  SSTL_C_ASSERT(value != 0);
  SSTL_C_EQ(*value, 3);
  sstl_c_noalloc_end();
}

static void in_place_tree_erase_handles_rebalance_shapes(void) {
  c_ext_map8 m;
  c_ext_map8_iterator map_it;
  c_ext_set8 s;
  c_ext_set8_iterator set_it;

  sstl_c_noalloc_begin();
  c_ext_map8_init(&m);
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 4, 40));
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 2, 20));
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 6, 60));
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 1, 10));
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 3, 30));
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 5, 50));
  SSTL_C_ASSERT(c_ext_map8_insert(&m, 7, 70));

  SSTL_C_EQ(c_ext_map8_erase(&m, 1), 1u);
  SSTL_C_EQ(c_ext_map8_erase(&m, 6), 1u);
  SSTL_C_EQ(c_ext_map8_erase(&m, 4), 1u);
  SSTL_C_EQ(c_ext_map8_erase(&m, 42), 0u);
  SSTL_C_ASSERT(c_ext_map8_find(&m, 1) == 0);
  SSTL_C_ASSERT(c_ext_map8_find(&m, 4) == 0);
  SSTL_C_ASSERT(c_ext_map8_find(&m, 6) == 0);
  SSTL_C_EQ(c_ext_map8_size(&m), 4u);

  map_it = c_ext_map8_begin(&m);
  assert_next_map8_entry(&m, &map_it, 2, 20);
  assert_next_map8_entry(&m, &map_it, 3, 30);
  assert_next_map8_entry(&m, &map_it, 5, 50);
  assert_next_map8_entry(&m, &map_it, 7, 70);
  SSTL_C_EQ(map_it, c_ext_map8_end(&m));

  c_ext_set8_init(&s);
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 4));
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 2));
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 6));
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 1));
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 3));
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 5));
  SSTL_C_ASSERT(c_ext_set8_insert(&s, 7));
  SSTL_C_EQ(c_ext_set8_erase(&s, 1), 1u);
  SSTL_C_EQ(c_ext_set8_erase(&s, 6), 1u);
  SSTL_C_EQ(c_ext_set8_erase(&s, 4), 1u);
  SSTL_C_EQ(c_ext_set8_erase(&s, 42), 0u);
  SSTL_C_ASSERT(c_ext_set8_find(&s, 1) == 0);
  SSTL_C_ASSERT(c_ext_set8_find(&s, 4) == 0);
  SSTL_C_ASSERT(c_ext_set8_find(&s, 6) == 0);
  SSTL_C_EQ(c_ext_set8_size(&s), 4u);

  set_it = c_ext_set8_begin(&s);
  assert_next_set8_value(&s, &set_it, 2);
  assert_next_set8_value(&s, &set_it, 3);
  assert_next_set8_value(&s, &set_it, 5);
  assert_next_set8_value(&s, &set_it, 7);
  SSTL_C_EQ(set_it, c_ext_set8_end(&s));
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"erase_by_key_reports_count_and_preserves_remaining_entries", erase_by_key_reports_count_and_preserves_remaining_entries},
    {"map_iteration_is_sorted_by_key", map_iteration_is_sorted_by_key},
    {"set_iteration_is_sorted_and_erase_preserves_tree_search", set_iteration_is_sorted_and_erase_preserves_tree_search},
    {"public_iterators_to_successor_survive_two_child_erase", public_iterators_to_successor_survive_two_child_erase},
    {"in_place_tree_erase_handles_rebalance_shapes", in_place_tree_erase_handles_rebalance_shapes}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
