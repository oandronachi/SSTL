/**
 * @file sstl_flat_map.h
 * @brief C99 SSTL sorted, contiguous, fixed-capacity map API.
 *
 * The C flat_map macro stores entries in ascending key order inside caller
 * owned object storage. Insertions shift later entries, never allocate heap
 * memory, and compare through a caller-supplied three-way comparator hook.
 * Operations return `false` when capacity or uniqueness rules reject the
 * operation under the return-oriented error policy.
 */
#ifndef SSTL_C_FLAT_MAP_H
/** @def SSTL_C_FLAT_MAP_H
 * @brief Include guard for sstl_flat_map.h.
 */
#define SSTL_C_FLAT_MAP_H

#include "sstl_algorithm.h"

/** @def SSTL_FLAT_MAP_DECLARE
 * @brief Declare a typed, sorted, fixed-capacity C flat_map family.
 * @param NAME Public function/type prefix.
 * @param K Key type stored by value.
 * @param V Mapped value type stored by value.
 * @param CAP Fixed compile-time entry capacity.
 * @param CMP Three-way comparator with signature compatible with `int cmp(const void*, const void*)`.
 */
#define SSTL_FLAT_MAP_DECLARE(NAME, K, V, CAP, CMP) \
  typedef struct NAME##_pair { K key; /**< @brief Stored key for this flat_map entry. */ V value; /**< @brief Stored value associated with key. */ } NAME##_pair; \
  typedef struct NAME { NAME##_pair data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Inline sorted entry storage; slot zero exists when CAP is zero. */ size_t size; /**< @brief Number of live sorted entries currently stored. */ } NAME; \
  /** \
   * @brief Initialize the flat_map to the empty sorted range. \
   * @param m Map instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* m); \
  /** \
   * @brief Return the number of live key/value entries. \
   * @param m Map instance. \
   * @return The number of live key/value entries. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* m); \
  /** \
   * @brief Return the fixed key/value entry capacity. \
   * @param m Map instance. \
   * @return The fixed key/value entry capacity. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* m); \
  /** \
   * @brief Return true when no live entries are present. \
   * @param m Map instance. \
   * @return True when no live entries are present. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* m); \
  /** \
   * @brief Return true when no more entries can be inserted. \
   * @param m Map instance. \
   * @return True when no more entries can be inserted. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* m); \
  SSTL_C_INLINE void NAME##_clear(NAME* m); \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b); \
  /** \
   * @brief Return the first index whose key is not less than `key`. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return The first index whose key is not less than `key`. \
   */ \
  SSTL_C_INLINE size_t NAME##_lower_bound(const NAME* m, K key); \
  SSTL_C_INLINE size_t NAME##_upper_bound(const NAME* m, K key); \
  SSTL_C_INLINE size_t NAME##_equal_range_first(const NAME* m, K key); \
  SSTL_C_INLINE size_t NAME##_equal_range_second(const NAME* m, K key); \
  /** \
   * @brief Return the value for `key`, or null when absent. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return The value for `key`, or null when absent. \
   */ \
  SSTL_C_INLINE V* NAME##_find(NAME* m, K key); \
  SSTL_C_INLINE size_t NAME##_count(NAME* m, K key); \
  /** \
   * @brief Return the pair at `index`, or null when `index` is outside the live range. \
   * @param m Map instance. \
   * @param index Caller-supplied argument used by this operation. \
   * @return The pair at `index`, or null when `index` is outside the live range. \
   */ \
  SSTL_C_INLINE NAME##_pair* NAME##_at_index(NAME* m, size_t index); \
  /** \
   * @brief Insert `key`/`value` while preserving sorted order. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert(NAME* m, K key, V value); \
  /** \
   * @brief Erase `key` and return the number of entries removed. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_erase(NAME* m, K key);

/** @def SSTL_FLAT_MAP_DEFINE
 * @brief Define a typed, sorted, fixed-capacity C flat_map family.
 * @param NAME Public function/type prefix.
 * @param K Key type stored by value.
 * @param V Mapped value type stored by value.
 * @param CAP Fixed compile-time entry capacity.
 * @param CMP Three-way comparator with signature compatible with `int cmp(const void*, const void*)`.
 */
#define SSTL_FLAT_MAP_DEFINE(NAME, K, V, CAP, CMP) \
  /** \
   * @brief Initialize the flat_map to the empty sorted range. \
   * @param m Map instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* m) { m->size = 0u; } \
  /** \
   * @brief Return the number of live key/value entries. \
   * @param m Map instance. \
   * @return The number of live key/value entries. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* m) { return m->size; } \
  /** \
   * @brief Return the fixed key/value entry capacity. \
   * @param m Map instance. \
   * @return The fixed key/value entry capacity. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* m) { (void)m; return (size_t)(CAP); } \
  /** \
   * @brief Return true when no live entries are present. \
   * @param m Map instance. \
   * @return True when no live entries are present. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* m) { return m->size == 0u; } \
  /** \
   * @brief Return true when no more entries can be inserted. \
   * @param m Map instance. \
   * @return True when no more entries can be inserted. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* m) { return m->size == (size_t)(CAP); } \
  SSTL_C_INLINE void NAME##_clear(NAME* m) { m->size = 0u; } \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b) { NAME tmp = *a; *a = *b; *b = tmp; } \
  /** \
   * @brief Return the first index whose key is not less than `key`. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return The first index whose key is not less than `key`. \
   */ \
  SSTL_C_INLINE size_t NAME##_lower_bound(const NAME* m, K key) { size_t i; for (i = 0u; i != m->size; ++i) if ((CMP)((const void*)&m->data[i].key, (const void*)&key) >= 0) return i; return m->size; } \
  SSTL_C_INLINE size_t NAME##_upper_bound(const NAME* m, K key) { size_t i; for (i = 0u; i != m->size; ++i) if ((CMP)((const void*)&key, (const void*)&m->data[i].key) < 0) return i; return m->size; } \
  SSTL_C_INLINE size_t NAME##_equal_range_first(const NAME* m, K key) { return NAME##_lower_bound(m, key); } \
  SSTL_C_INLINE size_t NAME##_equal_range_second(const NAME* m, K key) { return NAME##_upper_bound(m, key); } \
  /** \
   * @brief Return the value for `key`, or null when absent. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return The value for `key`, or null when absent. \
   */ \
  SSTL_C_INLINE V* NAME##_find(NAME* m, K key) { size_t i = NAME##_lower_bound(m, key); return i != m->size && (CMP)((const void*)&key, (const void*)&m->data[i].key) == 0 ? &m->data[i].value : 0; } \
  SSTL_C_INLINE size_t NAME##_count(NAME* m, K key) { return NAME##_find(m, key) ? 1u : 0u; } \
  /** \
   * @brief Return the pair at `index`, or null when `index` is outside the live range. \
   * @param m Map instance. \
   * @param index Caller-supplied argument used by this operation. \
   * @return The pair at `index`, or null when `index` is outside the live range. \
   */ \
  SSTL_C_INLINE NAME##_pair* NAME##_at_index(NAME* m, size_t index) { return index < m->size ? &m->data[index] : 0; } \
  /** \
   * @brief Insert `key`/`value` while preserving sorted order. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert(NAME* m, K key, V value) { size_t i; size_t pos = NAME##_lower_bound(m, key); if (pos != m->size && (CMP)((const void*)&key, (const void*)&m->data[pos].key) == 0) return false; if (m->size == (size_t)(CAP)) { SSTL_C_PANIC("flat_map full"); return false; } for (i = m->size; i != pos; --i) m->data[i] = m->data[i - 1u]; m->data[pos].key = key; m->data[pos].value = value; ++m->size; return true; } \
  /** \
   * @brief Erase `key` and return the number of entries removed. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_erase(NAME* m, K key) { size_t i; size_t pos = NAME##_lower_bound(m, key); if (pos == m->size || (CMP)((const void*)&key, (const void*)&m->data[pos].key) != 0) return 0u; for (i = pos + 1u; i != m->size; ++i) m->data[i - 1u] = m->data[i]; --m->size; return 1u; }

#endif

