/**
 * @file sstl_stack.h
 * @brief C99 fixed-capacity LIFO adapter backed by inline array storage.
 *
 * The generated API stores elements directly inside the stack object. Pushes
 * and pops touch only the logical top, so no external allocation or shifting is
 * needed.
 */
#ifndef SSTL_C_STACK_H
/** @def SSTL_C_STACK_H
 * @brief Include guard for sstl_stack.h.
 */
#define SSTL_C_STACK_H

#include "sstl_config.h"

/** @def SSTL_STACK_DECLARE
 * @brief Declare a typed fixed-capacity C LIFO stack API.
 */
#define SSTL_STACK_DECLARE(NAME, T, CAP) \
  typedef struct NAME { T data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Inline storage for stacked values. */ size_t size; /**< @brief Number of live stack elements. */ } NAME; \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_init(NAME* s); \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* s); \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_capacity(const NAME* s); \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* s); \
  /** \
   * @brief Run the generated full operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_full(const NAME* s); \
  /** \
   * @brief Run the generated push operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_push(NAME* s, T x); \
  /** \
   * @brief Run the generated pop operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_pop(NAME* s, T* out); \
  /** \
   * @brief Run the generated top operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_top(NAME* s); \
  /** \
   * @brief Run the generated top const operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED const T* NAME##_top_const(const NAME* s);

/** @def SSTL_STACK_DEFINE
 * @brief Define a typed fixed-capacity C LIFO stack API.
 */
#define SSTL_STACK_DEFINE(NAME, T, CAP) \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_init(NAME* s) { s->size = 0u; } \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* s) { return s->size; } \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_capacity(const NAME* s) { (void)s; return (size_t)(CAP); } \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* s) { return s->size == 0u; } \
  /** \
   * @brief Run the generated full operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_full(const NAME* s) { return s->size == (size_t)(CAP); } \
  /** \
   * @brief Run the generated push operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_push(NAME* s, T x) { if (s->size == (size_t)(CAP)) { SSTL_C_PANIC("stack full"); return false; } s->data[s->size++] = x; return true; } \
  /** \
   * @brief Run the generated pop operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_pop(NAME* s, T* out) { if (s->size == 0u) { SSTL_C_PANIC("stack empty"); return false; } --s->size; if (out) *out = s->data[s->size]; return true; } \
  /** \
   * @brief Run the generated top operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_top(NAME* s) { if (s->size == 0u) { SSTL_C_PANIC("stack top"); return 0; } return &s->data[s->size - 1u]; } \
  /** \
   * @brief Run the generated top const operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED const T* NAME##_top_const(const NAME* s) { if (s->size == 0u) { SSTL_C_PANIC("stack top"); return 0; } return &s->data[s->size - 1u]; }

#endif

