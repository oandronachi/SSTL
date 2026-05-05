/* test_id: freestanding.c99.public_probe */
/* api_ids: [c.freestanding.compile] */
/* req_ids: [REQ-01, REQ-02, REQ-07] */
/* patterns: [FREESTANDING-COMPILE] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This is primarily a compile/link contract test. Passing means the public headers are self-contained in the intended dialect/profile.
 *   - Runtime behavior is intentionally minimal; the important failure mode is a diagnostic from the compiler or linker.
 *   - Add new public headers here when the implementation grows so dialect drift is caught early.
 */
#include <sstl/c/sstl_vector.h>
#include <sstl/c/sstl_queue.h>
#include <sstl/c/sstl_stack.h>
#include <sstl/c/sstl_priority_queue.h>

static int free_cmp_int_ptr(const void* a, const void* b) {
  const int* left = (const int*)a;
  const int* right = (const int*)b;
  return (*left > *right) - (*left < *right);
}

SSTL_VECTOR_DECLARE(free_vec, int, 2)
SSTL_VECTOR_DEFINE(free_vec, int, 2)

SSTL_QUEUE_DECLARE(free_queue, int, 2)
SSTL_QUEUE_DEFINE(free_queue, int, 2)

SSTL_STACK_DECLARE(free_stack, int, 2)
SSTL_STACK_DEFINE(free_stack, int, 2)

SSTL_PRIORITY_QUEUE_DECLARE(free_pqueue, int, 2, free_cmp_int_ptr)
SSTL_PRIORITY_QUEUE_DEFINE(free_pqueue, int, 2, free_cmp_int_ptr)

void _start(void) {
  free_vec v;
  free_queue q;
  free_stack s;
  free_pqueue p;
  free_vec_init(&v);
  free_queue_init(&q);
  free_stack_init(&s);
  free_pqueue_init(&p);
  (void)free_vec_push_back(&v, 1);
  (void)free_queue_push(&q, 1);
  (void)free_stack_push(&s, 1);
  (void)free_pqueue_push(&p, 1);
  for (;;) {}
}
