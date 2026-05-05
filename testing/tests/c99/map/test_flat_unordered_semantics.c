/* test_id: c99.map.flat_unordered_semantics */
/* api_ids: [c.flat_map.contract, c.flat_map.lower_bound, c.flat_set.contract, c.flat_set.lower_bound, c.unordered_map.contract, c.unordered_map.bucket_count, c.unordered_set.contract, c.unordered_set.bucket_count] */
/* req_ids: [REQ-03, REQ-05, REQ-06, REQ-07] */
/* patterns: [AUDIT-NOALLOC, C-API-PARITY, CAPACITY-BOUNDARY] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - The flat containers are verified as sorted contiguous ranges, not linear unsorted maps/sets.
 *   - The unordered containers are verified as fixed bucket-chain containers, including a collision path.
 *   - All checks run under the no-allocation audit guard because these families must remain object-owned.
 */
#include <sstl/c/sstl_flat_map.h>
#include <sstl/c/sstl_flat_set.h>
#include <sstl/c/sstl_unordered_map.h>
#include <sstl/c/sstl_unordered_set.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_FLAT_MAP_DECLARE(c_sem_flat_map, int, int, 4, sstl_cmp_int_v)
SSTL_FLAT_MAP_DEFINE(c_sem_flat_map, int, int, 4, sstl_cmp_int_v)

SSTL_FLAT_SET_DECLARE(c_sem_flat_set, int, 4, sstl_cmp_int_v)
SSTL_FLAT_SET_DEFINE(c_sem_flat_set, int, 4, sstl_cmp_int_v)

SSTL_UNORDERED_MAP_DECLARE(c_sem_unordered_map, int, int, 4, 2, sstl_eq_int_v, sstl_hash_int_v)
SSTL_UNORDERED_MAP_DEFINE(c_sem_unordered_map, int, int, 4, 2, sstl_eq_int_v, sstl_hash_int_v)

SSTL_UNORDERED_SET_DECLARE(c_sem_unordered_set, int, 4, 2, sstl_eq_int_v, sstl_hash_int_v)
SSTL_UNORDERED_SET_DEFINE(c_sem_unordered_set, int, 4, 2, sstl_eq_int_v, sstl_hash_int_v)

static void flat_map_and_set_keep_sorted_unique_ranges(void) {
  c_sem_flat_map map;
  c_sem_flat_set set;
  sstl_c_noalloc_begin();
  c_sem_flat_map_init(&map);
  SSTL_C_ASSERT(c_sem_flat_map_insert(&map, 30, 300));
  SSTL_C_ASSERT(c_sem_flat_map_insert(&map, 10, 100));
  SSTL_C_ASSERT(c_sem_flat_map_insert(&map, 20, 200));
  SSTL_C_ASSERT(!c_sem_flat_map_insert(&map, 20, 999));
  SSTL_C_EQ(c_sem_flat_map_size(&map), 3u);
  SSTL_C_EQ(c_sem_flat_map_lower_bound(&map, 20), 1u);
  SSTL_C_EQ(map.data[0].key, 10);
  SSTL_C_EQ(map.data[1].key, 20);
  SSTL_C_EQ(map.data[2].key, 30);
  SSTL_C_EQ(*c_sem_flat_map_find(&map, 30), 300);
  SSTL_C_EQ(c_sem_flat_map_erase(&map, 20), 1u);
  SSTL_C_EQ(map.data[0].key, 10);
  SSTL_C_EQ(map.data[1].key, 30);

  c_sem_flat_set_init(&set);
  SSTL_C_ASSERT(c_sem_flat_set_insert(&set, 4));
  SSTL_C_ASSERT(c_sem_flat_set_insert(&set, 1));
  SSTL_C_ASSERT(c_sem_flat_set_insert(&set, 3));
  SSTL_C_ASSERT(!c_sem_flat_set_insert(&set, 3));
  SSTL_C_EQ(c_sem_flat_set_lower_bound(&set, 3), 1u);
  SSTL_C_EQ(set.data[0], 1);
  SSTL_C_EQ(set.data[1], 3);
  SSTL_C_EQ(set.data[2], 4);
  SSTL_C_ASSERT(c_sem_flat_set_find(&set, 4) != 0);
  SSTL_C_EQ(c_sem_flat_set_erase(&set, 3), 1u);
  SSTL_C_EQ(set.data[1], 4);
  sstl_c_noalloc_end();
}

static void unordered_map_and_set_use_fixed_bucket_chains(void) {
  c_sem_unordered_map map;
  c_sem_unordered_set set;
  size_t bucket = 0u;
  int first = -1;
  int second = -1;
  int candidate;
  sstl_c_noalloc_begin();
  for (candidate = 0; candidate != 64; ++candidate) {
    size_t current = c_sem_unordered_map_bucket_index(candidate);
    if (first < 0) {
      first = candidate;
      bucket = current;
    } else if (current == bucket) {
      second = candidate;
      break;
    }
  }
  SSTL_C_ASSERT(first >= 0);
  SSTL_C_ASSERT(second >= 0);

  c_sem_unordered_map_init(&map);
  SSTL_C_EQ(c_sem_unordered_map_bucket_count(&map), 2u);
  SSTL_C_EQ(map.free_head, 0u);
  SSTL_C_ASSERT(c_sem_unordered_map_insert(&map, first, first + 100));
  SSTL_C_ASSERT(c_sem_unordered_map_insert(&map, second, second + 100));
  SSTL_C_EQ(c_sem_unordered_map_size(&map), 2u);
  SSTL_C_EQ(map.free_head, 2u);
  SSTL_C_ASSERT(map.buckets[bucket] != (size_t)-1);
  SSTL_C_ASSERT(map.next[map.buckets[bucket]] != (size_t)-1);
  SSTL_C_EQ(*c_sem_unordered_map_find(&map, first), first + 100);
  SSTL_C_EQ(*c_sem_unordered_map_find(&map, second), second + 100);
  SSTL_C_EQ(c_sem_unordered_map_erase(&map, first), 1u);
  SSTL_C_EQ(map.free_head, 0u);
  SSTL_C_ASSERT(c_sem_unordered_map_find(&map, first) == 0);
  SSTL_C_EQ(*c_sem_unordered_map_find(&map, second), second + 100);
  SSTL_C_ASSERT(c_sem_unordered_map_insert(&map, first + 64, first + 164));
  SSTL_C_EQ(c_sem_unordered_map_find_slot(&map, first + 64), 0u);

  c_sem_unordered_set_init(&set);
  SSTL_C_EQ(c_sem_unordered_set_bucket_count(&set), 2u);
  SSTL_C_EQ(set.free_head, 0u);
  SSTL_C_ASSERT(c_sem_unordered_set_insert(&set, first));
  SSTL_C_ASSERT(c_sem_unordered_set_insert(&set, second));
  SSTL_C_EQ(set.free_head, 2u);
  SSTL_C_ASSERT(!c_sem_unordered_set_insert(&set, second));
  SSTL_C_ASSERT(c_sem_unordered_set_find(&set, first) != 0);
  SSTL_C_ASSERT(c_sem_unordered_set_find(&set, second) != 0);
  SSTL_C_EQ(c_sem_unordered_set_erase(&set, first), 1u);
  SSTL_C_EQ(set.free_head, 0u);
  SSTL_C_ASSERT(c_sem_unordered_set_find(&set, first) == 0);
  SSTL_C_ASSERT(c_sem_unordered_set_find(&set, second) != 0);
  SSTL_C_ASSERT(c_sem_unordered_set_insert(&set, first + 64));
  SSTL_C_EQ(c_sem_unordered_set_find_slot(&set, first + 64), 0u);
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"flat_map_and_set_keep_sorted_unique_ranges", flat_map_and_set_keep_sorted_unique_ranges},
    {"unordered_map_and_set_use_fixed_bucket_chains", unordered_map_and_set_use_fixed_bucket_chains}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
