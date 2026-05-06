/**
 * @file sstl_forward_list.h
 * @brief C99 fixed-capacity forward_list backed by an object-owned node pool.
 *
 * Generated forward-list families expose opaque slot iterators. Insertions and
 * erasures relink nodes from a static free-list instead of shifting a compact
 * array, so iterators to unaffected nodes keep naming the same node slot.
 */
#ifndef SSTL_C_FORWARD_LIST_H
/** @def SSTL_C_FORWARD_LIST_H
 * @brief Include guard for sstl_forward_list.h.
 */
#define SSTL_C_FORWARD_LIST_H

#include "sstl_config.h"

/** @def SSTL_FORWARD_LIST_DECLARE
 * @brief Declare a typed, fixed-capacity C forward_list family.
 */
#define SSTL_FORWARD_LIST_DECLARE(NAME, T, CAP) \
  typedef size_t NAME##_iterator; \
  typedef struct NAME##_node { T value; /**< @brief Stored element value while `used` is true. */ size_t next; /**< @brief Next live node or free node slot. */ bool used; /**< @brief Whether this pool slot currently owns a live value. */ } NAME##_node; \
  typedef struct NAME { NAME##_node nodes[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Static node pool owned by this list object. */ size_t head; /**< @brief First live node slot, or end sentinel. */ size_t free_head; /**< @brief First free node slot, or end sentinel. */ size_t size; /**< @brief Number of live nodes. */ } NAME; \
  /** \
   * @brief Initialize the forward_list to the empty state. \
   * @param l List instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* l); \
  /** \
   * @brief Return true when no elements are present. \
   * @param l List instance. \
   * @return True when no elements are present. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* l); \
  /** \
   * @brief Return true when no more elements can be inserted. \
   * @param l List instance. \
   * @return True when no more elements can be inserted. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* l); \
  /** \
   * @brief Return the fixed element capacity. \
   * @param l List instance. \
   * @return The fixed element capacity. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* l); \
  /** \
   * @brief Return the number of live elements. \
   * @param l List instance. \
   * @return The number of live elements. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* l); \
  /** \
   * @brief Return the sentinel iterator representing the position before the first element. \
   * @param l List instance. \
   * @return The sentinel iterator representing the position before the first element. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_before_begin(const NAME* l); \
  /** \
   * @brief Return an iterator to the first element, or end when the list is empty. \
   * @param l List instance. \
   * @return An iterator to the first element, or end when the list is empty. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_begin(const NAME* l); \
  /** \
   * @brief Return the one-past-end iterator. \
   * @param l List instance. \
   * @return The one-past-end iterator. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_end(const NAME* l); \
  /** \
   * @brief Advance an iterator, treating before_begin as advancing to begin. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_next(const NAME* l, NAME##_iterator pos); \
  /** \
   * @brief Return a pointer to the element at `pos`, or null when `pos` is not dereferenceable. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @return A pointer to the element at `pos`, or null when `pos` is not dereferenceable. \
   */ \
  SSTL_C_INLINE T* NAME##_at(NAME* l, NAME##_iterator pos); \
  /** \
   * @brief Return a pointer to the element at `pos`, or null when `pos` is not dereferenceable. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @return A pointer to the element at `pos`, or null when `pos` is not dereferenceable. \
   */ \
  SSTL_C_INLINE T* NAME##_deref(NAME* l, NAME##_iterator pos); \
  /** \
   * @brief Return a pointer to the first element, or null when empty under return policy. \
   * @param l List instance. \
   * @return A pointer to the first element, or null when empty under return policy. \
   */ \
  SSTL_C_INLINE T* NAME##_front(NAME* l); \
  /** \
   * @brief Try to return the first element without invoking the active error policy. \
   * @param l List instance. \
   * @return Pointer to the first element, or null when empty. \
   */ \
  SSTL_C_INLINE T* NAME##_try_front(NAME* l); \
  /** \
   * @brief Insert `value` at the front of the list. \
   * @param l List instance. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_push_front(NAME* l, T value); \
  /** \
   * @brief Try to insert `value` at the front without invoking the active error policy. \
   * @param l List instance. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_push_front(NAME* l, T value); \
  /** \
   * @brief Remove the first element, optionally copying it to `out`. \
   * @param l List instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_pop_front(NAME* l, T* out); \
  /** \
   * @brief Try to remove the first element without invoking the active error policy. \
   * @param l List instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_pop_front(NAME* l, T* out); \
  /** \
   * @brief Insert `value` after `pos` and return the new element iterator or end on failure. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_insert_after(NAME* l, NAME##_iterator pos, T value); \
  /** \
   * @brief Try to insert `value` after `pos` without invoking the active error policy. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_try_insert_after(NAME* l, NAME##_iterator pos, T value); \
  /** \
   * @brief Erase the element after `pos`, optionally copying it to `out`, and return the successor iterator. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param out Caller-provided destination for produced values. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_erase_after(NAME* l, NAME##_iterator pos, T* out); \
  /** \
   * @brief Move all elements from `other` after `pos` using destination-owned storage. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param other Other object participating in the operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_splice_after(NAME* l, NAME##_iterator pos, NAME* other); \
  /** \
   * @brief Try to move all elements from `other` after `pos` without invoking the active error policy. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param other Other object participating in the operation. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_splice_after(NAME* l, NAME##_iterator pos, NAME* other); \
  /** \
   * @brief Move one element from `other` after `before` into `l` after `pos`. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param other Other object participating in the operation. \
   * @param before Caller-supplied argument used by this operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_splice_after_one(NAME* l, NAME##_iterator pos, NAME* other, NAME##_iterator before); \
  /** \
   * @brief Try to move one element from `other` after `before` into `l` after `pos`. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param other Other object participating in the operation. \
   * @param before Caller-supplied argument used by this operation. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_splice_after_one(NAME* l, NAME##_iterator pos, NAME* other, NAME##_iterator before); \
  /** \
   * @brief Compare two stored values byte-for-byte for value-family helpers. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_value_equal(const T* a, const T* b); \
  /** \
   * @brief Remove all elements equal to `value`. \
   * @param l List instance. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_remove(NAME* l, T value); \
  /** \
   * @brief Remove all elements for which `pred` returns true. \
   * @param l List instance. \
   * @param pred Predicate used to test elements. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_remove_if(NAME* l, bool (*pred)(const T*)); \
  /** \
   * @brief Remove adjacent duplicate elements. \
   * @param l List instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_unique(NAME* l); \
  /** \
   * @brief Sort the list in ascending order according to `cmp`. \
   * @param l List instance. \
   * @param cmp Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_sort(NAME* l, sstl_cmp_fn cmp); \
  /** \
   * @brief Merge sorted `other` into `l` while preserving sorted order. \
   * @param l List instance. \
   * @param other Other object participating in the operation. \
   * @param cmp Caller-supplied argument used by this operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_merge(NAME* l, NAME* other, sstl_cmp_fn cmp); \
  /** \
   * @brief Try to merge sorted `other` into `l` without invoking the active error policy. \
   * @param l List instance. \
   * @param other Other object participating in the operation. \
   * @param cmp Caller-supplied argument used by this operation. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_merge(NAME* l, NAME* other, sstl_cmp_fn cmp);

/** @def SSTL_FORWARD_LIST_DEFINE
 * @brief Define a typed, fixed-capacity C forward_list family.
 */
#define SSTL_FORWARD_LIST_DEFINE(NAME, T, CAP) \
  /** \
   * @brief Return the private end sentinel used by this generated family. \
   * @return The private end sentinel used by this generated family. \
   */ \
  SSTL_C_INLINE size_t NAME##_end_sentinel(void) { return (size_t)-1; } \
  /** \
   * @brief Return the private before-begin sentinel used by this generated family. \
   * @return The private before-begin sentinel used by this generated family. \
   */ \
  SSTL_C_INLINE size_t NAME##_before_sentinel(void) { return (size_t)-2; } \
  /** \
   * @brief Initialize the forward_list to the empty state. \
   * @param l List instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* l) { size_t i; l->head = NAME##_end_sentinel(); l->free_head = (CAP) == 0 ? NAME##_end_sentinel() : 0u; l->size = 0u; for (i = 0u; i != (size_t)(CAP); ++i) { l->nodes[i].next = (i + 1u < (size_t)(CAP)) ? i + 1u : NAME##_end_sentinel(); l->nodes[i].used = false; } } \
  /** \
   * @brief Return true when no elements are present. \
   * @param l List instance. \
   * @return True when no elements are present. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* l) { return l->size == 0u; } \
  /** \
   * @brief Return true when no more elements can be inserted. \
   * @param l List instance. \
   * @return True when no more elements can be inserted. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* l) { return l->size == (size_t)(CAP); } \
  /** \
   * @brief Return the fixed element capacity. \
   * @param l List instance. \
   * @return The fixed element capacity. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* l) { (void)l; return (size_t)(CAP); } \
  /** \
   * @brief Return the number of live elements. \
   * @param l List instance. \
   * @return The number of live elements. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* l) { return l->size; } \
  /** \
   * @brief Return the sentinel iterator representing the position before the first element. \
   * @param l List instance. \
   * @return The sentinel iterator representing the position before the first element. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_before_begin(const NAME* l) { (void)l; return NAME##_before_sentinel(); } \
  /** \
   * @brief Return an iterator to the first element, or end when the list is empty. \
   * @param l List instance. \
   * @return An iterator to the first element, or end when the list is empty. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_begin(const NAME* l) { return l->head; } \
  /** \
   * @brief Return the one-past-end iterator. \
   * @param l List instance. \
   * @return The one-past-end iterator. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_end(const NAME* l) { (void)l; return NAME##_end_sentinel(); } \
  /** \
   * @brief Advance an iterator, treating before_begin as advancing to begin. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_next(const NAME* l, NAME##_iterator pos) { if (pos == NAME##_before_sentinel()) return l->head; return (pos < (size_t)(CAP) && l->nodes[pos].used) ? l->nodes[pos].next : NAME##_end_sentinel(); } \
  /** \
   * @brief Return a pointer to the element at `pos`, or null when `pos` is not dereferenceable. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @return A pointer to the element at `pos`, or null when `pos` is not dereferenceable. \
   */ \
  SSTL_C_INLINE T* NAME##_at(NAME* l, NAME##_iterator pos) { return (pos < (size_t)(CAP) && l->nodes[pos].used) ? &l->nodes[pos].value : 0; } \
  /** \
   * @brief Return a pointer to the element at `pos`, or null when `pos` is not dereferenceable. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @return A pointer to the element at `pos`, or null when `pos` is not dereferenceable. \
   */ \
  SSTL_C_INLINE T* NAME##_deref(NAME* l, NAME##_iterator pos) { return NAME##_at(l, pos); } \
  /** \
   * @brief Return a pointer to the first element, or null when empty under return policy. \
   * @param l List instance. \
   * @return A pointer to the first element, or null when empty under return policy. \
   */ \
  SSTL_C_INLINE T* NAME##_front(NAME* l) { if (l->head == NAME##_end_sentinel()) { SSTL_C_PANIC("forward_list empty"); return 0; } return &l->nodes[l->head].value; } \
  /** \
   * @brief Try to return the first element without invoking the active error policy. \
   * @param l List instance. \
   * @return Pointer to the first element, or null when empty. \
   */ \
  SSTL_C_INLINE T* NAME##_try_front(NAME* l) { return l->head == NAME##_end_sentinel() ? 0 : &l->nodes[l->head].value; } \
  /** \
   * @brief Reserve a free node slot and copy `value` into it. \
   * @param l List instance. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_allocate_node(NAME* l, T value, size_t* out) { size_t slot; if (l->size == (size_t)(CAP) || l->free_head == NAME##_end_sentinel()) { SSTL_C_PANIC("forward_list full"); return false; } slot = l->free_head; l->free_head = l->nodes[slot].next; l->nodes[slot].value = value; l->nodes[slot].next = NAME##_end_sentinel(); l->nodes[slot].used = true; *out = slot; return true; } \
  /** \
   * @brief Return a node slot to the free-list. \
   * @param l List instance. \
   * @param slot Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_release_node(NAME* l, size_t slot) { l->nodes[slot].used = false; l->nodes[slot].next = l->free_head; l->free_head = slot; } \
  /** \
   * @brief Insert `value` at the front of the list. \
   * @param l List instance. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_push_front(NAME* l, T value) { return NAME##_insert_after(l, NAME##_before_begin(l), value) != NAME##_end(l); } \
  /** \
   * @brief Try to insert `value` at the front without invoking the active error policy. \
   * @param l List instance. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_push_front(NAME* l, T value) { if (l->size == (size_t)(CAP) || l->free_head == NAME##_end_sentinel()) return false; return NAME##_insert_after(l, NAME##_before_begin(l), value) != NAME##_end(l); } \
  /** \
   * @brief Remove the first element, optionally copying it to `out`. \
   * @param l List instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_pop_front(NAME* l, T* out) { size_t before = l->size; (void)NAME##_erase_after(l, NAME##_before_begin(l), out); return l->size != before; } \
  /** \
   * @brief Try to remove the first element without invoking the active error policy. \
   * @param l List instance. \
   * @param out Caller-provided destination for produced values. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_pop_front(NAME* l, T* out) { if (l->head == NAME##_end_sentinel()) return false; return NAME##_erase_after(l, NAME##_before_begin(l), out) != NAME##_end(l) || l->head == NAME##_end_sentinel(); } \
  /** \
   * @brief Insert `value` after `pos` and return the new element iterator or end on failure. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_insert_after(NAME* l, NAME##_iterator pos, T value) { size_t slot; if (pos != NAME##_before_sentinel() && (pos >= (size_t)(CAP) || !l->nodes[pos].used)) { SSTL_C_PANIC("forward_list bad insert_after"); return NAME##_end(l); } if (!NAME##_allocate_node(l, value, &slot)) return NAME##_end(l); if (pos == NAME##_before_sentinel()) { l->nodes[slot].next = l->head; l->head = slot; } else { l->nodes[slot].next = l->nodes[pos].next; l->nodes[pos].next = slot; } ++l->size; return slot; } \
  /** \
   * @brief Try to insert `value` after `pos` without invoking the active error policy. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_try_insert_after(NAME* l, NAME##_iterator pos, T value) { if (pos != NAME##_before_sentinel() && (pos >= (size_t)(CAP) || !l->nodes[pos].used)) return NAME##_end(l); if (l->size == (size_t)(CAP) || l->free_head == NAME##_end_sentinel()) return NAME##_end(l); return NAME##_insert_after(l, pos, value); } \
  /** \
   * @brief Erase the element after `pos`, optionally copying it to `out`, and return the successor iterator. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param out Caller-provided destination for produced values. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE NAME##_iterator NAME##_erase_after(NAME* l, NAME##_iterator pos, T* out) { size_t target; size_t after; if (pos == NAME##_before_sentinel()) target = l->head; else if (pos < (size_t)(CAP) && l->nodes[pos].used) target = l->nodes[pos].next; else return NAME##_end(l); if (target == NAME##_end_sentinel()) return NAME##_end(l); after = l->nodes[target].next; if (out) *out = l->nodes[target].value; if (pos == NAME##_before_sentinel()) l->head = after; else l->nodes[pos].next = after; NAME##_release_node(l, target); --l->size; return after; } \
  /** \
   * @brief Move all elements from `other` after `pos` using destination-owned storage. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param other Other object participating in the operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_try_splice_after(NAME* l, NAME##_iterator pos, NAME* other) { size_t it; size_t insert_pos = pos; if (l == other || other->size == 0u) return true; if (pos != NAME##_before_sentinel() && (pos >= (size_t)(CAP) || !l->nodes[pos].used)) return false; if ((size_t)(CAP) - l->size < other->size) return false; for (it = other->head; it != NAME##_end_sentinel(); it = other->nodes[it].next) insert_pos = NAME##_insert_after(l, insert_pos, other->nodes[it].value); NAME##_init(other); return true; } \
  SSTL_C_INLINE bool NAME##_splice_after(NAME* l, NAME##_iterator pos, NAME* other) { if (!NAME##_try_splice_after(l, pos, other)) { SSTL_C_PANIC("forward_list splice_after"); return false; } return true; } \
  /** \
   * @brief Move one element from `other` after `before` into `l` after `pos`. \
   * @param l List instance. \
   * @param pos Zero-based logical position. \
   * @param other Other object participating in the operation. \
   * @param before Caller-supplied argument used by this operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_try_splice_after_one(NAME* l, NAME##_iterator pos, NAME* other, NAME##_iterator before) { T value; size_t target; if (l == other) return true; if (pos != NAME##_before_sentinel() && (pos >= (size_t)(CAP) || !l->nodes[pos].used)) return false; if (l->size == (size_t)(CAP)) return false; target = before == NAME##_before_sentinel() ? other->head : ((before < (size_t)(CAP) && other->nodes[before].used) ? other->nodes[before].next : NAME##_end_sentinel()); if (target == NAME##_end_sentinel()) return false; (void)NAME##_erase_after(other, before, &value); return NAME##_insert_after(l, pos, value) != NAME##_end(l); } \
  SSTL_C_INLINE bool NAME##_splice_after_one(NAME* l, NAME##_iterator pos, NAME* other, NAME##_iterator before) { if (!NAME##_try_splice_after_one(l, pos, other, before)) { SSTL_C_PANIC("forward_list splice_after_one"); return false; } return true; } \
  /** \
   * @brief Compare two stored values byte-for-byte for value-family helpers. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_value_equal(const T* a, const T* b) { size_t i; const unsigned char* pa = (const unsigned char*)a; const unsigned char* pb = (const unsigned char*)b; for (i = 0u; i != sizeof(T); ++i) if (pa[i] != pb[i]) return false; return true; } \
  /** \
   * @brief Swap two stored values in place. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_swap_values(T* a, T* b) { T tmp = *a; *a = *b; *b = tmp; } \
  /** \
   * @brief Remove all elements equal to `value`. \
   * @param l List instance. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_remove(NAME* l, T value) { size_t prev = NAME##_before_begin(l); while (NAME##_next(l, prev) != NAME##_end(l)) { size_t current = NAME##_next(l, prev); if (NAME##_value_equal(&l->nodes[current].value, &value)) (void)NAME##_erase_after(l, prev, 0); else prev = current; } } \
  /** \
   * @brief Remove all elements for which `pred` returns true. \
   * @param l List instance. \
   * @param pred Predicate used to test elements. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_remove_if(NAME* l, bool (*pred)(const T*)) { size_t prev = NAME##_before_begin(l); while (NAME##_next(l, prev) != NAME##_end(l)) { size_t current = NAME##_next(l, prev); if (pred(&l->nodes[current].value)) (void)NAME##_erase_after(l, prev, 0); else prev = current; } } \
  /** \
   * @brief Remove adjacent duplicate elements. \
   * @param l List instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_unique(NAME* l) { size_t current = l->head; while (current != NAME##_end_sentinel() && l->nodes[current].next != NAME##_end_sentinel()) { size_t next = l->nodes[current].next; if (NAME##_value_equal(&l->nodes[current].value, &l->nodes[next].value)) (void)NAME##_erase_after(l, current, 0); else current = next; } } \
  /** \
   * @brief Sort the list in ascending order according to `cmp`. \
   * @param l List instance. \
   * @param cmp Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_sort(NAME* l, sstl_cmp_fn cmp) { size_t i; for (i = l->head; i != NAME##_end_sentinel(); i = l->nodes[i].next) { size_t best = i; size_t j; for (j = l->nodes[i].next; j != NAME##_end_sentinel(); j = l->nodes[j].next) if (cmp((const void*)&l->nodes[j].value, (const void*)&l->nodes[best].value) < 0) best = j; if (best != i) NAME##_swap_values(&l->nodes[i].value, &l->nodes[best].value); } } \
  /** \
   * @brief Merge sorted `other` into `l` while preserving sorted order. \
   * @param l List instance. \
   * @param other Other object participating in the operation. \
   * @param cmp Caller-supplied argument used by this operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_try_merge(NAME* l, NAME* other, sstl_cmp_fn cmp) { size_t it; if (l == other) return true; if ((size_t)(CAP) - l->size < other->size) return false; for (it = other->head; it != NAME##_end_sentinel(); it = other->nodes[it].next) (void)NAME##_insert_after(l, NAME##_end_sentinel() == l->head ? NAME##_before_begin(l) : l->head, other->nodes[it].value); NAME##_sort(l, cmp); NAME##_init(other); return true; } \
  SSTL_C_INLINE bool NAME##_merge(NAME* l, NAME* other, sstl_cmp_fn cmp) { if (!NAME##_try_merge(l, other, cmp)) { SSTL_C_PANIC("forward_list merge full"); return false; } return true; }

#endif

