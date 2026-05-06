/**
 * @file sstl_flat_set.h
 * @brief C99 SSTL sorted, contiguous, fixed-capacity set API.
 *
 * The C flat_set macro stores unique keys in ascending order inside the object
 * itself. The implementation is intentionally array-backed: insertion shifts
 * later keys, lookup uses the sorted invariant through a comparator hook, and
 * no operation performs heap allocation.
 */
#ifndef SSTL_C_FLAT_SET_H
/** @def SSTL_C_FLAT_SET_H
 * @brief Include guard for sstl_flat_set.h.
 */
#define SSTL_C_FLAT_SET_H

#include "sstl_algorithm.h"

/** @def SSTL_FLAT_SET_DECLARE
 * @brief Declare a typed, sorted, fixed-capacity C flat_set family.
 * @param NAME Public function/type prefix.
 * @param K Key type stored by value.
 * @param CAP Fixed compile-time key capacity.
 * @param CMP Three-way comparator with signature compatible with `int cmp(const void*, const void*)`.
 */
#define SSTL_FLAT_SET_DECLARE(NAME, K, CAP, CMP) \
  typedef struct NAME { K data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Inline sorted key storage; slot zero exists when CAP is zero. */ size_t size; /**< @brief Number of live keys currently stored. */ } NAME; \
  /** \
   * @brief Initialize the flat_set to the empty sorted range. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* s); \
  /** \
   * @brief Return the number of live keys. \
   * @param s String or set instance. \
   * @return The number of live keys. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* s); \
  /** \
   * @brief Return the fixed key capacity. \
   * @param s String or set instance. \
   * @return The fixed key capacity. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* s); \
  /** \
   * @brief Return true when no live keys are present. \
   * @param s String or set instance. \
   * @return True when no live keys are present. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* s); \
  /** \
   * @brief Return true when no more keys can be inserted. \
   * @param s String or set instance. \
   * @return True when no more keys can be inserted. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* s); \
  SSTL_C_INLINE void NAME##_clear(NAME* s); \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b); \
  /** \
   * @brief Return the first index whose key is not less than `key`. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return The first index whose key is not less than `key`. \
   */ \
  SSTL_C_INLINE size_t NAME##_lower_bound(const NAME* s, K key); \
  SSTL_C_INLINE size_t NAME##_upper_bound(const NAME* s, K key); \
  SSTL_C_INLINE size_t NAME##_equal_range_first(const NAME* s, K key); \
  SSTL_C_INLINE size_t NAME##_equal_range_second(const NAME* s, K key); \
  /** \
   * @brief Return the stored key matching `key`, or null when absent. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return The stored key matching `key`, or null when absent. \
   */ \
  SSTL_C_INLINE K* NAME##_find(NAME* s, K key); \
  SSTL_C_INLINE size_t NAME##_count(NAME* s, K key); \
  /** \
   * @brief Insert `key` while preserving sorted order and uniqueness. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert(NAME* s, K key); \
  /** \
   * @brief Erase `key` and return the number of keys removed. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_erase(NAME* s, K key);

/** @def SSTL_FLAT_SET_DEFINE
 * @brief Define a typed, sorted, fixed-capacity C flat_set family.
 * @param NAME Public function/type prefix.
 * @param K Key type stored by value.
 * @param CAP Fixed compile-time key capacity.
 * @param CMP Three-way comparator with signature compatible with `int cmp(const void*, const void*)`.
 */
#define SSTL_FLAT_SET_DEFINE(NAME, K, CAP, CMP) \
  /** \
   * @brief Initialize the flat_set to the empty sorted range. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* s) { s->size = 0u; } \
  /** \
   * @brief Return the number of live keys. \
   * @param s String or set instance. \
   * @return The number of live keys. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* s) { return s->size; } \
  /** \
   * @brief Return the fixed key capacity. \
   * @param s String or set instance. \
   * @return The fixed key capacity. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* s) { (void)s; return (size_t)(CAP); } \
  /** \
   * @brief Return true when no live keys are present. \
   * @param s String or set instance. \
   * @return True when no live keys are present. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* s) { return s->size == 0u; } \
  /** \
   * @brief Return true when no more keys can be inserted. \
   * @param s String or set instance. \
   * @return True when no more keys can be inserted. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* s) { return s->size == (size_t)(CAP); } \
  SSTL_C_INLINE void NAME##_clear(NAME* s) { s->size = 0u; } \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b) { NAME tmp = *a; *a = *b; *b = tmp; } \
  /** \
   * @brief Return the first index whose key is not less than `key`. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return The first index whose key is not less than `key`. \
   */ \
  SSTL_C_INLINE size_t NAME##_lower_bound(const NAME* s, K key) { size_t i; for (i = 0u; i != s->size; ++i) if ((CMP)((const void*)&s->data[i], (const void*)&key) >= 0) return i; return s->size; } \
  SSTL_C_INLINE size_t NAME##_upper_bound(const NAME* s, K key) { size_t i; for (i = 0u; i != s->size; ++i) if ((CMP)((const void*)&key, (const void*)&s->data[i]) < 0) return i; return s->size; } \
  SSTL_C_INLINE size_t NAME##_equal_range_first(const NAME* s, K key) { return NAME##_lower_bound(s, key); } \
  SSTL_C_INLINE size_t NAME##_equal_range_second(const NAME* s, K key) { return NAME##_upper_bound(s, key); } \
  /** \
   * @brief Return the stored key matching `key`, or null when absent. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return The stored key matching `key`, or null when absent. \
   */ \
  SSTL_C_INLINE K* NAME##_find(NAME* s, K key) { size_t i = NAME##_lower_bound(s, key); return i != s->size && (CMP)((const void*)&key, (const void*)&s->data[i]) == 0 ? &s->data[i] : 0; } \
  SSTL_C_INLINE size_t NAME##_count(NAME* s, K key) { return NAME##_find(s, key) ? 1u : 0u; } \
  /** \
   * @brief Insert `key` while preserving sorted order and uniqueness. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert(NAME* s, K key) { size_t i; size_t pos = NAME##_lower_bound(s, key); if (pos != s->size && (CMP)((const void*)&key, (const void*)&s->data[pos]) == 0) return false; if (s->size == (size_t)(CAP)) { SSTL_C_PANIC("flat_set full"); return false; } for (i = s->size; i != pos; --i) s->data[i] = s->data[i - 1u]; s->data[pos] = key; ++s->size; return true; } \
  /** \
   * @brief Erase `key` and return the number of keys removed. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_erase(NAME* s, K key) { size_t i; size_t pos = NAME##_lower_bound(s, key); if (pos == s->size || (CMP)((const void*)&key, (const void*)&s->data[pos]) != 0) return 0u; for (i = pos + 1u; i != s->size; ++i) s->data[i - 1u] = s->data[i]; --s->size; return 1u; }

#endif

