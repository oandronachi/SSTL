/**
 * @file sstl_vector.h
 * @brief C99 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_C_VECTOR_H
/** @def SSTL_C_VECTOR_H
 * @brief Include guard for sstl_vector.h.
 */
#define SSTL_C_VECTOR_H

#include "sstl_config.h"

/**
 * @brief Declare a typed fixed-capacity C vector API.
 * @param NAME Public struct/type prefix to generate.
 * @param T Element type stored directly in the inline array.
 * @param CAP Maximum number of live elements.
 *
 * The generated functions are `static` so each translation unit can instantiate
 * exactly the typed vectors it needs without a separate object file.
 */
#define SSTL_VECTOR_DECLARE(NAME, T, CAP) \
  typedef struct NAME { T data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Inline element array; slot zero exists even when CAP is zero. */ size_t size; /**< @brief Number of live elements currently stored in data. */ } NAME; \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_init(NAME* v); \
  /** \
   * @brief Run the generated clear operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_clear(NAME* v); \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* v); \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_capacity(const NAME* v); \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* v); \
  /** \
   * @brief Run the generated full operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_full(const NAME* v); \
  /** \
   * @brief Run the generated push back operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_push_back(NAME* v, T x); \
  /** \
   * @brief Run the generated try push back operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  static SSTL_C_UNUSED bool NAME##_try_push_back(NAME* v, T x); \
  /** \
   * @brief Run the generated resize operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param n Requested count or size. \
   * @param fill Value used for newly created slots. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_resize(NAME* v, size_t n, T fill); \
  /** \
   * @brief Run the generated try resize operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param n Requested count or size. \
   * @param fill Value used for newly created slots. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  static SSTL_C_UNUSED bool NAME##_try_resize(NAME* v, size_t n, T fill); \
  /** \
   * @brief Run the generated insert operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param pos Zero-based logical position. \
   * @param x Element value supplied by the caller. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_insert(NAME* v, size_t pos, T x); \
  /** \
   * @brief Run the generated try insert operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param pos Zero-based logical position. \
   * @param x Element value supplied by the caller. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  static SSTL_C_UNUSED bool NAME##_try_insert(NAME* v, size_t pos, T x); \
  /** \
   * @brief Run the generated erase operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param pos Zero-based logical position. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_erase(NAME* v, size_t pos, T* out); \
  /** \
   * @brief Run the generated pop back operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_pop_back(NAME* v, T* out); \
  /** \
   * @brief Run the generated try pop back operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  static SSTL_C_UNUSED bool NAME##_try_pop_back(NAME* v, T* out); \
  /** \
   * @brief Run the generated at operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param i Zero-based logical index. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_at(NAME* v, size_t i); \
  /** \
   * @brief Run the generated try at operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param i Zero-based logical index. \
   * @return Pointer to the requested object on success; null on failure. \
   */ \
  static SSTL_C_UNUSED T* NAME##_try_at(NAME* v, size_t i);

/**
 * @brief Define the functions declared by `SSTL_VECTOR_DECLARE`.
 * @param NAME Public struct/type prefix used in the declaration macro.
 * @param T Element type stored directly in the inline array.
 * @param CAP Maximum number of live elements.
 *
 * `NAME_push_back` follows the active SSTL C error policy on full capacity,
 * while `NAME_try_push_back` is always a quiet bool-returning probe.
 */
#define SSTL_VECTOR_DEFINE(NAME, T, CAP) \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_init(NAME* v) { v->size = 0u; } \
  /** \
   * @brief Run the generated clear operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_clear(NAME* v) { v->size = 0u; } \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* v) { return v->size; } \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_capacity(const NAME* v) { (void)v; return (size_t)(CAP); } \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* v) { return v->size == 0u; } \
  /** \
   * @brief Run the generated full operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_full(const NAME* v) { return v->size == (size_t)(CAP); } \
  /** \
   * @brief Run the generated push back operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_push_back(NAME* v, T x) { if (v->size == (size_t)(CAP)) { SSTL_C_PANIC("vector full"); return false; } v->data[v->size++] = x; return true; } \
  /** \
   * @brief Run the generated try push back operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param x Element value supplied by the caller. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  static SSTL_C_UNUSED bool NAME##_try_push_back(NAME* v, T x) { if (v->size == (size_t)(CAP)) return false; v->data[v->size++] = x; return true; } \
  /** \
   * @brief Run the generated resize operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param n Requested count or size. \
   * @param fill Value used for newly created slots. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_resize(NAME* v, size_t n, T fill) { if (n > (size_t)(CAP)) { SSTL_C_PANIC("vector resize"); return false; } while (v->size < n) v->data[v->size++] = fill; v->size = n; return true; } \
  /** \
   * @brief Run the generated try resize operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param n Requested count or size. \
   * @param fill Value used for newly created slots. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  static SSTL_C_UNUSED bool NAME##_try_resize(NAME* v, size_t n, T fill) { if (n > (size_t)(CAP)) return false; while (v->size < n) v->data[v->size++] = fill; v->size = n; return true; } \
  /** \
   * @brief Run the generated insert operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param pos Zero-based logical position. \
   * @param x Element value supplied by the caller. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_insert(NAME* v, size_t pos, T x) { size_t i; if (pos > v->size || v->size == (size_t)(CAP)) { SSTL_C_PANIC("vector insert"); return false; } for (i = v->size; i != pos; --i) v->data[i] = v->data[i - 1u]; v->data[pos] = x; ++v->size; return true; } \
  /** \
   * @brief Run the generated try insert operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param pos Zero-based logical position. \
   * @param x Element value supplied by the caller. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  static SSTL_C_UNUSED bool NAME##_try_insert(NAME* v, size_t pos, T x) { size_t i; if (pos > v->size || v->size == (size_t)(CAP)) return false; for (i = v->size; i != pos; --i) v->data[i] = v->data[i - 1u]; v->data[pos] = x; ++v->size; return true; } \
  /** \
   * @brief Run the generated erase operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param pos Zero-based logical position. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_erase(NAME* v, size_t pos, T* out) { size_t i; if (pos >= v->size) { SSTL_C_PANIC("vector erase"); return false; } if (out) *out = v->data[pos]; for (i = pos + 1u; i != v->size; ++i) v->data[i - 1u] = v->data[i]; --v->size; return true; } \
  /** \
   * @brief Run the generated pop back operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_pop_back(NAME* v, T* out) { if (v->size == 0u) { SSTL_C_PANIC("vector empty"); return false; } --v->size; if (out) *out = v->data[v->size]; return true; } \
  /** \
   * @brief Run the generated try pop back operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  static SSTL_C_UNUSED bool NAME##_try_pop_back(NAME* v, T* out) { if (v->size == 0u) return false; --v->size; if (out) *out = v->data[v->size]; return true; } \
  /** \
   * @brief Run the generated at operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param i Zero-based logical index. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_at(NAME* v, size_t i) { if (i >= v->size) { SSTL_C_PANIC("vector at"); return 0; } return &v->data[i]; } \
  /** \
   * @brief Run the generated try at operation for this typed SSTL family. \
   * @param v Generated object or variant instance. \
   * @param i Zero-based logical index. \
   * @return Pointer to the requested object on success; null on failure. \
   */ \
  static SSTL_C_UNUSED T* NAME##_try_at(NAME* v, size_t i) { return i < v->size ? &v->data[i] : 0; }

#endif

