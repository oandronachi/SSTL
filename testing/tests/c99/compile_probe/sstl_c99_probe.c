/* test_id: c99.compile.public_headers */
/* api_ids: [c.public_headers, c.array.contract, c.array.cmp, c.span.contract, c.string_view.contract, c.deque.contract, c.queue.contract, c.stack.contract, c.priority_queue.contract, c.list.contract, c.forward_list.contract, c.map.contract, c.set.contract, c.set.erase, c.flat_map.contract, c.flat_set.contract, c.unordered_map.contract, c.unordered_set.contract, c.bitset.contract, c.optional.contract, c.variant.contract, c.function.contract, c.algorithm.contract, c.owning.try_capacity, c.hooks.void_callbacks] */
/* req_ids: [REQ-01, REQ-07, REQ-10] */
/* patterns: [STL-COMPILE-PARITY, C-API-PARITY, FREESTANDING-COMPILE] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It is a compile/link contract test for the generated C99 typed APIs.
 *   - Every public C macro family is instantiated and lightly exercised so an empty header cannot pass by doing nothing.
 *   - Runtime checks remain small because the deeper behavioral tests live beside the individual families.
 */
#include <sstl/c/sstl_array.h>
#include <sstl/c/sstl_vector.h>
#include <sstl/c/sstl_string.h>
#include <sstl/c/sstl_string_view.h>
#include <sstl/c/sstl_span.h>
#include <sstl/c/sstl_deque.h>
#include <sstl/c/sstl_queue.h>
#include <sstl/c/sstl_stack.h>
#include <sstl/c/sstl_priority_queue.h>
#include <sstl/c/sstl_list.h>
#include <sstl/c/sstl_forward_list.h>
#include <sstl/c/sstl_map.h>
#include <sstl/c/sstl_set.h>
#include <sstl/c/sstl_flat_map.h>
#include <sstl/c/sstl_flat_set.h>
#include <sstl/c/sstl_unordered_map.h>
#include <sstl/c/sstl_unordered_set.h>
#include <sstl/c/sstl_bitset.h>
#include <sstl/c/sstl_optional.h>
#include <sstl/c/sstl_variant.h>
#include <sstl/c/sstl_function.h>
#include <sstl/c/sstl_algorithm.h>

static int probe_zero(void* ctx) { return ctx ? *(int*)ctx : 0; }
static int probe_inc(void* ctx, int x) { return x + (ctx ? *(int*)ctx : 1); }
static int probe_add2(void* ctx, int a, int b) { return a + b + (ctx ? *(int*)ctx : 0); }
static int probe_add3(void* ctx, int a, int b, int c) { return a + b + c + (ctx ? *(int*)ctx : 0); }
static int probe_cmp_int_ptr(const void* a, const void* b) {
  const int* left = (const int*)a;
  const int* right = (const int*)b;
  return (*left > *right) - (*left < *right);
}

typedef struct probe_key {
  int major;
  int minor;
} probe_key;

typedef struct probe_alignas_storage {
  SSTL_ALIGNAS(16) char bytes[16];
} probe_alignas_storage;

SSTL_STATIC_ASSERT(sizeof(probe_alignas_storage) >= 16u, c99_static_assert_probe);

static int probe_key_cmp(const void* a, const void* b) {
  const probe_key* left = (const probe_key*)a;
  const probe_key* right = (const probe_key*)b;
  if (left->major != right->major) return (left->major > right->major) - (left->major < right->major);
  return (left->minor > right->minor) - (left->minor < right->minor);
}

static bool probe_key_eq(const void* a, const void* b) {
  const probe_key* left = (const probe_key*)a;
  const probe_key* right = (const probe_key*)b;
  return left->major == right->major && left->minor == right->minor;
}

static size_t probe_key_hash(const void* key) {
  return sstl_hash_bytes_v(key, sizeof(probe_key));
}

SSTL_ALGORITHM_DECLARE(probe_alg, int)
SSTL_ALGORITHM_DEFINE(probe_alg, int, probe_cmp_int_ptr)

SSTL_NUMERIC_ALGORITHM_DECLARE(probe_num, int)
SSTL_NUMERIC_ALGORITHM_DEFINE(probe_num, int)

SSTL_ARRAY_DECLARE(probe_arr, int, 2)
SSTL_ARRAY_DEFINE(probe_arr, int, 2)

SSTL_VECTOR_DECLARE(probe_vec, int, 2)
SSTL_VECTOR_DEFINE(probe_vec, int, 2)

SSTL_STRING_DECLARE(probe_str, 2)
SSTL_STRING_DEFINE(probe_str, 2)

SSTL_STRING_VIEW_DECLARE(probe_sv)
SSTL_STRING_VIEW_DEFINE(probe_sv)

SSTL_SPAN_DECLARE(probe_span, int)
SSTL_SPAN_DEFINE(probe_span, int)

SSTL_DEQUE_DECLARE(probe_deque, int, 2)
SSTL_DEQUE_DEFINE(probe_deque, int, 2)

SSTL_QUEUE_DECLARE(probe_queue, int, 2)
SSTL_QUEUE_DEFINE(probe_queue, int, 2)

SSTL_STACK_DECLARE(probe_stack, int, 2)
SSTL_STACK_DEFINE(probe_stack, int, 2)

SSTL_PRIORITY_QUEUE_DECLARE(probe_pqueue, int, 3, probe_cmp_int_ptr)
SSTL_PRIORITY_QUEUE_DEFINE(probe_pqueue, int, 3, probe_cmp_int_ptr)

SSTL_LIST_DECLARE(probe_list, int, 2)
SSTL_LIST_DEFINE(probe_list, int, 2)

SSTL_FORWARD_LIST_DECLARE(probe_flist, int, 2)
SSTL_FORWARD_LIST_DEFINE(probe_flist, int, 2)
extern int probe_flist_prev; /* Must remain an object declaration: forward_list intentionally has no prev helper. */

SSTL_MAP_DECLARE(probe_map, int, int, 2, sstl_cmp_int)
SSTL_MAP_DEFINE(probe_map, int, int, 2, sstl_cmp_int)

SSTL_SET_DECLARE(probe_set, int, 2, sstl_cmp_int)
SSTL_SET_DEFINE(probe_set, int, 2, sstl_cmp_int)

SSTL_FLAT_MAP_DECLARE(probe_flat_map, int, int, 2, sstl_cmp_int_v)
SSTL_FLAT_MAP_DEFINE(probe_flat_map, int, int, 2, sstl_cmp_int_v)

SSTL_FLAT_SET_DECLARE(probe_flat_set, int, 2, sstl_cmp_int_v)
SSTL_FLAT_SET_DEFINE(probe_flat_set, int, 2, sstl_cmp_int_v)

SSTL_UNORDERED_MAP_DECLARE(probe_umap, int, int, 2, 5, sstl_eq_int, sstl_hash_int)
SSTL_UNORDERED_MAP_DEFINE(probe_umap, int, int, 2, 5, sstl_eq_int, sstl_hash_int)

SSTL_UNORDERED_SET_DECLARE(probe_uset, int, 2, 5, sstl_eq_int, sstl_hash_int)
SSTL_UNORDERED_SET_DEFINE(probe_uset, int, 2, 5, sstl_eq_int, sstl_hash_int)

SSTL_MAP_DECLARE(probe_struct_map, probe_key, int, 2, probe_key_cmp)
SSTL_MAP_DEFINE(probe_struct_map, probe_key, int, 2, probe_key_cmp)

SSTL_SET_DECLARE(probe_struct_set, probe_key, 2, probe_key_cmp)
SSTL_SET_DEFINE(probe_struct_set, probe_key, 2, probe_key_cmp)

SSTL_FLAT_MAP_DECLARE(probe_struct_flat_map, probe_key, int, 2, probe_key_cmp)
SSTL_FLAT_MAP_DEFINE(probe_struct_flat_map, probe_key, int, 2, probe_key_cmp)

SSTL_FLAT_SET_DECLARE(probe_struct_flat_set, probe_key, 2, probe_key_cmp)
SSTL_FLAT_SET_DEFINE(probe_struct_flat_set, probe_key, 2, probe_key_cmp)

SSTL_UNORDERED_MAP_DECLARE(probe_struct_umap, probe_key, int, 2, 5, probe_key_eq, probe_key_hash)
SSTL_UNORDERED_MAP_DEFINE(probe_struct_umap, probe_key, int, 2, 5, probe_key_eq, probe_key_hash)

SSTL_UNORDERED_SET_DECLARE(probe_struct_uset, probe_key, 2, 5, probe_key_eq, probe_key_hash)
SSTL_UNORDERED_SET_DEFINE(probe_struct_uset, probe_key, 2, 5, probe_key_eq, probe_key_hash)

SSTL_BITSET_DECLARE(probe_bits, 9)
SSTL_BITSET_DEFINE(probe_bits, 9)

SSTL_OPTIONAL_DECLARE(probe_opt, int)
SSTL_OPTIONAL_DEFINE(probe_opt, int)

SSTL_VARIANT2_DECLARE(probe_var, int, char)
SSTL_VARIANT2_DEFINE(probe_var, int, char)

SSTL_VARIANT3_DECLARE(probe_var3, int, char, long)
SSTL_VARIANT3_DEFINE(probe_var3, int, char, long)

SSTL_VARIANT4_DECLARE(probe_var4, int, char, long, unsigned)
SSTL_VARIANT4_DEFINE(probe_var4, int, char, long, unsigned)

SSTL_FUNCTION0_DECLARE(probe_fn0, int)
SSTL_FUNCTION0_DEFINE(probe_fn0, int)

SSTL_FUNCTION1_DECLARE(probe_fn1, int, int)
SSTL_FUNCTION1_DEFINE(probe_fn1, int, int)

SSTL_FUNCTION2_DECLARE(probe_fn2, int, int, int)
SSTL_FUNCTION2_DEFINE(probe_fn2, int, int, int)

SSTL_FUNCTION3_DECLARE(probe_fn3, int, int, int, int)
SSTL_FUNCTION3_DEFINE(probe_fn3, int, int, int, int)

static void probe_void_hook_signature_checks(void) {
  typedef int (*array_cmp_sig)(const probe_arr*, const probe_arr*, sstl_cmp_fn);
  typedef void (*list_sort_sig)(probe_list*, sstl_cmp_fn);
  typedef bool (*list_merge_sig)(probe_list*, probe_list*, sstl_cmp_fn);
  typedef void (*flist_sort_sig)(probe_flist*, sstl_cmp_fn);
  typedef bool (*flist_merge_sig)(probe_flist*, probe_flist*, sstl_cmp_fn);
  typedef int* (*alg_pred_to_ptr_sig)(int*, int*, sstl_pred_fn);
  typedef bool (*alg_pred_to_bool_sig)(int*, int*, sstl_pred_fn);
  typedef size_t (*alg_pred_to_size_sig)(int*, int*, sstl_pred_fn);
  typedef int* (*alg_copy_if_sig)(int*, int*, int*, sstl_pred_fn);
  typedef int* (*alg_transform_sig)(int*, int*, int*, sstl_transform_fn);
  typedef void (*alg_replace_if_sig)(int*, int*, sstl_pred_fn, int);

  array_cmp_sig array_cmp = probe_arr_cmp;
  list_sort_sig list_sort = probe_list_sort;
  list_merge_sig list_merge = probe_list_merge;
  flist_sort_sig flist_sort = probe_flist_sort;
  flist_merge_sig flist_merge = probe_flist_merge;
  alg_pred_to_ptr_sig find_if_sig = probe_alg_find_if;
  alg_pred_to_ptr_sig find_if_not_sig = probe_alg_find_if_not;
  alg_pred_to_bool_sig all_of_sig = probe_alg_all_of;
  alg_pred_to_bool_sig any_of_sig = probe_alg_any_of;
  alg_pred_to_bool_sig none_of_sig = probe_alg_none_of;
  alg_pred_to_size_sig count_if_sig = probe_alg_count_if;
  alg_copy_if_sig copy_if_sig = probe_alg_copy_if;
  alg_transform_sig transform_sig = probe_alg_transform;
  alg_replace_if_sig replace_if_sig = probe_alg_replace_if;
  alg_pred_to_ptr_sig remove_if_sig = probe_alg_remove_if;
  alg_pred_to_ptr_sig partition_sig = probe_alg_partition;
  alg_pred_to_ptr_sig stable_partition_sig = probe_alg_stable_partition;

  (void)array_cmp;
  (void)list_sort;
  (void)list_merge;
  (void)flist_sort;
  (void)flist_merge;
  (void)find_if_sig;
  (void)find_if_not_sig;
  (void)all_of_sig;
  (void)any_of_sig;
  (void)none_of_sig;
  (void)count_if_sig;
  (void)copy_if_sig;
  (void)transform_sig;
  (void)replace_if_sig;
  (void)remove_if_sig;
  (void)partition_sig;
  (void)stable_partition_sig;
}

int main(void) {
  int sample[2] = {1, 2};
  int out = 0;
  int zero_bias = 0;
  int one_bias = 1;
  size_t hash = 0u;
  int alg_values[3] = {3, 1, 2};
  int alg_two = 2;
  int alg_out[3] = {0, 0, 0};

  probe_void_hook_signature_checks();

  if (SSTL_ALIGNOF(probe_alignas_storage) < 16u) return 80;

  probe_arr arr;
  probe_arr_fill(&arr, 4);
  if (probe_arr_size(&arr) != 2u || *probe_arr_at(&arr, 0u) != 4) return 1;
  if (*probe_arr_front(&arr) != 4 || *probe_arr_back(&arr) != 4) return 70;
  if (*probe_arr_try_front(&arr) != 4 || *probe_arr_try_back(&arr) != 4) return 200;
  {
    probe_arr arr_equal;
    probe_arr arr_greater;
    probe_arr_fill(&arr_equal, 4);
    probe_arr_fill(&arr_greater, 4);
    arr_greater.data[1] = 5;
    if (probe_arr_cmp(&arr, &arr_equal, probe_cmp_int_ptr) != 0) return 43;
    if (probe_arr_cmp(&arr, &arr_greater, probe_cmp_int_ptr) >= 0) return 44;
    if (probe_arr_cmp(&arr_greater, &arr, probe_cmp_int_ptr) <= 0) return 45;
    probe_arr_swap(&arr, &arr_greater);
    if (arr.data[1] != 5 || arr_greater.data[1] != 4) return 71;
  }

  probe_vec vec;
  probe_vec_init(&vec);
  if (!probe_vec_push_back(&vec, 1) || !probe_vec_try_pop_back(&vec, &out) || out != 1) return 2;
  if (probe_vec_capacity(&vec) != 2u || !probe_vec_try_push_back(&vec, 2) || !probe_vec_try_insert(&vec, 0u, 1)) return 62;
  if (*probe_vec_front(&vec) != 1 || *probe_vec_back(&vec) != 2 || probe_vec_data(&vec)[1] != 2) return 106;
  if (probe_vec_try_insert(&vec, 0u, 9) || probe_vec_try_resize(&vec, 3u, 0)) return 63;
  if (!probe_vec_assign(&vec, 2u, 6) || probe_vec_data(&vec)[0] != 6 || probe_vec_data(&vec)[1] != 6) return 150;
  if (probe_vec_try_assign(&vec, 3u, 0)) return 151;
  if (!probe_vec_try_assign(&vec, 1u, 7) || probe_vec_size(&vec) != 1u || *probe_vec_front(&vec) != 7) return 152;
  if (*probe_vec_try_front(&vec) != 7 || *probe_vec_try_back(&vec) != 7) return 201;
  {
    probe_vec other_vec;
    probe_vec_init(&other_vec);
    if (!probe_vec_try_push_back(&other_vec, 9)) return 107;
    probe_vec_swap(&vec, &other_vec);
    if (probe_vec_size(&vec) != 1u || *probe_vec_front(&vec) != 9 || probe_vec_size(&other_vec) != 1u) return 108;
  }
  probe_vec_clear(&vec);
  if (!probe_vec_empty(&vec) || !probe_vec_try_push_back(&vec, 4) || *probe_vec_at(&vec, 0u) != 4) return 102;

  probe_str str;
  probe_str str_other;
  probe_str_init(&str);
  if (!probe_str_push_back(&str, 'x') || probe_str_c_str(&str)[0] != 'x') return 3;
  if (!probe_str_try_insert_char(&str, 1u, 'y') || probe_str_try_push_back(&str, 'z')) return 64;
  if (!probe_str_try_pop_back(&str) || probe_str_empty(&str) || !probe_str_pop_back(&str) || !probe_str_empty(&str) || probe_str_try_pop_back(&str)) return 46;
  if (!probe_str_resize(&str, 2u, 'y') || probe_str_c_str(&str)[0] != 'y' || probe_str_c_str(&str)[1] != 'y') return 47;
  if (probe_str_resize(&str, 3u, 'z')) return 48;
  probe_str_init(&str_other);
  if (!probe_str_try_assign(&str_other, "q") || !probe_str_try_append_n(&str_other, "r", 1u)) return 49;
  if (probe_str_try_append_n(&str_other, "s", 1u) || probe_str_try_replace(&str_other, 0u, 0u, "too")) return 65;
  probe_str_swap(&str, &str_other);
  if (probe_str_c_str(&str)[0] != 'q' || probe_str_c_str(&str_other)[0] != 'y') return 50;
  if (probe_str_compare(&str, "qr") != 0 || probe_str_find(&str, "r") != 1u || probe_str_rfind(&str, "q") != 0u) return 72;
  if (probe_str_find(&str, "z") != SSTL_STRING_NPOS || probe_str_find_first_of(&str, "r") != 1u || probe_str_find_last_of(&str, "q") != 0u) return 73;
  if (probe_str_find_first_not_of(&str, "q") != 1u || probe_str_find_last_not_of(&str, "r") != 0u) return 74;
  {
    probe_str str_sub = probe_str_substr(&str, 1u, 1u);
    if (probe_str_size(&str_sub) != 1u || probe_str_c_str(&str_sub)[0] != 'r') return 75;
  }
  probe_str_clear(&str);
  if (!probe_str_empty(&str) || probe_str_c_str(&str)[0] != 0 || !probe_str_try_push_back(&str, 'a')) return 103;

  {
    probe_sv sv = probe_sv_make("abcdef", 6u);
    if (probe_sv_size(&sv) != 6u || *probe_sv_at(&sv, 1u) != 'b') return 28;
    if (probe_sv_find(&sv, "cd") != 2u || probe_sv_rfind(&sv, "ef") != 4u) return 29;
    if (probe_sv_find_first_of(&sv, "de") != 3u || probe_sv_find_last_of(&sv, "bd") != 3u) return 76;
    if (probe_sv_find_first_not_of(&sv, "ab") != 2u || probe_sv_find_last_not_of(&sv, "ef") != 3u) return 77;
    if (probe_sv_find_first_of(&sv, "xy") != SSTL_STRING_VIEW_NPOS || probe_sv_find_last_not_of(&sv, "abcdef") != SSTL_STRING_VIEW_NPOS) return 78;
    if (!probe_sv_starts_with(&sv, "ab") || !probe_sv_ends_with(&sv, "ef")) return 30;
    if (probe_sv_compare(&sv, "abcdef") != 0) return 31;
    probe_sv_remove_prefix(&sv, 1u);
    probe_sv_remove_suffix(&sv, 1u);
    if (probe_sv_size(&sv) != 4u || *probe_sv_data(&sv) != 'b') return 32;
  }

  probe_span span = probe_span_make(sample, 2u);
  if (probe_span_size(&span) != 2u || *probe_span_at(&span, 1u) != 2) return 4;
  if (probe_span_size_bytes(&span) != 2u * sizeof(sample[0]) || *probe_span_front(&span) != 1 || *probe_span_back(&span) != 2) return 79;
  if (*probe_span_try_front(&span) != 1 || *probe_span_try_back(&span) != 2) return 202;
  if (probe_span_begin(&span) != sample || probe_span_end(&span) != sample + 2) return 80;
  {
    probe_span first = probe_span_first(&span, 1u);
    probe_span last = probe_span_last(&span, 1u);
    probe_span middle = probe_span_subspan(&span, 1u, 1u);
    if (probe_span_size(&first) != 1u || *probe_span_data(&first) != 1) return 81;
    if (probe_span_size(&last) != 1u || *probe_span_data(&last) != 2) return 82;
    if (probe_span_size(&middle) != 1u || *probe_span_data(&middle) != 2) return 83;
  }

  probe_deque deque;
  probe_deque_init(&deque);
  if (probe_deque_capacity(&deque) != 2u || !probe_deque_try_push_front(&deque, 7) || !probe_deque_try_push_back(&deque, 8)) return 5;
  if (*probe_deque_try_front(&deque) != 7 || *probe_deque_try_back(&deque) != 8) return 203;
  if (probe_deque_try_push_back(&deque, 9) || !probe_deque_try_pop_back(&deque, &out) || out != 8) return 66;
  if (!probe_deque_try_pop_front(&deque, &out) || out != 7 || probe_deque_try_pop_front(&deque, &out)) return 204;
  if (!probe_deque_try_push_front(&deque, 7)) return 205;
  if (!probe_deque_try_insert(&deque, 1u, 9) || *probe_deque_at(&deque, 1u) != 9) return 84;
  if (!probe_deque_try_erase(&deque, 0u, &out) || out != 7 || *probe_deque_at(&deque, 0u) != 9) return 85;
  if (!probe_deque_try_resize(&deque, 2u, 5) || *probe_deque_at(&deque, 1u) != 5 || probe_deque_try_resize(&deque, 3u, 0)) return 86;
  if (*probe_deque_front(&deque) != 9 || *probe_deque_back(&deque) != 5) return 109;
  {
    probe_deque other_deque;
    probe_deque_init(&other_deque);
    if (!probe_deque_try_push_back(&other_deque, 1)) return 87;
    probe_deque_swap(&deque, &other_deque);
    if (probe_deque_size(&deque) != 1u || *probe_deque_at(&deque, 0u) != 1 || probe_deque_size(&other_deque) != 2u) return 88;
  }
  probe_deque_clear(&deque);
  if (!probe_deque_empty(&deque) || !probe_deque_try_push_front(&deque, 3) || *probe_deque_at(&deque, 0u) != 3) return 104;

  {
    probe_queue queue;
    probe_stack stack;
    probe_pqueue pqueue;
    probe_queue_init(&queue);
    probe_stack_init(&stack);
    probe_pqueue_init(&pqueue);
    if (probe_queue_capacity(&queue) != 2u || !probe_queue_try_push(&queue, 1) || !probe_queue_try_push(&queue, 2)) return 20;
    if (probe_queue_try_push(&queue, 3)) return 206;
    if (!probe_queue_full(&queue) || *probe_queue_front(&queue) != 1 || *probe_queue_front_const(&queue) != 1 ||
        *probe_queue_try_front(&queue) != 1 || *probe_queue_try_front_const(&queue) != 1 ||
        *probe_queue_back(&queue) != 2 || *probe_queue_back_const(&queue) != 2 ||
        *probe_queue_try_back(&queue) != 2 || *probe_queue_try_back_const(&queue) != 2 ||
        !probe_queue_try_pop(&queue, &out) || out != 1) return 110;
    {
      probe_queue other_queue;
      probe_queue_init(&other_queue);
      if (!probe_queue_push(&other_queue, 7)) return 111;
      probe_queue_swap(&queue, &other_queue);
      if (*probe_queue_front(&queue) != 7 || *probe_queue_front(&other_queue) != 2) return 112;
    }
    if (probe_stack_capacity(&stack) != 2u || !probe_stack_try_push(&stack, 1) || !probe_stack_try_push(&stack, 2)) return 21;
    if (probe_stack_try_push(&stack, 3)) return 207;
    if (!probe_stack_full(&stack) || *probe_stack_top(&stack) != 2 || *probe_stack_top_const(&stack) != 2 ||
        *probe_stack_try_top(&stack) != 2 || *probe_stack_try_top_const(&stack) != 2 ||
        !probe_stack_try_pop(&stack, &out) || out != 2) return 113;
    {
      probe_stack other_stack;
      probe_stack_init(&other_stack);
      if (!probe_stack_push(&other_stack, 7)) return 114;
      probe_stack_swap(&stack, &other_stack);
      if (*probe_stack_top(&stack) != 7 || *probe_stack_top(&other_stack) != 1) return 115;
    }
    if (!probe_pqueue_try_push(&pqueue, 1) || !probe_pqueue_try_push(&pqueue, 3) || !probe_pqueue_try_push(&pqueue, 2)) return 22;
    if (probe_pqueue_try_push(&pqueue, 4)) return 208;
    if (probe_pqueue_capacity(&pqueue) != 3u || !probe_pqueue_full(&pqueue) ||
        *probe_pqueue_top(&pqueue) != 3 || *probe_pqueue_top_const(&pqueue) != 3 ||
        *probe_pqueue_try_top(&pqueue) != 3 || *probe_pqueue_try_top_const(&pqueue) != 3 ||
        !probe_pqueue_try_pop(&pqueue, &out) || out != 3) return 23;
    {
      probe_pqueue other_pqueue;
      probe_pqueue_init(&other_pqueue);
      if (!probe_pqueue_push(&other_pqueue, 9)) return 116;
      probe_pqueue_swap(&pqueue, &other_pqueue);
      if (*probe_pqueue_top(&pqueue) != 9 || probe_pqueue_size(&other_pqueue) != 2u) return 117;
    }
  }

  probe_list list;
  probe_list_init(&list);
  if (probe_list_capacity(&list) != 2u || !probe_list_try_push_back(&list, 8) || *probe_list_front(&list) != 8) return 6;
  if (*probe_list_try_front(&list) != 8 || *probe_list_try_back(&list) != 8) return 209;
  if (!probe_list_try_push_front(&list, 7) || probe_list_try_insert(&list, 0u, 6)) return 67;
  if (*probe_list_try_front(&list) != 7 || *probe_list_try_back(&list) != 8) return 210;
  if (!probe_list_try_pop_back(&list, &out) || out != 8 || !probe_list_try_pop_front(&list, &out) || out != 7 || !probe_list_empty(&list)) return 51;
  {
    probe_list other_list;
    probe_list_init(&other_list);
    if (!probe_list_try_push_back(&list, 1) || !probe_list_try_push_back(&other_list, 2)) return 89;
    if (!probe_list_try_splice(&list, probe_list_end(&list), &other_list) || !probe_list_empty(&other_list) || probe_list_size(&list) != 2u) return 90;
    if (*probe_list_at(&list, probe_list_next(&list, probe_list_begin(&list))) != 2) return 91;
    if (*probe_list_deref(&list, probe_list_prev(&list, probe_list_end(&list))) != 2) return 124;
  }
  probe_list_clear(&list);
  {
    probe_list one_src;
    probe_list_init(&one_src);
    if (!probe_list_try_push_back(&one_src, 3)) return 153;
    if (!probe_list_try_splice_one(&list, probe_list_end(&list), &one_src, probe_list_begin(&one_src))) return 154;
    if (!probe_list_empty(&one_src) || probe_list_size(&list) != 1u || *probe_list_front(&list) != 3) return 155;
  }
  probe_list_clear(&list);
  {
    probe_list range_src;
    probe_list_init(&range_src);
    if (!probe_list_try_push_back(&range_src, 4) || !probe_list_try_push_back(&range_src, 5)) return 156;
    if (!probe_list_try_splice_range(&list, probe_list_end(&list), &range_src, probe_list_begin(&range_src), probe_list_end(&range_src))) return 157;
    if (!probe_list_empty(&range_src) || probe_list_size(&list) != 2u || *probe_list_front(&list) != 4 || *probe_list_back(&list) != 5) return 158;
  }
  probe_list_clear(&list);
  {
    probe_list merge_src;
    probe_list_init(&merge_src);
    if (!probe_list_try_push_back(&list, 1) || !probe_list_try_push_back(&merge_src, 2)) return 211;
    if (!probe_list_try_merge(&list, &merge_src, probe_cmp_int_ptr) || !probe_list_empty(&merge_src) || probe_list_size(&list) != 2u) return 212;
    if (*probe_list_front(&list) != 1 || *probe_list_back(&list) != 2) return 213;
  }
  probe_list_clear(&list);
  if (!probe_list_empty(&list) || !probe_list_try_push_back(&list, 5) || *probe_list_front(&list) != 5) return 105;

  probe_flist flist;
  probe_flist_init(&flist);
  if (probe_flist_capacity(&flist) != 2u || !probe_flist_try_push_front(&flist, 9)) return 7;
  if (probe_flist_begin(&flist) == probe_flist_end(&flist) || *probe_flist_at(&flist, probe_flist_begin(&flist)) != 9) return 33;
  if (*probe_flist_deref(&flist, probe_flist_begin(&flist)) != 9) return 125;
  if (*probe_flist_try_front(&flist) != 9) return 214;
  if (probe_flist_try_insert_after(&flist, probe_flist_before_begin(&flist), 8) == probe_flist_end(&flist)) return 68;
  if (probe_flist_try_insert_after(&flist, probe_flist_before_begin(&flist), 7) != probe_flist_end(&flist)) return 69;
  if (!probe_flist_try_pop_front(&flist, &out) || out != 8) return 34;
  {
    probe_flist flist_dst;
    probe_flist flist_src;
    probe_flist_init(&flist_dst);
    probe_flist_init(&flist_src);
    if (!probe_flist_try_push_front(&flist_dst, 1) || !probe_flist_try_push_front(&flist_src, 2)) return 215;
    if (!probe_flist_try_splice_after(&flist_dst, probe_flist_before_begin(&flist_dst), &flist_src) ||
        !probe_flist_empty(&flist_src) || probe_flist_size(&flist_dst) != 2u) return 216;
  }
  {
    probe_flist flist_dst;
    probe_flist flist_src;
    probe_flist_init(&flist_dst);
    probe_flist_init(&flist_src);
    if (!probe_flist_try_push_front(&flist_src, 4)) return 217;
    if (!probe_flist_try_splice_after_one(&flist_dst, probe_flist_before_begin(&flist_dst), &flist_src, probe_flist_before_begin(&flist_src)) ||
        !probe_flist_empty(&flist_src) || probe_flist_size(&flist_dst) != 1u || *probe_flist_try_front(&flist_dst) != 4) return 218;
  }
  {
    probe_flist flist_dst;
    probe_flist flist_src;
    probe_flist_init(&flist_dst);
    probe_flist_init(&flist_src);
    if (!probe_flist_try_push_front(&flist_dst, 1) || !probe_flist_try_push_front(&flist_src, 2)) return 219;
    if (!probe_flist_try_merge(&flist_dst, &flist_src, probe_cmp_int_ptr) ||
        !probe_flist_empty(&flist_src) || probe_flist_size(&flist_dst) != 2u) return 220;
  }

  probe_map map;
  probe_map_init(&map);
  if (probe_map_capacity(&map) != 2u || !probe_map_insert(&map, 1, 10) || !probe_map_insert(&map, 2, 20) || *probe_map_find(&map, 1) != 10) return 8;
  if (probe_map_deref(&map, probe_map_begin(&map))->key != 1) return 126;
  if (probe_map_deref(&map, probe_map_prev(&map, probe_map_end(&map)))->key != 2) return 127;
  if (probe_map_prev(&map, probe_map_begin(&map)) != probe_map_end(&map)) return 128;
  if (probe_map_prev(&map, (probe_map_iterator)-2) != probe_map_end(&map)) return 129;

  probe_set set;
  probe_set_init(&set);
  if (probe_set_capacity(&set) != 2u || !probe_set_insert(&set, 3) || !probe_set_insert(&set, 4) || *probe_set_find(&set, 3) != 3) return 9;
  if (*probe_set_deref(&set, probe_set_begin(&set)) != 3) return 130;
  if (*probe_set_deref(&set, probe_set_prev(&set, probe_set_end(&set))) != 4) return 131;
  if (probe_set_prev(&set, probe_set_begin(&set)) != probe_set_end(&set)) return 132;
  if (probe_set_erase(&set, 3) != 1u || probe_set_find(&set, 3) != 0) return 52;

  probe_flat_map fmap;
  probe_flat_map_init(&fmap);
  if (probe_flat_map_capacity(&fmap) != 2u || !probe_flat_map_insert(&fmap, 2, 20) || !probe_flat_map_insert(&fmap, 1, 10)) return 10;
  if (*probe_flat_map_find(&fmap, 2) != 20 || probe_flat_map_at_index(&fmap, 0u)->key != 1) return 35;
  if (probe_flat_map_count(&fmap, 2) != 1u || probe_flat_map_equal_range_first(&fmap, 2) != 1u ||
      probe_flat_map_equal_range_second(&fmap, 2) != 2u) return 118;
  probe_flat_map_clear(&fmap);
  if (!probe_flat_map_empty(&fmap)) return 119;

  probe_flat_set fset;
  probe_flat_set_init(&fset);
  if (probe_flat_set_capacity(&fset) != 2u || !probe_flat_set_insert(&fset, 4) || !probe_flat_set_insert(&fset, 1) || *probe_flat_set_find(&fset, 4) != 4) return 11;
  if (fset.data[0] != 1 || fset.data[1] != 4) return 36;
  if (probe_flat_set_count(&fset, 4) != 1u || probe_flat_set_equal_range_first(&fset, 4) != 1u ||
      probe_flat_set_equal_range_second(&fset, 4) != 2u) return 120;
  probe_flat_set_clear(&fset);
  if (!probe_flat_set_empty(&fset)) return 121;

  probe_umap umap;
  probe_umap_init(&umap);
  if (probe_umap_capacity(&umap) != 2u || probe_umap_bucket_count(&umap) != 5u || !probe_umap_insert(&umap, 5, 50)) return 12;
  if (*probe_umap_find(&umap, 5) != 50 || probe_umap_find_slot(&umap, 5) == (size_t)-1) return 37;
  if (probe_umap_count(&umap, 5) != 1u || probe_umap_count(&umap, 4) != 0u) return 122;
  if (probe_umap_load_factor(&umap) <= 0.0f || probe_umap_max_load_factor(&umap) < probe_umap_load_factor(&umap)) return 159;
  if (probe_umap_begin(&umap) == probe_umap_end(&umap) || probe_umap_deref(&umap, probe_umap_begin(&umap))->key != 5) return 160;
  if (probe_umap_equal_range_first(&umap, 5) == probe_umap_end(&umap) ||
      probe_umap_equal_range_second(&umap, 5) != probe_umap_next(&umap, probe_umap_equal_range_first(&umap, 5))) return 161;

  probe_uset uset;
  probe_uset_init(&uset);
  if (probe_uset_capacity(&uset) != 2u || probe_uset_bucket_count(&uset) != 5u || !probe_uset_insert(&uset, 6)) return 13;
  if (*probe_uset_find(&uset, 6) != 6 || probe_uset_find_slot(&uset, 6) == (size_t)-1) return 38;
  if (probe_uset_count(&uset, 6) != 1u || probe_uset_count(&uset, 5) != 0u) return 123;
  if (probe_uset_load_factor(&uset) <= 0.0f || probe_uset_max_load_factor(&uset) < probe_uset_load_factor(&uset)) return 162;
  if (probe_uset_begin(&uset) == probe_uset_end(&uset) || *probe_uset_deref(&uset, probe_uset_begin(&uset)) != 6) return 163;
  if (probe_uset_equal_range_first(&uset, 6) == probe_uset_end(&uset) ||
      probe_uset_equal_range_second(&uset, 6) != probe_uset_next(&uset, probe_uset_equal_range_first(&uset, 6))) return 164;

  {
    probe_key low = {1, 1};
    probe_key high = {1, 2};
    probe_struct_map struct_map;
    probe_struct_set struct_set;
    probe_struct_flat_map struct_flat_map;
    probe_struct_flat_set struct_flat_set;
    probe_struct_umap struct_umap;
    probe_struct_uset struct_uset;
    probe_struct_map_init(&struct_map);
    probe_struct_set_init(&struct_set);
    probe_struct_flat_map_init(&struct_flat_map);
    probe_struct_flat_set_init(&struct_flat_set);
    probe_struct_umap_init(&struct_umap);
    probe_struct_uset_init(&struct_uset);
    if (!probe_struct_map_insert(&struct_map, high, 12) || !probe_struct_map_insert(&struct_map, low, 11)) return 53;
    if (*probe_struct_map_find(&struct_map, low) != 11 || probe_struct_map_begin(&struct_map) == probe_struct_map_end(&struct_map)) return 54;
    if (!probe_struct_set_insert(&struct_set, high) || !probe_struct_set_insert(&struct_set, low) || probe_struct_set_find(&struct_set, low) == 0) return 55;
    if (!probe_struct_flat_map_insert(&struct_flat_map, high, 22) || !probe_struct_flat_map_insert(&struct_flat_map, low, 21)) return 56;
    if (probe_struct_flat_map_lower_bound(&struct_flat_map, high) != 1u || *probe_struct_flat_map_find(&struct_flat_map, low) != 21) return 57;
    if (!probe_struct_flat_set_insert(&struct_flat_set, high) || !probe_struct_flat_set_insert(&struct_flat_set, low)) return 58;
    if (probe_struct_flat_set_lower_bound(&struct_flat_set, high) != 1u || probe_struct_flat_set_find(&struct_flat_set, low) == 0) return 59;
    if (!probe_struct_umap_insert(&struct_umap, high, 32) || *probe_struct_umap_find(&struct_umap, high) != 32) return 60;
    if (!probe_struct_uset_insert(&struct_uset, low) || probe_struct_uset_find(&struct_uset, low) == 0) return 61;
  }

  probe_bits bits;
  probe_bits_reset(&bits);
  probe_bits_set(&bits, 3u);
  if (!probe_bits_test(&bits, 3u) || probe_bits_count(&bits) != 1u) return 14;
  if (sizeof(bits.words[0]) != sizeof(SSTL_C_BITSET_WORD_TYPE) || sizeof(bits.words[0]) != sizeof(unsigned long)) return 39;
  if (!probe_bits_any(&bits) || probe_bits_none(&bits) || probe_bits_all(&bits) || probe_bits_to_ulong(&bits) != 8ul) return 92;
  {
    char bit_text[10];
    probe_bits other_bits;
    probe_bits inverted_bits;
    if (!probe_bits_to_string(&bits, bit_text, sizeof(bit_text), '0', '1') || bit_text[5] != '1') return 93;
    inverted_bits = probe_bits_not(&bits);
    if (probe_bits_test(&inverted_bits, 3u) || !probe_bits_test(&inverted_bits, 0u)) return 94;
    probe_bits_reset(&other_bits);
    probe_bits_set(&other_bits, 4u);
    probe_bits_or_assign(&bits, &other_bits);
    if (!probe_bits_test(&bits, 4u)) return 95;
    probe_bits_and_assign(&bits, &other_bits);
    if (probe_bits_test(&bits, 3u) || !probe_bits_test(&bits, 4u)) return 96;
    probe_bits_xor_assign(&bits, &other_bits);
    if (probe_bits_any(&bits)) return 97;
    probe_bits_set(&bits, 1u);
    probe_bits_shift_left_assign(&bits, 2u);
    if (!probe_bits_test(&bits, 3u)) return 98;
    probe_bits_shift_right_assign(&bits, 1u);
    if (!probe_bits_test(&bits, 2u)) return 99;
  }

  probe_opt opt;
  probe_opt_init(&opt);
  probe_opt_set(&opt, 33);
  if (!probe_opt_has_value(&opt) || *probe_opt_try_value(&opt) != 33) return 15;
  if (*probe_opt_value(&opt) != 33 || probe_opt_value_or(&opt, 7) != 33) return 100;
  probe_opt_reset(&opt);
  if (probe_opt_try_value(&opt) != 0 || probe_opt_value_or(&opt, 7) != 7) return 101;

  probe_var var;
  probe_var_set0(&var, 44);
  if (probe_var_index(&var) != 0u || *probe_var_get0(&var) != 44) return 16;
  {
    probe_var3 var3;
    probe_var4 var4;
    probe_var3_set2(&var3, 55l);
    probe_var4_set3(&var4, 66u);
    if (probe_var3_index(&var3) != 2u || *probe_var3_get2(&var3) != 55l) return 24;
    if (probe_var4_index(&var4) != 3u || *probe_var4_get3(&var4) != 66u) return 25;
  }

  {
    probe_fn0 fn0 = probe_fn0_make(probe_zero, &zero_bias);
    probe_fn1 fn1 = probe_fn1_make(probe_inc, &one_bias);
    probe_fn2 fn2 = probe_fn2_make(probe_add2, &zero_bias);
    probe_fn3 fn3 = probe_fn3_make(probe_add3, &zero_bias);
    if (!probe_fn0_valid(&fn0) || probe_fn0_call(&fn0) != 0) return 17;
    if (!probe_fn1_valid(&fn1) || probe_fn1_call(&fn1, 4) != 5) return 18;
    if (!probe_fn2_valid(&fn2) || probe_fn2_call(&fn2, 3, 4) != 7) return 26;
    if (!probe_fn3_valid(&fn3) || probe_fn3_call(&fn3, 1, 2, 3) != 6) return 27;
  }

  hash = sstl_hash_bytes_v(sample, sizeof(sample));
  {
    int one = 1;
    int two = 2;
    unsigned one_u = 1u;
    unsigned two_u = 2u;
    const char* a_text = "a";
    const char* b_text = "b";
    const char* same_text = "same";
    sstl_cmp_fn cmp_sig = sstl_cmp_int;
    sstl_eq_fn eq_sig = sstl_eq_int;
    sstl_hash_fn hash_sig = sstl_hash_int;
    sstl_hash_fn bytes_hash_sig = sstl_hash_bytes;
    const size_t canonical_bytes_hash = bytes_hash_sig(sample);
    if (cmp_sig(&one, &two) >= 0 || sstl_cmp_int_v(&one, &two) >= 0 || sstl_cmp_uint(&two_u, &one_u) <= 0 ||
        sstl_cmp_uint_v(&two_u, &one_u) <= 0 || sstl_cmp_str(&a_text, &b_text) >= 0 || sstl_cmp_str_v(&a_text, &b_text) >= 0 ||
        !eq_sig(&one, &one) || !sstl_eq_int_v(&one, &one) || sstl_eq_uint(&one_u, &two_u) || sstl_eq_uint_v(&one_u, &two_u) ||
        !sstl_eq_str(&same_text, &same_text) || !sstl_eq_str_v(&same_text, &same_text) || hash == 0u ||
        hash_sig(&one) == 0u || canonical_bytes_hash != sstl_hash_bytes(sample)) return 19;
  }
  probe_alg_sort(alg_values, alg_values + 3);
  if (alg_values[0] != 1 || alg_values[2] != 3 || !probe_alg_binary_search(alg_values, alg_values + 3, &alg_two)) return 40;
  if (probe_num_accumulate(alg_values, alg_values + 3, 0) != 6) return 41;
  probe_num_iota(alg_out, alg_out + 3, 4);
  if (alg_out[0] != 4 || alg_out[2] != 6) return 42;

  return 0;
}
