/**
 * @file sstl_unordered_map.h
 * @brief C99 SSTL fixed-bucket, fixed-capacity unordered map API.
 *
 * The C unordered_map macro uses object-owned bucket heads and slot chains.
 * Keys are compared and hashed through caller-supplied hooks, slots are
 * recycled through an intrusive free-list inside the object, and operations
 * never allocate heap memory.
 */
#ifndef SSTL_C_UNORDERED_MAP_H
/** @def SSTL_C_UNORDERED_MAP_H
 * @brief Include guard for sstl_unordered_map.h.
 */
#define SSTL_C_UNORDERED_MAP_H

#include "sstl_algorithm.h"

/** @def SSTL_UNORDERED_MAP_DECLARE
 * @brief Declare a typed, fixed-bucket C unordered_map family.
 * @param NAME Public function/type prefix.
 * @param K Key type stored by value.
 * @param V Mapped value type stored by value.
 * @param CAP Fixed compile-time entry capacity.
 * @param BUCKETS Fixed compile-time bucket count.
 * @param EQ Equality predicate with signature compatible with `bool eq(const void*, const void*)`.
 * @param HASH Hash hook with signature compatible with `size_t hash(const void*)`.
 */
#define SSTL_UNORDERED_MAP_DECLARE(NAME, K, V, CAP, BUCKETS, EQ, HASH) \
  typedef struct NAME##_pair { K key; /**< @brief Stored key for this unordered_map entry. */ V value; /**< @brief Stored value associated with key. */ } NAME##_pair; \
  typedef size_t NAME##_iterator; \
  typedef struct NAME { NAME##_pair data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Inline slot storage for key/value entries. */ size_t next[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Per-slot link for bucket chains while live, or free-list link while unused. */ bool used[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Per-slot live marker used by the fixed slot pool. */ size_t buckets[(BUCKETS) == 0 ? 1 : (BUCKETS)]; /**< @brief Bucket heads storing slot indexes or the null sentinel. */ size_t free_head; /**< @brief First reusable slot in the intrusive free-list, or the null sentinel. */ size_t size; /**< @brief Number of live entries currently stored. */ } NAME; \
  /** \
   * @brief Initialize all buckets and slots to the empty state. \
   * @param m Map instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* m); \
  /** \
   * @brief Return the number of live entries. \
   * @param m Map instance. \
   * @return The number of live entries. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* m); \
  /** \
   * @brief Return the fixed entry capacity. \
   * @param m Map instance. \
   * @return The fixed entry capacity. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* m); \
  /** \
   * @brief Return true when no live entries are present. \
   * @param m Map instance. \
   * @return True when no live entries are present. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* m); \
  /** \
   * @brief Return true when no free entry slots remain. \
   * @param m Map instance. \
   * @return True when no free entry slots remain. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* m); \
  SSTL_C_INLINE void NAME##_clear(NAME* m); \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b); \
  /** \
   * @brief Return the compile-time bucket count. \
   * @param m Map instance. \
   * @return The compile-time bucket count. \
   */ \
  SSTL_C_INLINE size_t NAME##_bucket_count(const NAME* m); \
  SSTL_C_INLINE float NAME##_load_factor(const NAME* m); \
  SSTL_C_INLINE float NAME##_max_load_factor(const NAME* m); \
  /** \
   * @brief Return the bucket index for `key`, or zero when the bucket count is zero. \
   * @param key Lookup or insertion key. \
   * @return The bucket index for `key`, or zero when the bucket count is zero. \
   */ \
  SSTL_C_INLINE size_t NAME##_bucket_index(K key); \
  /** \
   * @brief Return the live slot containing `key`, or the null slot sentinel when absent. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return The live slot containing `key`, or the null slot sentinel when absent. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_slot(const NAME* m, K key); \
  /** \
   * @brief Return the value for `key`, or null when absent. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return The value for `key`, or null when absent. \
   */ \
  SSTL_C_INLINE V* NAME##_find(NAME* m, K key); \
  SSTL_C_INLINE size_t NAME##_count(NAME* m, K key); \
  SSTL_C_INLINE NAME##_iterator NAME##_begin(const NAME* m); \
  SSTL_C_INLINE NAME##_iterator NAME##_end(const NAME* m); \
  SSTL_C_INLINE NAME##_iterator NAME##_next(const NAME* m, NAME##_iterator it); \
  SSTL_C_INLINE NAME##_pair* NAME##_deref(NAME* m, NAME##_iterator it); \
  SSTL_C_INLINE NAME##_iterator NAME##_equal_range_first(const NAME* m, K key); \
  SSTL_C_INLINE NAME##_iterator NAME##_equal_range_second(const NAME* m, K key); \
  /** \
   * @brief Insert `key`/`value` into the hashed bucket chain. \
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

/** @def SSTL_UNORDERED_MAP_DEFINE
 * @brief Define a typed, fixed-bucket C unordered_map family.
 * @param NAME Public function/type prefix.
 * @param K Key type stored by value.
 * @param V Mapped value type stored by value.
 * @param CAP Fixed compile-time entry capacity.
 * @param BUCKETS Fixed compile-time bucket count.
 * @param EQ Equality predicate with signature compatible with `bool eq(const void*, const void*)`.
 * @param HASH Hash hook with signature compatible with `size_t hash(const void*)`.
 */
#define SSTL_UNORDERED_MAP_DEFINE(NAME, K, V, CAP, BUCKETS, EQ, HASH) \
  /** \
   * @brief Initialize all buckets and slots to the empty state. \
   * @param m Map instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* m) { size_t i; m->free_head = (CAP) == 0 ? (size_t)-1 : 0u; for (i = 0u; i != (size_t)((BUCKETS) == 0 ? 1 : (BUCKETS)); ++i) m->buckets[i] = (size_t)-1; for (i = 0u; i != (size_t)((CAP) == 0 ? 1 : (CAP)); ++i) { m->next[i] = (i + 1u < (size_t)(CAP)) ? i + 1u : (size_t)-1; m->used[i] = false; } m->size = 0u; } \
  /** \
   * @brief Return the number of live entries. \
   * @param m Map instance. \
   * @return The number of live entries. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* m) { return m->size; } \
  /** \
   * @brief Return the fixed entry capacity. \
   * @param m Map instance. \
   * @return The fixed entry capacity. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* m) { (void)m; return (size_t)(CAP); } \
  /** \
   * @brief Return true when no live entries are present. \
   * @param m Map instance. \
   * @return True when no live entries are present. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* m) { return m->size == 0u; } \
  /** \
   * @brief Return true when no free entry slots remain. \
   * @param m Map instance. \
   * @return True when no free entry slots remain. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* m) { return m->size == (size_t)(CAP); } \
  SSTL_C_INLINE void NAME##_clear(NAME* m) { NAME##_init(m); } \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b) { NAME tmp = *a; *a = *b; *b = tmp; } \
  /** \
   * @brief Return the compile-time bucket count. \
   * @param m Map instance. \
   * @return The compile-time bucket count. \
   */ \
  SSTL_C_INLINE size_t NAME##_bucket_count(const NAME* m) { (void)m; return (size_t)(BUCKETS); } \
  SSTL_C_INLINE float NAME##_load_factor(const NAME* m) { return (BUCKETS) == 0 ? 0.0f : (float)m->size / (float)(BUCKETS); } \
  SSTL_C_INLINE float NAME##_max_load_factor(const NAME* m) { (void)m; return (BUCKETS) == 0 ? 0.0f : (float)(CAP) / (float)(BUCKETS); } \
  /** \
   * @brief Return the bucket index for `key`, or zero when the bucket count is zero. \
   * @param key Lookup or insertion key. \
   * @return The bucket index for `key`, or zero when the bucket count is zero. \
   */ \
  SSTL_C_INLINE size_t NAME##_bucket_index(K key) { return (BUCKETS) == 0 ? 0u : (size_t)((HASH)((const void*)&key) % (unsigned)((BUCKETS) == 0 ? 1 : (BUCKETS))); } \
  /** \
   * @brief Return the live slot containing `key`, or the null slot sentinel when absent. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return The live slot containing `key`, or the null slot sentinel when absent. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_slot(const NAME* m, K key) { size_t cur; if ((BUCKETS) == 0) return (size_t)-1; cur = m->buckets[NAME##_bucket_index(key)]; while (cur != (size_t)-1) { if (m->used[cur] && (EQ)((const void*)&m->data[cur].key, (const void*)&key)) return cur; cur = m->next[cur]; } return (size_t)-1; } \
  /** \
   * @brief Return the value for `key`, or null when absent. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return The value for `key`, or null when absent. \
   */ \
  SSTL_C_INLINE V* NAME##_find(NAME* m, K key) { size_t slot = NAME##_find_slot(m, key); return slot == (size_t)-1 ? 0 : &m->data[slot].value; } \
  SSTL_C_INLINE size_t NAME##_count(NAME* m, K key) { return NAME##_find_slot(m, key) == (size_t)-1 ? 0u : 1u; } \
  SSTL_C_INLINE NAME##_iterator NAME##_begin(const NAME* m) { size_t i; for (i = 0u; i != (size_t)(CAP); ++i) if (m->used[i]) return i; return (size_t)-1; } \
  SSTL_C_INLINE NAME##_iterator NAME##_end(const NAME* m) { (void)m; return (size_t)-1; } \
  SSTL_C_INLINE NAME##_iterator NAME##_next(const NAME* m, NAME##_iterator it) { size_t i; if (it == (size_t)-1) return (size_t)-1; for (i = it + 1u; i != (size_t)(CAP); ++i) if (m->used[i]) return i; return (size_t)-1; } \
  SSTL_C_INLINE NAME##_pair* NAME##_deref(NAME* m, NAME##_iterator it) { return it < (size_t)(CAP) && m->used[it] ? &m->data[it] : 0; } \
  SSTL_C_INLINE NAME##_iterator NAME##_equal_range_first(const NAME* m, K key) { return NAME##_find_slot(m, key); } \
  SSTL_C_INLINE NAME##_iterator NAME##_equal_range_second(const NAME* m, K key) { NAME##_iterator first = NAME##_equal_range_first(m, key); return first == (size_t)-1 ? (size_t)-1 : NAME##_next(m, first); } \
  /** \
   * @brief Insert `key`/`value` into the hashed bucket chain. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert(NAME* m, K key, V value) { size_t slot; size_t bucket; if (NAME##_find_slot(m, key) != (size_t)-1) return false; if (m->size == (size_t)(CAP) || (BUCKETS) == 0 || m->free_head == (size_t)-1) { SSTL_C_PANIC("unordered_map full"); return false; } slot = m->free_head; m->free_head = m->next[slot]; bucket = NAME##_bucket_index(key); m->data[slot].key = key; m->data[slot].value = value; m->used[slot] = true; m->next[slot] = m->buckets[bucket]; m->buckets[bucket] = slot; ++m->size; return true; } \
  /** \
   * @brief Erase `key` and return the number of entries removed. \
   * @param m Map instance. \
   * @param key Lookup or insertion key. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_erase(NAME* m, K key) { size_t bucket; size_t prev = (size_t)-1; size_t cur; if ((BUCKETS) == 0) return 0u; bucket = NAME##_bucket_index(key); cur = m->buckets[bucket]; while (cur != (size_t)-1) { if (m->used[cur] && (EQ)((const void*)&m->data[cur].key, (const void*)&key)) { if (prev == (size_t)-1) m->buckets[bucket] = m->next[cur]; else m->next[prev] = m->next[cur]; m->used[cur] = false; m->next[cur] = m->free_head; m->free_head = cur; --m->size; return 1u; } prev = cur; cur = m->next[cur]; } return 0u; }

#endif

