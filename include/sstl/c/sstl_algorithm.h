/**
 * @file sstl_algorithm.h
 * @brief C99 SSTL public header with static, allocation-free algorithm APIs.
 *
 * The declarations in this file are part of the local SSTL contract. The C
 * algorithm surface is intentionally generated through macros because C99 has
 * no templates: each translation unit chooses the concrete element type and
 * comparator it needs, and the generated functions operate directly on caller
 * owned pointer ranges.
 */
#ifndef SSTL_C_ALGORITHM_H
/** @def SSTL_C_ALGORITHM_H
 * @brief Include guard for sstl_algorithm.h.
 */
#define SSTL_C_ALGORITHM_H

#include "sstl_config.h"

/**
 * @def SSTL_ALGORITHM_DECLARE
 * @brief Declare a typed C99 algorithm family for pointer ranges.
 * @param NAME Public function prefix to generate.
 * @param T Element type stored in the pointer ranges.
 *
 * Generated functions use half-open ranges `[first, last)`, return pointers in
 * the same spirit as C++ iterators, and never allocate memory. Predicates take
 * `const T*`, unary transforms return `T`, and comparison-based algorithms use
 * the comparator passed to `SSTL_ALGORITHM_DEFINE`.
 */
#define SSTL_ALGORITHM_DECLARE(NAME, T) \
  /** \
   * @brief Run the generated find operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_find(T* first, T* last, const T* value); \
  /** \
   * @brief Run the generated find if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_find_if(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated find if not operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_find_if_not(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated all of operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_all_of(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated any of operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_any_of(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated none of operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_none_of(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated count operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_count(T* first, T* last, const T* value); \
  /** \
   * @brief Run the generated count if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_count_if(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated equal operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param other Other object participating in the operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_equal(T* first, T* last, T* other); \
  /** \
   * @brief Run the generated mismatch first operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param other Other object participating in the operation. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_mismatch_first(T* first, T* last, T* other); \
  /** \
   * @brief Run the generated mismatch second operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param other Other object participating in the operation. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_mismatch_second(T* first, T* last, T* other); \
  /** \
   * @brief Run the generated search operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param needle_first Start of the search subsequence. \
   * @param needle_last End of the search subsequence. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_search(T* first, T* last, T* needle_first, T* needle_last); \
  /** \
   * @brief Run the generated lexicographical compare operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_lexicographical_compare(T* a_first, T* a_last, T* b_first, T* b_last); \
  /** \
   * @brief Run the generated min element operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_min_element(T* first, T* last); \
  /** \
   * @brief Run the generated max element operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_max_element(T* first, T* last); \
  /** \
   * @brief Run the generated minmax first operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_minmax_first(T* first, T* last); \
  /** \
   * @brief Run the generated minmax second operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_minmax_second(T* first, T* last); \
  /** \
   * @brief Run the generated copy operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_copy(T* first, T* last, T* out); \
  /** \
   * @brief Run the generated copy n operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param n Requested count or size. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_copy_n(T* first, size_t n, T* out); \
  /** \
   * @brief Run the generated copy if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_copy_if(T* first, T* last, T* out, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated copy backward operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out_last One-past-end destination used by backward-copy operations. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_copy_backward(T* first, T* last, T* out_last); \
  /** \
   * @brief Run the generated fill operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_fill(T* first, T* last, T value); \
  /** \
   * @brief Run the generated fill n operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param n Requested count or size. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_fill_n(T* first, size_t n, T value); \
  /** \
   * @brief Run the generated transform operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @param op Transform operation. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_transform(T* first, T* last, T* out, sstl_transform_fn op); \
  /** \
   * @brief Run the generated replace operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param old_value Value to replace. \
   * @param new_value Replacement value. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_replace(T* first, T* last, const T* old_value, T new_value); \
  /** \
   * @brief Run the generated replace if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @param new_value Replacement value. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_replace_if(T* first, T* last, sstl_pred_fn pred, T new_value); \
  /** \
   * @brief Run the generated remove operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_remove(T* first, T* last, const T* value); \
  /** \
   * @brief Run the generated remove if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_remove_if(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated iter swap operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_iter_swap(T* a, T* b); \
  /** \
   * @brief Run the generated reverse operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_reverse(T* first, T* last); \
  /** \
   * @brief Run the generated rotate operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param middle Rotation or sorted-prefix split point. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_rotate(T* first, T* middle, T* last); \
  /** \
   * @brief Run the generated swap ranges operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param other Other object participating in the operation. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_swap_ranges(T* first, T* last, T* other); \
  /** \
   * @brief Run the generated unique operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_unique(T* first, T* last); \
  /** \
   * @brief Run the generated sort operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_sort(T* first, T* last); \
  /** \
   * @brief Run the generated stable sort operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_stable_sort(T* first, T* last); \
  /** \
   * @brief Run the generated partial sort operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param middle Rotation or sorted-prefix split point. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_partial_sort(T* first, T* middle, T* last); \
  /** \
   * @brief Run the generated nth element operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param nth Position that should contain the nth ordered element. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_nth_element(T* first, T* nth, T* last); \
  /** \
   * @brief Run the generated is sorted operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_is_sorted(T* first, T* last); \
  /** \
   * @brief Run the generated is sorted until operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_is_sorted_until(T* first, T* last); \
  /** \
   * @brief Run the generated make heap operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_make_heap(T* first, T* last); \
  /** \
   * @brief Run the generated push heap operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_push_heap(T* first, T* last); \
  /** \
   * @brief Run the generated pop heap operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_pop_heap(T* first, T* last); \
  /** \
   * @brief Run the generated sort heap operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_sort_heap(T* first, T* last); \
  /** \
   * @brief Run the generated partition operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_partition(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated stable partition operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_stable_partition(T* first, T* last, sstl_pred_fn pred); \
  /** \
   * @brief Run the generated merge operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_merge(T* a_first, T* a_last, T* b_first, T* b_last, T* out); \
  /** \
   * @brief Run the generated includes operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param subset_first Start of the sorted subset candidate. \
   * @param subset_last End of the sorted subset candidate. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_includes(T* first, T* last, T* subset_first, T* subset_last); \
  /** \
   * @brief Run the generated set union operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_set_union(T* a_first, T* a_last, T* b_first, T* b_last, T* out); \
  /** \
   * @brief Run the generated set intersection operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_set_intersection(T* a_first, T* a_last, T* b_first, T* b_last, T* out); \
  /** \
   * @brief Run the generated set difference operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_set_difference(T* a_first, T* a_last, T* b_first, T* b_last, T* out); \
  /** \
   * @brief Run the generated set symmetric difference operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_set_symmetric_difference(T* a_first, T* a_last, T* b_first, T* b_last, T* out); \
  /** \
   * @brief Run the generated lower bound operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_lower_bound(T* first, T* last, const T* value); \
  /** \
   * @brief Run the generated upper bound operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_upper_bound(T* first, T* last, const T* value); \
  /** \
   * @brief Run the generated equal range first operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_equal_range_first(T* first, T* last, const T* value); \
  /** \
   * @brief Run the generated equal range second operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_equal_range_second(T* first, T* last, const T* value); \
  /** \
   * @brief Run the generated binary search operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_binary_search(T* first, T* last, const T* value);

/**
 * @def SSTL_ALGORITHM_DEFINE
 * @brief Define a typed C99 algorithm family declared by `SSTL_ALGORITHM_DECLARE`.
 * @param NAME Public function prefix used by the declaration macro.
 * @param T Element type stored in the pointer ranges.
 * @param CMP Three-way comparator with signature `int cmp(const T*, const T*)`.
 *
 * All generated algorithms are header-local and allocation-free. Sorting and
 * partitioning use only swaps and automatic temporaries of type `T`; output
 * algorithms require the caller to provide enough destination storage.
 */
#define SSTL_ALGORITHM_DEFINE(NAME, T, CMP) \
  /** \
   * @brief Internal generated comparison helper for equality checks. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_algo_equal_value(const T* a, const T* b) { return (CMP)((const void*)a, (const void*)b) == 0; } \
  /** \
   * @brief Internal generated comparison helper for strict weak ordering checks. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_algo_less_value(const T* a, const T* b) { return (CMP)((const void*)a, (const void*)b) < 0; } \
  /** \
   * @brief Run the generated iter swap operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_iter_swap(T* a, T* b) { T tmp = *a; *a = *b; *b = tmp; } \
  /** \
   * @brief Internal generated heap sift-down helper used by heap algorithms. \
   * @param first Start of the half-open range. \
   * @param start Caller-supplied argument used by this operation. \
   * @param count Requested element or character count. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_heap_sift_down(T* first, size_t start, size_t count) { size_t root = start; while ((root * 2u + 1u) < count) { size_t child = root * 2u + 1u; size_t swap_index = root; if (NAME##_algo_less_value(&first[swap_index], &first[child])) swap_index = child; if ((child + 1u) < count && NAME##_algo_less_value(&first[swap_index], &first[child + 1u])) swap_index = child + 1u; if (swap_index == root) return; NAME##_iter_swap(&first[root], &first[swap_index]); root = swap_index; } } \
  /** \
   * @brief Internal generated depth-budget helper for introsort. \
   * @param count Requested element or character count. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_sort_depth(size_t count) { size_t depth = 0u; while (count > 1u) { count /= 2u; ++depth; } return depth * 2u; } \
  /** \
   * @brief Internal generated heap-sort helper used by introsort fallback and bounded ordering. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_heap_sort_range(T* first, T* last) { size_t count = (size_t)(last - first); if (count < 2u) return; { size_t start = count / 2u; while (start != 0u) { --start; NAME##_heap_sift_down(first, start, count); } while (count > 1u) { NAME##_iter_swap(first, first + count - 1u); --count; NAME##_heap_sift_down(first, 0u, count); } } } \
  /** \
   * @brief Internal generated introsort helper used by sort. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param depth Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_intro_sort(T* first, T* last, size_t depth) { while ((last - first) > 1) { size_t n = (size_t)(last - first); size_t i = 0u; size_t j = n - 1u; T pivot; if (depth == 0u) { NAME##_heap_sort_range(first, last); return; } --depth; pivot = first[n / 2u]; while (i <= j) { while (NAME##_algo_less_value(&first[i], &pivot)) ++i; while (NAME##_algo_less_value(&pivot, &first[j])) { if (j == 0u) break; --j; } if (i <= j) { NAME##_iter_swap(&first[i], &first[j]); ++i; if (j == 0u) break; --j; } } if ((j + 1u) < (n - i)) { if ((j + 1u) > 1u) NAME##_intro_sort(first, first + j + 1u, depth); first += i; } else { if ((n - i) > 1u) NAME##_intro_sort(first + i, last, depth); last = first + j + 1u; } } } \
  /** \
   * @brief Internal generated introsort entry point used by sort. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_quick_sort(T* first, T* last) { NAME##_intro_sort(first, last, NAME##_sort_depth((size_t)(last - first))); } \
  /** \
   * @brief Run the generated find operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_find(T* first, T* last, const T* value) { while (first != last) { if (NAME##_algo_equal_value(first, value)) return first; ++first; } return last; } \
  /** \
   * @brief Run the generated find if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_find_if(T* first, T* last, sstl_pred_fn pred) { while (first != last) { if (pred(first)) return first; ++first; } return last; } \
  /** \
   * @brief Run the generated find if not operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_find_if_not(T* first, T* last, sstl_pred_fn pred) { while (first != last) { if (!pred(first)) return first; ++first; } return last; } \
  /** \
   * @brief Run the generated all of operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_all_of(T* first, T* last, sstl_pred_fn pred) { while (first != last) { if (!pred(first)) return false; ++first; } return true; } \
  /** \
   * @brief Run the generated any of operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_any_of(T* first, T* last, sstl_pred_fn pred) { return NAME##_find_if(first, last, pred) != last; } \
  /** \
   * @brief Run the generated none of operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_none_of(T* first, T* last, sstl_pred_fn pred) { return NAME##_find_if(first, last, pred) == last; } \
  /** \
   * @brief Run the generated count operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_count(T* first, T* last, const T* value) { size_t n = 0u; while (first != last) { if (NAME##_algo_equal_value(first, value)) ++n; ++first; } return n; } \
  /** \
   * @brief Run the generated count if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_count_if(T* first, T* last, sstl_pred_fn pred) { size_t n = 0u; while (first != last) { if (pred(first)) ++n; ++first; } return n; } \
  /** \
   * @brief Run the generated equal operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param other Other object participating in the operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_equal(T* first, T* last, T* other) { while (first != last) { if (!NAME##_algo_equal_value(first, other)) return false; ++first; ++other; } return true; } \
  /** \
   * @brief Run the generated mismatch first operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param other Other object participating in the operation. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_mismatch_first(T* first, T* last, T* other) { while (first != last && NAME##_algo_equal_value(first, other)) { ++first; ++other; } return first; } \
  /** \
   * @brief Run the generated mismatch second operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param other Other object participating in the operation. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_mismatch_second(T* first, T* last, T* other) { while (first != last && NAME##_algo_equal_value(first, other)) { ++first; ++other; } return other; } \
  /** \
   * @brief Run the generated search operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param needle_first Start of the search subsequence. \
   * @param needle_last End of the search subsequence. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_search(T* first, T* last, T* needle_first, T* needle_last) { if (needle_first == needle_last) return first; while (first != last) { T* h = first; T* n = needle_first; while (h != last && n != needle_last && NAME##_algo_equal_value(h, n)) { ++h; ++n; } if (n == needle_last) return first; if (h == last) return last; ++first; } return last; } \
  /** \
   * @brief Run the generated lexicographical compare operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_lexicographical_compare(T* a_first, T* a_last, T* b_first, T* b_last) { while (a_first != a_last && b_first != b_last) { if (NAME##_algo_less_value(a_first, b_first)) return true; if (NAME##_algo_less_value(b_first, a_first)) return false; ++a_first; ++b_first; } return a_first == a_last && b_first != b_last; } \
  /** \
   * @brief Run the generated min element operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_min_element(T* first, T* last) { T* best = first; if (first == last) return last; ++first; while (first != last) { if (NAME##_algo_less_value(first, best)) best = first; ++first; } return best; } \
  /** \
   * @brief Run the generated max element operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_max_element(T* first, T* last) { T* best = first; if (first == last) return last; ++first; while (first != last) { if (NAME##_algo_less_value(best, first)) best = first; ++first; } return best; } \
  /** \
   * @brief Run the generated minmax first operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_minmax_first(T* first, T* last) { return NAME##_min_element(first, last); } \
  /** \
   * @brief Run the generated minmax second operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_minmax_second(T* first, T* last) { return NAME##_max_element(first, last); } \
  /** \
   * @brief Run the generated copy operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_copy(T* first, T* last, T* out) { while (first != last) *out++ = *first++; return out; } \
  /** \
   * @brief Run the generated copy n operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param n Requested count or size. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_copy_n(T* first, size_t n, T* out) { while (n--) *out++ = *first++; return out; } \
  /** \
   * @brief Run the generated copy if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_copy_if(T* first, T* last, T* out, sstl_pred_fn pred) { while (first != last) { if (pred(first)) *out++ = *first; ++first; } return out; } \
  /** \
   * @brief Run the generated copy backward operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out_last One-past-end destination used by backward-copy operations. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_copy_backward(T* first, T* last, T* out_last) { while (first != last) *--out_last = *--last; return out_last; } \
  /** \
   * @brief Run the generated fill operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_fill(T* first, T* last, T value) { while (first != last) *first++ = value; } \
  /** \
   * @brief Run the generated fill n operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param n Requested count or size. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_fill_n(T* first, size_t n, T value) { while (n--) *first++ = value; return first; } \
  /** \
   * @brief Run the generated transform operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @param op Transform operation. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_transform(T* first, T* last, T* out, sstl_transform_fn op) { while (first != last) { op((void*)out, (const void*)first); ++out; ++first; } return out; } \
  /** \
   * @brief Run the generated replace operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param old_value Value to replace. \
   * @param new_value Replacement value. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_replace(T* first, T* last, const T* old_value, T new_value) { while (first != last) { if (NAME##_algo_equal_value(first, old_value)) *first = new_value; ++first; } } \
  /** \
   * @brief Run the generated replace if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @param new_value Replacement value. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_replace_if(T* first, T* last, sstl_pred_fn pred, T new_value) { while (first != last) { if (pred(first)) *first = new_value; ++first; } } \
  /** \
   * @brief Run the generated remove operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_remove(T* first, T* last, const T* value) { T* out = first; while (first != last) { if (!NAME##_algo_equal_value(first, value)) *out++ = *first; ++first; } return out; } \
  /** \
   * @brief Run the generated remove if operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_remove_if(T* first, T* last, sstl_pred_fn pred) { T* out = first; while (first != last) { if (!pred(first)) *out++ = *first; ++first; } return out; } \
  /** \
   * @brief Run the generated reverse operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_reverse(T* first, T* last) { while (first != last) { --last; if (first == last) break; NAME##_iter_swap(first, last); ++first; } } \
  /** \
   * @brief Run the generated rotate operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param middle Rotation or sorted-prefix split point. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_rotate(T* first, T* middle, T* last) { size_t shift = (size_t)(middle - first); size_t len = (size_t)(last - first); if (first == middle) return last; if (middle == last) return first; NAME##_reverse(first, middle); NAME##_reverse(middle, last); NAME##_reverse(first, last); return first + (len - shift); } \
  /** \
   * @brief Run the generated swap ranges operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param other Other object participating in the operation. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_swap_ranges(T* first, T* last, T* other) { while (first != last) { NAME##_iter_swap(first, other); ++first; ++other; } return other; } \
  /** \
   * @brief Run the generated unique operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_unique(T* first, T* last) { T* out = first; if (first == last) return last; while (++first != last) { if (!NAME##_algo_equal_value(out, first)) { ++out; if (out != first) *out = *first; } } return out + 1; } \
  /** \
   * @brief Run the generated sort operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_sort(T* first, T* last) { NAME##_quick_sort(first, last); } \
  /** \
   * @brief Return the first merge position whose value is not less than `value`. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return The first merge position whose value is not less than `value`. \
   */ \
  SSTL_C_INLINE T* NAME##_stable_lower_bound(T* first, T* last, const T* value) { size_t count = (size_t)(last - first); while (count > 0u) { size_t step = count / 2u; T* it = first + step; if (NAME##_algo_less_value(it, value)) { first = it + 1; count -= step + 1u; } else { count = step; } } return first; } \
  /** \
   * @brief Return the first merge position whose value is greater than `value`. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return The first merge position whose value is greater than `value`. \
   */ \
  SSTL_C_INLINE T* NAME##_stable_upper_bound(T* first, T* last, const T* value) { size_t count = (size_t)(last - first); while (count > 0u) { size_t step = count / 2u; T* it = first + step; if (!NAME##_algo_less_value(value, it)) { first = it + 1; count -= step + 1u; } else { count = step; } } return first; } \
  /** \
   * @brief Merge two adjacent sorted ranges in place using rotations only. \
   * @param first Start of the half-open range. \
   * @param middle Rotation or sorted-prefix split point. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_inplace_merge(T* first, T* middle, T* last) { T* first_cut; T* second_cut; T* new_middle; if (first == middle || middle == last) return; if (last - first == 2) { if (NAME##_algo_less_value(middle, first)) NAME##_iter_swap(first, middle); return; } if ((size_t)(middle - first) > (size_t)(last - middle)) { first_cut = first + (middle - first) / 2; second_cut = NAME##_stable_lower_bound(middle, last, first_cut); } else { second_cut = middle + (last - middle) / 2; first_cut = NAME##_stable_upper_bound(first, middle, second_cut); } new_middle = NAME##_rotate(first_cut, middle, second_cut); NAME##_inplace_merge(first, first_cut, new_middle); NAME##_inplace_merge(new_middle, second_cut, last); } \
  /** \
   * @brief Recursively stable-sort a pointer range with O(log n) stack and no heap. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_stable_sort_recursive(T* first, T* last) { size_t len = (size_t)(last - first); T* middle; if (len < 2u) return; middle = first + len / 2u; NAME##_stable_sort_recursive(first, middle); NAME##_stable_sort_recursive(middle, last); NAME##_inplace_merge(first, middle, last); } \
  /** \
   * @brief Run the generated stable sort operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_stable_sort(T* first, T* last) { NAME##_stable_sort_recursive(first, last); } \
  /** \
   * @brief Run the generated partial sort operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param middle Rotation or sorted-prefix split point. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_partial_sort(T* first, T* middle, T* last) { size_t count = (size_t)(middle - first); T* it; if (count == 0u) return; if (count > 1u) { size_t start = count / 2u; while (start != 0u) { --start; NAME##_heap_sift_down(first, start, count); } } for (it = middle; it != last; ++it) { if (NAME##_algo_less_value(it, first)) { NAME##_iter_swap(first, it); NAME##_heap_sift_down(first, 0u, count); } } NAME##_sort_heap(first, middle); } \
  /** \
   * @brief Run the generated nth element operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param nth Position that should contain the nth ordered element. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_nth_element(T* first, T* nth, T* last) { size_t nth_index; size_t lo; size_t hi; if (nth == last || first == last) return; nth_index = (size_t)(nth - first); lo = 0u; hi = (size_t)(last - first) - 1u; while (lo < hi) { size_t i = lo; size_t j = hi; T pivot = first[lo + ((hi - lo) / 2u)]; while (i <= j) { while (NAME##_algo_less_value(&first[i], &pivot)) ++i; while (NAME##_algo_less_value(&pivot, &first[j])) { if (j == 0u) break; --j; } if (i <= j) { NAME##_iter_swap(&first[i], &first[j]); ++i; if (j == 0u) break; --j; } } if (nth_index <= j) hi = j; else if (nth_index >= i) lo = i; else return; } } \
  /** \
   * @brief Run the generated is sorted until operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_is_sorted_until(T* first, T* last) { if (first == last) return last; while ((first + 1) != last) { if (NAME##_algo_less_value(first + 1, first)) return first + 1; ++first; } return last; } \
  /** \
   * @brief Run the generated is sorted operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_is_sorted(T* first, T* last) { return NAME##_is_sorted_until(first, last) == last; } \
  /** \
   * @brief Run the generated make heap operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_make_heap(T* first, T* last) { size_t count = (size_t)(last - first); if (count < 2u) return; { size_t start = (count - 2u) / 2u + 1u; while (start != 0u) { --start; NAME##_heap_sift_down(first, start, count); } } } \
  /** \
   * @brief Run the generated push heap operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_push_heap(T* first, T* last) { size_t count = (size_t)(last - first); if (count < 2u) return; { size_t child = count - 1u; while (child > 0u) { size_t parent = (child - 1u) / 2u; if (!NAME##_algo_less_value(&first[parent], &first[child])) break; NAME##_iter_swap(&first[parent], &first[child]); child = parent; } } } \
  /** \
   * @brief Run the generated pop heap operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_pop_heap(T* first, T* last) { size_t count = (size_t)(last - first); if (count < 2u) return; NAME##_iter_swap(first, last - 1); NAME##_heap_sift_down(first, 0u, count - 1u); } \
  /** \
   * @brief Run the generated sort heap operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_sort_heap(T* first, T* last) { while ((last - first) > 1) { NAME##_pop_heap(first, last); --last; } } \
  /** \
   * @brief Run the generated partition operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_partition(T* first, T* last, sstl_pred_fn pred) { for (;;) { while (first != last && pred(first)) ++first; if (first == last) return first; do { --last; if (first == last) return first; } while (!pred(last)); NAME##_iter_swap(first, last); ++first; } } \
  /** \
   * @brief Run the generated stable partition operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param pred Predicate used to test elements. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_stable_partition(T* first, T* last, sstl_pred_fn pred) { T* first_false = first; while (first_false != last && pred(first_false)) ++first_false; { T* scan = first_false; while (scan != last) { if (pred(scan)) { T value = *scan; T* move = scan; while (move != first_false) { *move = *(move - 1); --move; } *first_false = value; ++first_false; } ++scan; } } return first_false; } \
  /** \
   * @brief Run the generated merge operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_merge(T* a_first, T* a_last, T* b_first, T* b_last, T* out) { while (a_first != a_last && b_first != b_last) { if (NAME##_algo_less_value(b_first, a_first)) *out++ = *b_first++; else *out++ = *a_first++; } out = NAME##_copy(a_first, a_last, out); return NAME##_copy(b_first, b_last, out); } \
  /** \
   * @brief Run the generated lower bound operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_lower_bound(T* first, T* last, const T* value) { size_t count = (size_t)(last - first); while (count > 0u) { size_t step = count / 2u; T* it = first + step; if (NAME##_algo_less_value(it, value)) { first = it + 1; count -= step + 1u; } else { count = step; } } return first; } \
  /** \
   * @brief Run the generated upper bound operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_upper_bound(T* first, T* last, const T* value) { size_t count = (size_t)(last - first); while (count > 0u) { size_t step = count / 2u; T* it = first + step; if (!NAME##_algo_less_value(value, it)) { first = it + 1; count -= step + 1u; } else { count = step; } } return first; } \
  /** \
   * @brief Run the generated equal range first operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_equal_range_first(T* first, T* last, const T* value) { return NAME##_lower_bound(first, last, value); } \
  /** \
   * @brief Run the generated equal range second operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_equal_range_second(T* first, T* last, const T* value) { return NAME##_upper_bound(first, last, value); } \
  /** \
   * @brief Run the generated binary search operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_binary_search(T* first, T* last, const T* value) { T* it = NAME##_lower_bound(first, last, value); return it != last && NAME##_algo_equal_value(it, value); } \
  /** \
   * @brief Run the generated includes operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param subset_first Start of the sorted subset candidate. \
   * @param subset_last End of the sorted subset candidate. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_includes(T* first, T* last, T* subset_first, T* subset_last) { while (subset_first != subset_last) { first = NAME##_lower_bound(first, last, subset_first); if (first == last || !NAME##_algo_equal_value(first, subset_first)) return false; ++first; ++subset_first; } return true; } \
  /** \
   * @brief Run the generated set union operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_set_union(T* a_first, T* a_last, T* b_first, T* b_last, T* out) { while (a_first != a_last && b_first != b_last) { if (NAME##_algo_less_value(a_first, b_first)) *out++ = *a_first++; else if (NAME##_algo_less_value(b_first, a_first)) *out++ = *b_first++; else { *out++ = *a_first++; ++b_first; } } out = NAME##_copy(a_first, a_last, out); return NAME##_copy(b_first, b_last, out); } \
  /** \
   * @brief Run the generated set intersection operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_set_intersection(T* a_first, T* a_last, T* b_first, T* b_last, T* out) { while (a_first != a_last && b_first != b_last) { if (NAME##_algo_less_value(a_first, b_first)) ++a_first; else if (NAME##_algo_less_value(b_first, a_first)) ++b_first; else { *out++ = *a_first++; ++b_first; } } return out; } \
  /** \
   * @brief Run the generated set difference operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_set_difference(T* a_first, T* a_last, T* b_first, T* b_last, T* out) { while (a_first != a_last && b_first != b_last) { if (NAME##_algo_less_value(a_first, b_first)) *out++ = *a_first++; else if (NAME##_algo_less_value(b_first, a_first)) ++b_first; else { ++a_first; ++b_first; } } return NAME##_copy(a_first, a_last, out); } \
  /** \
   * @brief Run the generated set symmetric difference operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param b_last End of the second range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_set_symmetric_difference(T* a_first, T* a_last, T* b_first, T* b_last, T* out) { while (a_first != a_last && b_first != b_last) { if (NAME##_algo_less_value(a_first, b_first)) *out++ = *a_first++; else if (NAME##_algo_less_value(b_first, a_first)) *out++ = *b_first++; else { ++a_first; ++b_first; } } out = NAME##_copy(a_first, a_last, out); return NAME##_copy(b_first, b_last, out); }

/**
 * @def SSTL_NUMERIC_ALGORITHM_DECLARE
 * @brief Declare typed numeric algorithms for arithmetic-like element types.
 * @param NAME Public function prefix to generate.
 * @param T Arithmetic-like element type.
 *
 * These operations use the C operators `+`, `*`, `-`, and `++` directly, so
 * they are intentionally separated from `SSTL_ALGORITHM_DECLARE` to keep the
 * comparison/movement algorithms usable for non-arithmetic POD structs.
 */
#define SSTL_NUMERIC_ALGORITHM_DECLARE(NAME, T) \
  /** \
   * @brief Run the generated accumulate operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param init Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE T NAME##_accumulate(T* first, T* last, T init); \
  /** \
   * @brief Run the generated inner product operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param init Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE T NAME##_inner_product(T* a_first, T* a_last, T* b_first, T init); \
  /** \
   * @brief Run the generated partial sum operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_partial_sum(T* first, T* last, T* out); \
  /** \
   * @brief Run the generated adjacent difference operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_adjacent_difference(T* first, T* last, T* out); \
  /** \
   * @brief Run the generated iota operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_iota(T* first, T* last, T value);

/**
 * @def SSTL_NUMERIC_ALGORITHM_DEFINE
 * @brief Define typed numeric algorithms declared by `SSTL_NUMERIC_ALGORITHM_DECLARE`.
 * @param NAME Public function prefix used by the declaration macro.
 * @param T Arithmetic-like element type.
 */
#define SSTL_NUMERIC_ALGORITHM_DEFINE(NAME, T) \
  /** \
   * @brief Run the generated accumulate operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param init Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE T NAME##_accumulate(T* first, T* last, T init) { while (first != last) init = init + *first++; return init; } \
  /** \
   * @brief Run the generated inner product operation for this typed SSTL family. \
   * @param a_first Start of the first range. \
   * @param a_last End of the first range. \
   * @param b_first Start of the second range. \
   * @param init Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE T NAME##_inner_product(T* a_first, T* a_last, T* b_first, T init) { while (a_first != a_last) { init = init + (*a_first * *b_first); ++a_first; ++b_first; } return init; } \
  /** \
   * @brief Run the generated partial sum operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_partial_sum(T* first, T* last, T* out) { if (first == last) return out; { T sum = *first; *out++ = sum; ++first; while (first != last) { sum = sum + *first++; *out++ = sum; } } return out; } \
  /** \
   * @brief Run the generated adjacent difference operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param out Caller-provided destination for produced values. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE T* NAME##_adjacent_difference(T* first, T* last, T* out) { if (first == last) return out; { T prev = *first; *out++ = prev; ++first; while (first != last) { T cur = *first++; *out++ = cur - prev; prev = cur; } } return out; } \
  /** \
   * @brief Run the generated iota operation for this typed SSTL family. \
   * @param first Start of the half-open range. \
   * @param last One-past-end of the half-open range. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_iota(T* first, T* last, T value) { while (first != last) { *first++ = value; ++value; } }

#endif

