/**
 * @file sstl_queue.h
 * @brief C99 fixed-capacity FIFO adapter backed by inline circular storage.
 *
 * The generated API stores elements directly inside the queue object. Pushes
 * append at the logical back, pops remove from the logical front, and all
 * operations remain allocation-free.
 */
#ifndef SSTL_C_QUEUE_H
/** @def SSTL_C_QUEUE_H
 * @brief Include guard for sstl_queue.h.
 */
#define SSTL_C_QUEUE_H

#include "sstl_config.h"

/** @def SSTL_QUEUE_DECLARE
 * @brief Declare a typed fixed-capacity C FIFO queue API.
 */
#define SSTL_QUEUE_DECLARE(NAME, T, CAP) \
  typedef struct NAME { T data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Circular inline storage for queued values. */ size_t head; /**< @brief Physical index of the logical front element. */ size_t size; /**< @brief Number of live queue elements. */ } NAME; \
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
   * @brief Run the generated front operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_front(NAME* q); \
  /** \
   * @brief Try to access the front element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the front element, or null when empty. \
   */ \
  SSTL_C_INLINE T* NAME##_try_front(NAME* q); \
  /** \
   * @brief Run the generated front const operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE const T* NAME##_front_const(const NAME* q); \
  /** \
   * @brief Try to access the const front element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the front element, or null when empty. \
   */ \
  SSTL_C_INLINE const T* NAME##_try_front_const(const NAME* q); \
  /** \
   * @brief Run the generated back operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_back(NAME* q); \
  /** \
   * @brief Try to access the back element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the back element, or null when empty. \
   */ \
  SSTL_C_INLINE T* NAME##_try_back(NAME* q); \
  /** \
   * @brief Run the generated back const operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE const T* NAME##_back_const(const NAME* q); \
  /** \
   * @brief Try to access the const back element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the back element, or null when empty. \
   */ \
  SSTL_C_INLINE const T* NAME##_try_back_const(const NAME* q); \
  /** \
   * @brief Exchange two queues of the same generated type. \
   * @param a First queue. \
   * @param b Second queue. \
   */ \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b);

/** @def SSTL_QUEUE_DEFINE
 * @brief Define a typed fixed-capacity C FIFO queue API.
 */
#define SSTL_QUEUE_DEFINE(NAME, T, CAP) \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* q) { q->head = 0u; q->size = 0u; } \
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
  SSTL_C_INLINE bool NAME##_push(NAME* q, T x) { if (q->size == (size_t)(CAP)) { SSTL_C_PANIC("queue full"); return false; } q->data[(q->head + q->size) % ((CAP) == 0 ? 1u : (size_t)(CAP))] = x; ++q->size; return true; } \
  /** \
   * @brief Try to push without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_push(NAME* q, T x) { if (q->size == (size_t)(CAP)) return false; q->data[(q->head + q->size) % ((CAP) == 0 ? 1u : (size_t)(CAP))] = x; ++q->size; return true; } \
  /** \
   * @brief Run the generated pop operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_pop(NAME* q, T* out) { if (q->size == 0u) { SSTL_C_PANIC("queue empty"); return false; } if (out) *out = q->data[q->head]; q->head = (q->head + 1u) % ((CAP) == 0 ? 1u : (size_t)(CAP)); --q->size; return true; } \
  /** \
   * @brief Try to pop without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_pop(NAME* q, T* out) { if (q->size == 0u) return false; if (out) *out = q->data[q->head]; q->head = (q->head + 1u) % ((CAP) == 0 ? 1u : (size_t)(CAP)); --q->size; return true; } \
  /** \
   * @brief Run the generated front operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_front(NAME* q) { if (q->size == 0u) { SSTL_C_PANIC("queue front"); return 0; } return &q->data[q->head]; } \
  /** \
   * @brief Try to access the front element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the front element, or null when empty. \
   */ \
  SSTL_C_INLINE T* NAME##_try_front(NAME* q) { return q->size == 0u ? 0 : &q->data[q->head]; } \
  /** \
   * @brief Run the generated front const operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE const T* NAME##_front_const(const NAME* q) { if (q->size == 0u) { SSTL_C_PANIC("queue front"); return 0; } return &q->data[q->head]; } \
  /** \
   * @brief Try to access the const front element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the front element, or null when empty. \
   */ \
  SSTL_C_INLINE const T* NAME##_try_front_const(const NAME* q) { return q->size == 0u ? 0 : &q->data[q->head]; } \
  /** \
   * @brief Run the generated back operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_back(NAME* q) { if (q->size == 0u) { SSTL_C_PANIC("queue back"); return 0; } return &q->data[(q->head + q->size - 1u) % ((CAP) == 0 ? 1u : (size_t)(CAP))]; } \
  /** \
   * @brief Try to access the back element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the back element, or null when empty. \
   */ \
  SSTL_C_INLINE T* NAME##_try_back(NAME* q) { return q->size == 0u ? 0 : &q->data[(q->head + q->size - 1u) % ((CAP) == 0 ? 1u : (size_t)(CAP))]; } \
  /** \
   * @brief Run the generated back const operation for this typed SSTL family. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE const T* NAME##_back_const(const NAME* q) { if (q->size == 0u) { SSTL_C_PANIC("queue back"); return 0; } return &q->data[(q->head + q->size - 1u) % ((CAP) == 0 ? 1u : (size_t)(CAP))]; } \
  /** \
   * @brief Try to access the const back element without invoking the active error policy. \
   * @param q Queue or priority-queue instance. \
   * @return Pointer to the back element, or null when empty. \
   */ \
  SSTL_C_INLINE const T* NAME##_try_back_const(const NAME* q) { return q->size == 0u ? 0 : &q->data[(q->head + q->size - 1u) % ((CAP) == 0 ? 1u : (size_t)(CAP))]; } \
  /** \
   * @brief Exchange two queues of the same generated type. \
   * @param a First queue. \
   * @param b Second queue. \
   */ \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b) { NAME tmp = *a; *a = *b; *b = tmp; }

#endif
