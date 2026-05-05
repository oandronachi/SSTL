/**
 * @file sstl_unordered_set.h
 * @brief C99 SSTL fixed-bucket, fixed-capacity unordered set API.
 *
 * The C unordered_set macro uses object-owned bucket heads and slot chains.
 * Keys are compared and hashed through caller-supplied hooks, slots are
 * recycled through an intrusive free-list inside the object, and operations
 * never allocate heap memory.
 */
#ifndef SSTL_C_UNORDERED_SET_H
/** @def SSTL_C_UNORDERED_SET_H
 * @brief Include guard for sstl_unordered_set.h.
 */
#define SSTL_C_UNORDERED_SET_H

#include "sstl_algorithm.h"

/** @def SSTL_UNORDERED_SET_DECLARE
 * @brief Declare a typed, fixed-bucket C unordered_set family.
 * @param NAME Public function/type prefix.
 * @param K Key type stored by value.
 * @param CAP Fixed compile-time key capacity.
 * @param BUCKETS Fixed compile-time bucket count.
 * @param EQ Equality predicate with signature compatible with `bool eq(const void*, const void*)`.
 * @param HASH Hash hook with signature compatible with `size_t hash(const void*)`.
 */
#define SSTL_UNORDERED_SET_DECLARE(NAME, K, CAP, BUCKETS, EQ, HASH) \
  typedef struct NAME { K data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Inline slot storage for keys. */ size_t next[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Per-slot link for bucket chains while live, or free-list link while unused. */ bool used[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Per-slot live marker used by the fixed slot pool. */ size_t buckets[(BUCKETS) == 0 ? 1 : (BUCKETS)]; /**< @brief Bucket heads storing slot indexes or the null sentinel. */ size_t free_head; /**< @brief First reusable slot in the intrusive free-list, or the null sentinel. */ size_t size; /**< @brief Number of live keys currently stored. */ } NAME; \
  /** \
   * @brief Initialize all buckets and slots to the empty state. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_init(NAME* s); \
  /** \
   * @brief Return the number of live keys. \
   * @param s String or set instance. \
   * @return The number of live keys. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* s); \
  /** \
   * @brief Return the fixed key capacity. \
   * @param s String or set instance. \
   * @return The fixed key capacity. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_capacity(const NAME* s); \
  /** \
   * @brief Return true when no live keys are present. \
   * @param s String or set instance. \
   * @return True when no live keys are present. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* s); \
  /** \
   * @brief Return true when no free key slots remain. \
   * @param s String or set instance. \
   * @return True when no free key slots remain. \
   */ \
  static SSTL_C_UNUSED bool NAME##_full(const NAME* s); \
  /** \
   * @brief Return the compile-time bucket count. \
   * @param s String or set instance. \
   * @return The compile-time bucket count. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_bucket_count(const NAME* s); \
  /** \
   * @brief Return the bucket index for `key`, or zero when the bucket count is zero. \
   * @param key Lookup or insertion key. \
   * @return The bucket index for `key`, or zero when the bucket count is zero. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_bucket_index(K key); \
  /** \
   * @brief Return the live slot containing `key`, or the null slot sentinel when absent. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return The live slot containing `key`, or the null slot sentinel when absent. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_slot(NAME* s, K key); \
  /** \
   * @brief Return the stored key matching `key`, or null when absent. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return The stored key matching `key`, or null when absent. \
   */ \
  static SSTL_C_UNUSED K* NAME##_find(NAME* s, K key); \
  /** \
   * @brief Insert `key` into the hashed bucket chain. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_insert(NAME* s, K key); \
  /** \
   * @brief Erase `key` and return the number of keys removed. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_erase(NAME* s, K key);

/** @def SSTL_UNORDERED_SET_DEFINE
 * @brief Define a typed, fixed-bucket C unordered_set family.
 * @param NAME Public function/type prefix.
 * @param K Key type stored by value.
 * @param CAP Fixed compile-time key capacity.
 * @param BUCKETS Fixed compile-time bucket count.
 * @param EQ Equality predicate with signature compatible with `bool eq(const void*, const void*)`.
 * @param HASH Hash hook with signature compatible with `size_t hash(const void*)`.
 */
#define SSTL_UNORDERED_SET_DEFINE(NAME, K, CAP, BUCKETS, EQ, HASH) \
  /** \
   * @brief Initialize all buckets and slots to the empty state. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_init(NAME* s) { size_t i; s->free_head = (CAP) == 0 ? (size_t)-1 : 0u; for (i = 0u; i != (size_t)((BUCKETS) == 0 ? 1 : (BUCKETS)); ++i) s->buckets[i] = (size_t)-1; for (i = 0u; i != (size_t)((CAP) == 0 ? 1 : (CAP)); ++i) { s->next[i] = (i + 1u < (size_t)(CAP)) ? i + 1u : (size_t)-1; s->used[i] = false; } s->size = 0u; } \
  /** \
   * @brief Return the number of live keys. \
   * @param s String or set instance. \
   * @return The number of live keys. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* s) { return s->size; } \
  /** \
   * @brief Return the fixed key capacity. \
   * @param s String or set instance. \
   * @return The fixed key capacity. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_capacity(const NAME* s) { (void)s; return (size_t)(CAP); } \
  /** \
   * @brief Return true when no live keys are present. \
   * @param s String or set instance. \
   * @return True when no live keys are present. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* s) { return s->size == 0u; } \
  /** \
   * @brief Return true when no free key slots remain. \
   * @param s String or set instance. \
   * @return True when no free key slots remain. \
   */ \
  static SSTL_C_UNUSED bool NAME##_full(const NAME* s) { return s->size == (size_t)(CAP); } \
  /** \
   * @brief Return the compile-time bucket count. \
   * @param s String or set instance. \
   * @return The compile-time bucket count. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_bucket_count(const NAME* s) { (void)s; return (size_t)(BUCKETS); } \
  /** \
   * @brief Return the bucket index for `key`, or zero when the bucket count is zero. \
   * @param key Lookup or insertion key. \
   * @return The bucket index for `key`, or zero when the bucket count is zero. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_bucket_index(K key) { return (BUCKETS) == 0 ? 0u : (size_t)((HASH)((const void*)&key) % (unsigned)((BUCKETS) == 0 ? 1 : (BUCKETS))); } \
  /** \
   * @brief Return the live slot containing `key`, or the null slot sentinel when absent. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return The live slot containing `key`, or the null slot sentinel when absent. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_slot(NAME* s, K key) { size_t cur; if ((BUCKETS) == 0) return (size_t)-1; cur = s->buckets[NAME##_bucket_index(key)]; while (cur != (size_t)-1) { if (s->used[cur] && (EQ)((const void*)&s->data[cur], (const void*)&key)) return cur; cur = s->next[cur]; } return (size_t)-1; } \
  /** \
   * @brief Return the stored key matching `key`, or null when absent. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return The stored key matching `key`, or null when absent. \
   */ \
  static SSTL_C_UNUSED K* NAME##_find(NAME* s, K key) { size_t slot = NAME##_find_slot(s, key); return slot == (size_t)-1 ? 0 : &s->data[slot]; } \
  /** \
   * @brief Insert `key` into the hashed bucket chain. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_insert(NAME* s, K key) { size_t slot; size_t bucket; if (NAME##_find_slot(s, key) != (size_t)-1) return false; if (s->size == (size_t)(CAP) || (BUCKETS) == 0 || s->free_head == (size_t)-1) { SSTL_C_PANIC("unordered_set full"); return false; } slot = s->free_head; s->free_head = s->next[slot]; bucket = NAME##_bucket_index(key); s->data[slot] = key; s->used[slot] = true; s->next[slot] = s->buckets[bucket]; s->buckets[bucket] = slot; ++s->size; return true; } \
  /** \
   * @brief Erase `key` and return the number of keys removed. \
   * @param s String or set instance. \
   * @param key Lookup or insertion key. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_erase(NAME* s, K key) { size_t bucket; size_t prev = (size_t)-1; size_t cur; if ((BUCKETS) == 0) return 0u; bucket = NAME##_bucket_index(key); cur = s->buckets[bucket]; while (cur != (size_t)-1) { if (s->used[cur] && (EQ)((const void*)&s->data[cur], (const void*)&key)) { if (prev == (size_t)-1) s->buckets[bucket] = s->next[cur]; else s->next[prev] = s->next[cur]; s->used[cur] = false; s->next[cur] = s->free_head; s->free_head = cur; --s->size; return 1u; } prev = cur; cur = s->next[cur]; } return 0u; }

#endif

