/* test_id: c99.adapters.queue_stack_priority_queue */
/* api_ids: [c.queue.contract, c.stack.contract, c.priority_queue.contract] */
/* req_ids: [REQ-03, REQ-05, REQ-06, REQ-07] */
/* patterns: [AUDIT-NOALLOC, CAPACITY-BOUNDARY, C-API-PARITY] */
/*
 * Notes:
 *   - This test exercises the C adapter families as real generated APIs, not just as included headers.
 *   - Queue, stack, and priority_queue deliberately share the same element type so ordering differences are easy to see.
 *   - Capacity failures must return false and leave existing contents observable.
 *   - Zero-capacity instantiations prove the macros do not rely on non-standard zero-length arrays.
 */
#include <sstl/c/sstl_queue.h>
#include <sstl/c/sstl_stack.h>
#include <sstl/c/sstl_priority_queue.h>
#include "test_harness.h"
#include "noalloc_audit.h"

static int adapter_cmp_int_ptr(const void* a, const void* b) {
  const int* left = (const int*)a;
  const int* right = (const int*)b;
  return (*left > *right) - (*left < *right);
}

SSTL_QUEUE_DECLARE(adapter_queue, int, 3)
SSTL_QUEUE_DEFINE(adapter_queue, int, 3)

SSTL_STACK_DECLARE(adapter_stack, int, 3)
SSTL_STACK_DEFINE(adapter_stack, int, 3)

SSTL_PRIORITY_QUEUE_DECLARE(adapter_pqueue, int, 4, adapter_cmp_int_ptr)
SSTL_PRIORITY_QUEUE_DEFINE(adapter_pqueue, int, 4, adapter_cmp_int_ptr)

SSTL_QUEUE_DECLARE(adapter_queue0, int, 0)
SSTL_QUEUE_DEFINE(adapter_queue0, int, 0)

SSTL_STACK_DECLARE(adapter_stack0, int, 0)
SSTL_STACK_DEFINE(adapter_stack0, int, 0)

SSTL_PRIORITY_QUEUE_DECLARE(adapter_pqueue0, int, 0, adapter_cmp_int_ptr)
SSTL_PRIORITY_QUEUE_DEFINE(adapter_pqueue0, int, 0, adapter_cmp_int_ptr)

static void queue_preserves_fifo_order_and_capacity(void) {
  adapter_queue q;
  int out = 0;
  sstl_c_noalloc_begin();
  adapter_queue_init(&q);
  SSTL_C_EQ(adapter_queue_capacity(&q), 3u);
  SSTL_C_ASSERT(adapter_queue_empty(&q));
  SSTL_C_ASSERT(adapter_queue_push(&q, 10));
  SSTL_C_ASSERT(adapter_queue_push(&q, 20));
  SSTL_C_ASSERT(adapter_queue_push(&q, 30));
  SSTL_C_ASSERT(adapter_queue_full(&q));
  SSTL_C_ASSERT(!adapter_queue_push(&q, 40));
  SSTL_C_EQ(*adapter_queue_front(&q), 10);
  SSTL_C_ASSERT(adapter_queue_pop(&q, &out));
  SSTL_C_EQ(out, 10);
  SSTL_C_ASSERT(adapter_queue_push(&q, 40));
  SSTL_C_ASSERT(adapter_queue_pop(&q, &out));
  SSTL_C_EQ(out, 20);
  SSTL_C_ASSERT(adapter_queue_pop(&q, &out));
  SSTL_C_EQ(out, 30);
  SSTL_C_ASSERT(adapter_queue_pop(&q, &out));
  SSTL_C_EQ(out, 40);
  SSTL_C_ASSERT(!adapter_queue_pop(&q, &out));
  sstl_c_noalloc_end();
}

static void stack_preserves_lifo_order_and_capacity(void) {
  adapter_stack s;
  int out = 0;
  sstl_c_noalloc_begin();
  adapter_stack_init(&s);
  SSTL_C_EQ(adapter_stack_capacity(&s), 3u);
  SSTL_C_ASSERT(adapter_stack_push(&s, 10));
  SSTL_C_ASSERT(adapter_stack_push(&s, 20));
  SSTL_C_ASSERT(adapter_stack_push(&s, 30));
  SSTL_C_ASSERT(adapter_stack_full(&s));
  SSTL_C_ASSERT(!adapter_stack_push(&s, 40));
  SSTL_C_EQ(*adapter_stack_top(&s), 30);
  SSTL_C_ASSERT(adapter_stack_pop(&s, &out));
  SSTL_C_EQ(out, 30);
  SSTL_C_ASSERT(adapter_stack_pop(&s, &out));
  SSTL_C_EQ(out, 20);
  SSTL_C_ASSERT(adapter_stack_pop(&s, &out));
  SSTL_C_EQ(out, 10);
  SSTL_C_ASSERT(!adapter_stack_pop(&s, &out));
  sstl_c_noalloc_end();
}

static void priority_queue_pops_highest_priority_first(void) {
  adapter_pqueue q;
  int out = 0;
  sstl_c_noalloc_begin();
  adapter_pqueue_init(&q);
  SSTL_C_EQ(adapter_pqueue_capacity(&q), 4u);
  SSTL_C_ASSERT(adapter_pqueue_push(&q, 4));
  SSTL_C_ASSERT(adapter_pqueue_push(&q, 1));
  SSTL_C_ASSERT(adapter_pqueue_push(&q, 7));
  SSTL_C_ASSERT(adapter_pqueue_push(&q, 3));
  SSTL_C_ASSERT(adapter_pqueue_full(&q));
  SSTL_C_ASSERT(!adapter_pqueue_push(&q, 9));
  SSTL_C_EQ(*adapter_pqueue_top(&q), 7);
  SSTL_C_ASSERT(adapter_pqueue_pop(&q, &out));
  SSTL_C_EQ(out, 7);
  SSTL_C_ASSERT(adapter_pqueue_pop(&q, &out));
  SSTL_C_EQ(out, 4);
  SSTL_C_ASSERT(adapter_pqueue_pop(&q, &out));
  SSTL_C_EQ(out, 3);
  SSTL_C_ASSERT(adapter_pqueue_pop(&q, &out));
  SSTL_C_EQ(out, 1);
  SSTL_C_ASSERT(!adapter_pqueue_pop(&q, &out));
  sstl_c_noalloc_end();
}

static void zero_capacity_adapters_reject_pushes_without_zero_length_arrays(void) {
  adapter_queue0 q;
  adapter_stack0 s;
  adapter_pqueue0 p;
  sstl_c_noalloc_begin();
  adapter_queue0_init(&q);
  adapter_stack0_init(&s);
  adapter_pqueue0_init(&p);
  SSTL_C_EQ(adapter_queue0_capacity(&q), 0u);
  SSTL_C_EQ(adapter_stack0_capacity(&s), 0u);
  SSTL_C_EQ(adapter_pqueue0_capacity(&p), 0u);
  SSTL_C_ASSERT(adapter_queue0_empty(&q));
  SSTL_C_ASSERT(adapter_stack0_empty(&s));
  SSTL_C_ASSERT(adapter_pqueue0_empty(&p));
  SSTL_C_ASSERT(!adapter_queue0_push(&q, 1));
  SSTL_C_ASSERT(!adapter_stack0_push(&s, 1));
  SSTL_C_ASSERT(!adapter_pqueue0_push(&p, 1));
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"queue_preserves_fifo_order_and_capacity", queue_preserves_fifo_order_and_capacity},
    {"stack_preserves_lifo_order_and_capacity", stack_preserves_lifo_order_and_capacity},
    {"priority_queue_pops_highest_priority_first", priority_queue_pops_highest_priority_first},
    {"zero_capacity_adapters_reject_pushes_without_zero_length_arrays", zero_capacity_adapters_reject_pushes_without_zero_length_arrays}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
