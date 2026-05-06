/**
 * @file sstl_priority_queue.h
 * @brief C99 fixed-capacity priority queue backed by an inline binary heap.
 *
 * The generated API keeps the highest-priority element at index zero. The
 * comparator passed to the macro is a C function-like name with signature
 * `sstl_cmp_fn`; positive means the first value has higher
 * priority than the second value.
 */
#ifndef SSTL_C_PRIORITY_QUEUE_H
/** @def SSTL_C_PRIORITY_QUEUE_H
 * @brief Include guard for sstl_priority_queue.h.
 */
#define SSTL_C_PRIORITY_QUEUE_H

#include "sstl_config.h"

/** @def SSTL_PRIORITY_QUEUE_DECLARE
 * @brief Declare a typed fixed-capacity C priority queue API.
 */
#define SSTL_PRIORITY_QUEUE_DECLARE(NAME, T, CAP, CMP) \
  typedef struct NAME { T data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Inline binary-heap storage. */ size_t size; /**< @brief Number of live heap elements. */ } NAME; \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* q); \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* q); \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* q); \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* q); \
  /** \
   * @brief Run the generated full operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* q); \
  /** \
   * @brief Run the generated push operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_push(NAME* q, T x); \
  /** \
   * @brief Try to push without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_push(NAME* q, T x); \
  /** \
   * @brief Run the generated pop operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_pop(NAME* q, T* out); \
  /** \
   * @brief Try to pop without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_pop(NAME* q, T* out); \
  /** \
   * @brief Run the generated top operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_top(NAME* q); \
  /** \
   * @brief Try to access the top element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the top element, or null when empty. \
   */ \
  SSTL_C_INLINE T* NAME##_try_top(NAME* q); \
  /** \
   * @brief Run the generated top const operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE const T* NAME##_top_const(const NAME* q); \
  /** \
   * @brief Try to access the const top element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the top element, or null when empty. \
   */ \
  SSTL_C_INLINE const T* NAME##_try_top_const(const NAME* q); \
  /** \
   * @brief Exchange two priority queues of the same generated type. \
   * @param a First priority queue. \
   * @param b Second priority queue. \
   */ \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b);

/** @def SSTL_PRIORITY_QUEUE_DEFINE
 * @brief Define a typed fixed-capacity C priority queue API.
 */
#define SSTL_PRIORITY_QUEUE_DEFINE(NAME, T, CAP, CMP) \
  /** \
   * @brief Run the generated higher priority operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_higher_priority(const T* a, const T* b) { return (CMP)((const void*)a, (const void*)b) > 0; } \
  /** \
   * @brief Run the generated sift up operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @param child Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_sift_up(NAME* q, size_t child) { while (child != 0u) { size_t parent = (child - 1u) / 2u; if (!NAME##_higher_priority(&q->data[child], &q->data[parent])) break; { T tmp = q->data[child]; q->data[child] = q->data[parent]; q->data[parent] = tmp; } child = parent; } } \
  /** \
   * @brief Run the generated sift down operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @param parent Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_sift_down(NAME* q, size_t parent) { for (;;) { size_t left = parent * 2u + 1u; size_t right = left + 1u; size_t best = parent; if (left < q->size && NAME##_higher_priority(&q->data[left], &q->data[best])) best = left; if (right < q->size && NAME##_higher_priority(&q->data[right], &q->data[best])) best = right; if (best == parent) break; { T tmp = q->data[parent]; q->data[parent] = q->data[best]; q->data[best] = tmp; } parent = best; } } \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* q) { q->size = 0u; } \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* q) { return q->size; } \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* q) { (void)q; return (size_t)(CAP); } \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* q) { return q->size == 0u; } \
  /** \
   * @brief Run the generated full operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* q) { return q->size == (size_t)(CAP); } \
  /** \
   * @brief Run the generated push operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_push(NAME* q, T x) { if (q->size == (size_t)(CAP)) { SSTL_C_PANIC("priority_queue full"); return false; } q->data[q->size] = x; ++q->size; NAME##_sift_up(q, q->size - 1u); return true; } \
  /** \
   * @brief Try to push without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_push(NAME* q, T x) { if (q->size == (size_t)(CAP)) return false; q->data[q->size] = x; ++q->size; NAME##_sift_up(q, q->size - 1u); return true; } \
  /** \
   * @brief Run the generated pop operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_pop(NAME* q, T* out) { if (q->size == 0u) { SSTL_C_PANIC("priority_queue empty"); return false; } if (out) *out = q->data[0]; --q->size; if (q->size != 0u) { q->data[0] = q->data[q->size]; NAME##_sift_down(q, 0u); } return true; } \
  /** \
   * @brief Try to pop without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_pop(NAME* q, T* out) { if (q->size == 0u) return false; if (out) *out = q->data[0]; --q->size; if (q->size != 0u) { q->data[0] = q->data[q->size]; NAME##_sift_down(q, 0u); } return true; } \
  /** \
   * @brief Run the generated top operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_top(NAME* q) { if (q->size == 0u) { SSTL_C_PANIC("priority_queue top"); return 0; } return &q->data[0]; } \
  /** \
   * @brief Try to access the top element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the top element, or null when empty. \
   */ \
  SSTL_C_INLINE T* NAME##_try_top(NAME* q) { return q->size == 0u ? 0 : &q->data[0]; } \
  /** \
   * @brief Run the generated top const operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE const T* NAME##_top_const(const NAME* q) { if (q->size == 0u) { SSTL_C_PANIC("priority_queue top"); return 0; } return &q->data[0]; } \
  /** \
   * @brief Try to access the const top element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the top element, or null when empty. \
   */ \
  SSTL_C_INLINE const T* NAME##_try_top_const(const NAME* q) { return q->size == 0u ? 0 : &q->data[0]; } \
  /** \
   * @brief Exchange two priority queues of the same generated type. \
   * @param a First priority queue. \
   * @param b Second priority queue. \
   */ \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b) { NAME tmp = *a; *a = *b; *b = tmp; }

#endif
