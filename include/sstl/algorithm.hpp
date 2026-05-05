/**
 * @file algorithm.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_ALGORITHM_HPP
/** @def SSTL_ALGORITHM_HPP
 * @brief Include guard for algorithm.hpp.
 */
#define SSTL_ALGORITHM_HPP

#include "numeric.hpp"
#include "utility.hpp"

namespace sstl {

/*
  SSTL-owned algorithm subset.

  These implementations are intentionally simple and allocation-free.  They do
  not delegate to <algorithm> or <numeric>; tests may use std as an oracle, but
  the library itself remains suitable for embedded/freestanding builds.
*/

/**
 * @brief Forward declaration for sorted-range lower-bound lookup used by later algorithms.
 *
 * The definition below carries the full parameter and return documentation.
 */
template <class It, class T>
It lower_bound(It first, It last, const T& value);
/**
 * @brief Forward declaration for comparator-based sorted-range lower-bound lookup.
 *
 * The definition below carries the full parameter and return documentation.
 */
template <class It, class T, class Compare>
It lower_bound(It first, It last, const T& value, Compare comp);

/**
 * @brief Return the first iterator whose element compares equal to value, or last when absent.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return The first iterator whose element compares equal to value, or last when absent.
 */
template <class It, class T>
It find(It first, It last, const T& value) {
  for (; first != last; ++first) if (*first == value) return first;
  return last;
}

/**
 * @brief Return the first iterator whose element satisfies the predicate, or last when none do.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return The first iterator whose element satisfies the predicate, or last when none do.
 */
template <class It, class Pred>
It find_if(It first, It last, Pred pred) {
  for (; first != last; ++first) if (pred(*first)) return first;
  return last;
}

/**
 * @brief Return the first iterator whose element does not satisfy the predicate, or last when none do.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return The first iterator whose element does not satisfy the predicate, or last when none do.
 */
template <class It, class Pred>
It find_if_not(It first, It last, Pred pred) {
  for (; first != last; ++first) if (!pred(*first)) return first;
  return last;
}

/**
 * @brief Return true when every element in the range satisfies the predicate.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return True when every element in the range satisfies the predicate.
 */
template <class It, class Pred>
bool all_of(It first, It last, Pred pred) {
  for (; first != last; ++first) if (!pred(*first)) return false;
  return true;
}

/**
 * @brief Return true when at least one element in the range satisfies the predicate.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return True when at least one element in the range satisfies the predicate.
 */
template <class It, class Pred>
bool any_of(It first, It last, Pred pred) {
  for (; first != last; ++first) if (pred(*first)) return true;
  return false;
}

/**
 * @brief Return true when no element in the range satisfies the predicate.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return True when no element in the range satisfies the predicate.
 */
template <class It, class Pred>
bool none_of(It first, It last, Pred pred) { return !any_of(first, last, pred); }

/**
 * @brief Count elements in the range that compare equal to value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return Result described by the function brief.
 */
template <class It, class T>
unsigned count(It first, It last, const T& value) {
  unsigned n = 0;
  for (; first != last; ++first) if (*first == value) ++n;
  return n;
}

/**
 * @brief Count elements in the range that satisfy the predicate.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return Result described by the function brief.
 */
template <class It, class Pred>
unsigned count_if(It first, It last, Pred pred) {
  unsigned n = 0;
  for (; first != last; ++first) if (pred(*first)) ++n;
  return n;
}

/**
 * @brief Return true when both ranges contain equal elements in order.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param other Other object participating in the operation.
 * @return True when both ranges contain equal elements in order.
 */
template <class In1, class In2>
bool equal(In1 first, In1 last, In2 other) {
  for (; first != last; ++first, ++other) if (!(*first == *other)) return false;
  return true;
}

/**
 * @brief Return true when both ranges satisfy a caller-provided binary equality predicate.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param other Other object participating in the operation.
 * @param pred Predicate used to test elements.
 * @return True when both ranges satisfy a caller-provided binary equality predicate.
 */
template <class In1, class In2, class Pred>
bool equal(In1 first, In1 last, In2 other, Pred pred) {
  for (; first != last; ++first, ++other) if (!pred(*first, *other)) return false;
  return true;
}

/**
 * @brief Return the first mismatching iterator pair, or the two advanced end positions.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param other Other object participating in the operation.
 * @return The first mismatching iterator pair, or the two advanced end positions.
 */
template <class In1, class In2>
pair<In1, In2> mismatch(In1 first, In1 last, In2 other) {
  for (; first != last; ++first, ++other) if (!(*first == *other)) break;
  return make_pair(first, other);
}

/**
 * @brief Return the first iterator pair that fails a caller-provided binary equality predicate.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param other Other object participating in the operation.
 * @param pred Predicate used to test elements.
 * @return The first iterator pair that fails a caller-provided binary equality predicate.
 */
template <class In1, class In2, class Pred>
pair<In1, In2> mismatch(In1 first, In1 last, In2 other, Pred pred) {
  for (; first != last; ++first, ++other) if (!pred(*first, *other)) break;
  return make_pair(first, other);
}

/**
 * @brief Return the first occurrence of the needle range inside the haystack range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param needle_first Start of the search subsequence.
 * @param needle_last End of the search subsequence.
 * @return The first occurrence of the needle range inside the haystack range.
 */
template <class It1, class It2>
It1 search(It1 first, It1 last, It2 needle_first, It2 needle_last) {
  if (needle_first == needle_last) return first;
  for (; first != last; ++first) {
    It1 h = first;
    It2 n = needle_first;
    while (h != last && n != needle_last && *h == *n) { ++h; ++n; }
    if (n == needle_last) return first;
    if (h == last) return last;
  }
  return last;
}

/**
 * @brief Lexicographically compare two ranges using operator<.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class It1, class It2>
bool lexicographical_compare(It1 a, It1 ae, It2 b, It2 be) {
  for (; a != ae && b != be; ++a, ++b) {
    if (*a < *b) return true;
    if (*b < *a) return false;
  }
  return a == ae && b != be;
}

/**
 * @brief Lexicographically compare two ranges using a caller-provided strict ordering.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class It1, class It2, class Compare>
bool lexicographical_compare(It1 a, It1 ae, It2 b, It2 be, Compare comp) {
  for (; a != ae && b != be; ++a, ++b) {
    if (comp(*a, *b)) return true;
    if (comp(*b, *a)) return false;
  }
  return a == ae && b != be;
}

/**
 * @brief Return an iterator to the smallest element, or last for an empty range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return An iterator to the smallest element, or last for an empty range.
 */
template <class It>
It min_element(It first, It last) {
  if (first == last) return last;
  It best = first;
  for (++first; first != last; ++first) if (*first < *best) best = first;
  return best;
}

/**
 * @brief Return the smallest element according to a caller-provided strict ordering.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 * @return The smallest element according to a caller-provided strict ordering.
 */
template <class It, class Compare>
It min_element(It first, It last, Compare comp) {
  if (first == last) return last;
  It best = first;
  for (++first; first != last; ++first) if (comp(*first, *best)) best = first;
  return best;
}

/**
 * @brief Return an iterator to the largest element, or last for an empty range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return An iterator to the largest element, or last for an empty range.
 */
template <class It>
It max_element(It first, It last) {
  if (first == last) return last;
  It best = first;
  for (++first; first != last; ++first) if (*best < *first) best = first;
  return best;
}

/**
 * @brief Return the largest element according to a caller-provided strict ordering.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 * @return The largest element according to a caller-provided strict ordering.
 */
template <class It, class Compare>
It max_element(It first, It last, Compare comp) {
  if (first == last) return last;
  It best = first;
  for (++first; first != last; ++first) if (comp(*best, *first)) best = first;
  return best;
}

/**
 * @brief Return the smallest and largest element positions as a pair.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return The smallest and largest element positions as a pair.
 */
template <class It>
pair<It, It> minmax_element(It first, It last) {
  return make_pair(min_element(first, last), max_element(first, last));
}

/**
 * @brief Copy the input range into the output range and return the advanced output iterator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In, class Out>
Out copy(In first, In last, Out out) {
  for (; first != last; ++first, ++out) *out = *first;
  return out;
}

/**
 * @brief Copy exactly n elements into the output range and return the advanced output iterator.
 * @param first Start of the half-open range.
 * @param n Requested count or size.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In, class Size, class Out>
Out copy_n(In first, Size n, Out out) {
  for (Size i = 0; i != n; ++i, ++first, ++out) *out = *first;
  return out;
}

/**
 * @brief Copy only predicate-satisfying elements into the output range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out Caller-provided destination for produced values.
 * @param pred Predicate used to test elements.
 * @return Result described by the function brief.
 */
template <class In, class Out, class Pred>
Out copy_if(In first, In last, Out out, Pred pred) {
  for (; first != last; ++first) if (pred(*first)) *out++ = *first;
  return out;
}

/**
 * @brief Copy a bidirectional range ending at out_last while preserving overlap behavior.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out_last One-past-end destination used by backward-copy operations.
 * @return Result described by the function brief.
 */
template <class Bidir1, class Bidir2>
Bidir2 copy_backward(Bidir1 first, Bidir1 last, Bidir2 out_last) {
  while (first != last) *--out_last = *--last;
  return out_last;
}

/**
 * @brief Assign value to every element in the range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 */
template <class It, class T>
void fill(It first, It last, const T& value) {
  for (; first != last; ++first) *first = value;
}

/**
 * @brief Assign value to n consecutive elements and return the advanced iterator.
 * @param first Start of the half-open range.
 * @param n Requested count or size.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return Result described by the function brief.
 */
template <class It, class Size, class T>
It fill_n(It first, Size n, const T& value) {
  for (Size i = 0; i != n; ++i, ++first) *first = value;
  return first;
}

/**
 * @brief Apply a unary operation to each input element and write the results to output.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out Caller-provided destination for produced values.
 * @param op Transform operation.
 * @return Result described by the function brief.
 */
template <class In, class Out, class Unary>
Out transform(In first, In last, Out out, Unary op) {
  for (; first != last; ++first, ++out) *out = op(*first);
  return out;
}

/**
 * @brief Replace elements equal to old_value with new_value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param old_value Value to replace.
 * @param new_value Replacement value.
 */
template <class It, class T>
void replace(It first, It last, const T& old_value, const T& new_value) {
  for (; first != last; ++first) if (*first == old_value) *first = new_value;
}

/**
 * @brief Replace predicate-satisfying elements with value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 */
template <class It, class Pred, class T>
void replace_if(It first, It last, Pred pred, const T& value) {
  for (; first != last; ++first) if (pred(*first)) *first = value;
}

/**
 * @brief Move non-matching elements forward and return the new logical end.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return Result described by the function brief.
 */
template <class It, class T>
It remove(It first, It last, const T& value) {
  It out = first;
  for (; first != last; ++first) if (!(*first == value)) *out++ = *first;
  return out;
}

/**
 * @brief Move predicate-failing elements forward and return the new logical end.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return Result described by the function brief.
 */
template <class It, class Pred>
It remove_if(It first, It last, Pred pred) {
  It out = first;
  for (; first != last; ++first) if (!pred(*first)) *out++ = *first;
  return out;
}

/**
 * @brief Exchange the elements referred to by two iterators.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 */
template <class It1, class It2>
void iter_swap(It1 a, It2 b) {
  sstl::swap(*a, *b);
}

/**
 * @brief Reverse the order of elements in a bidirectional range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class Bidir>
void reverse(Bidir first, Bidir last) {
  while (first != last) {
    --last;
    if (first == last) break;
    iter_swap(first, last);
    ++first;
  }
}

/**
 * @brief Rotate a range so middle becomes the first element and return the new position of the former first element.
 * @param first Start of the half-open range.
 * @param middle Rotation or sorted-prefix split point.
 * @param last One-past-end of the half-open range.
 * @return Result described by the function brief.
 */
template <class It>
It rotate(It first, It middle, It last) {
  reverse(first, middle);
  reverse(middle, last);
  reverse(first, last);
  It ret = first;
  It m = middle;
  while (m != last) { ++ret; ++m; }
  return ret;
}

/**
 * @brief Swap each element in one range with the corresponding element in another range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class It1, class It2>
It2 swap_ranges(It1 first, It1 last, It2 out) {
  for (; first != last; ++first, ++out) iter_swap(first, out);
  return out;
}

/**
 * @brief Collapse consecutive equal elements and return the new logical end.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return Result described by the function brief.
 */
template <class It>
It unique(It first, It last) {
  if (first == last) return last;
  It out = first;
  It next = first;
  while (++next != last) {
    if (!(*out == *next)) *++out = *next;
  }
  return ++out;
}

/**
 * @brief Collapse consecutive predicate-equivalent elements and return the new logical end.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return Result described by the function brief.
 */
template <class It, class BinaryPred>
It unique(It first, It last, BinaryPred pred) {
  if (first == last) return last;
  It out = first;
  It next = first;
  while (++next != last) {
    if (!pred(*out, *next)) *++out = *next;
  }
  return ++out;
}

/**
 * @brief Return the depth budget used before introsort falls back to heap sort.
 * @param count Requested element or character count.
 * @return The depth budget used before introsort falls back to heap sort.
 */
inline long sort_depth_limit(long count) {
  long depth = 0;
  while (count > 1) {
    count /= 2;
    ++depth;
  }
  return depth * 2;
}

/**
 * @brief Restore max-heap order inside an indexed subrange.
 * @param first Start of the half-open range.
 * @param offset Caller-supplied argument used by this operation.
 * @param count Requested element or character count.
 * @param parent Caller-supplied argument used by this operation.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void heap_sift_down_slice(It first, long offset, long count, long parent, Compare comp) {
  for (;;) {
    const long left = parent * 2 + 1;
    const long right = left + 1;
    long best = parent;
    if (left < count && comp(first[offset + best], first[offset + left])) best = left;
    if (right < count && comp(first[offset + best], first[offset + right])) best = right;
    if (best == parent) break;
    iter_swap(first + offset + parent, first + offset + best);
    parent = best;
  }
}

/**
 * @brief Sort an indexed subrange with heap sort for the introsort fallback.
 * @param first Start of the half-open range.
 * @param offset Caller-supplied argument used by this operation.
 * @param count Requested element or character count.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void heap_sort_slice(It first, long offset, long count, Compare comp) {
  for (long i = count / 2; i != 0; --i) heap_sift_down_slice(first, offset, count, i - 1, comp);
  while (count > 1) {
    iter_swap(first + offset, first + offset + count - 1);
    --count;
    heap_sift_down_slice(first, offset, count, 0, comp);
  }
}

/**
 * @brief Internal introsort helper for random-access iterator and pointer ranges.
 * @param first Start of the half-open range.
 * @param lo Caller-supplied argument used by this operation.
 * @param hi Caller-supplied argument used by this operation.
 * @param depth Caller-supplied argument used by this operation.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Value, class Compare>
void intro_sort_indexed(It first, long lo, long hi, long depth, Compare comp) {
  while (lo < hi) {
    if (depth == 0) {
      heap_sort_slice(first, lo, hi - lo + 1, comp);
      return;
    }
    --depth;
    long i = lo;
    long j = hi;
    Value pivot = first[lo + ((hi - lo) / 2)];
    while (i <= j) {
      while (comp(first[i], pivot)) ++i;
      while (comp(pivot, first[j])) --j;
      if (i <= j) {
        iter_swap(first + i, first + j);
        ++i;
        --j;
      }
    }
    if ((j - lo) < (hi - i)) {
      if (lo < j) intro_sort_indexed<It, Value, Compare>(first, lo, j, depth, comp);
      lo = i;
    } else {
      if (i < hi) intro_sort_indexed<It, Value, Compare>(first, i, hi, depth, comp);
      hi = j;
    }
  }
}

/**
 * @brief Internal introsort entry point for iterator types with nested value_type.
 * @param first Start of the half-open range.
 * @param lo Caller-supplied argument used by this operation.
 * @param hi Caller-supplied argument used by this operation.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void quick_sort_indexed(It first, long lo, long hi, Compare comp) {
  intro_sort_indexed<It, typename It::value_type, Compare>(first, lo, hi, sort_depth_limit(hi - lo + 1), comp);
}

/**
 * @brief Internal introsort entry point specialized for raw pointer ranges.
 * @param first Start of the half-open range.
 * @param lo Caller-supplied argument used by this operation.
 * @param hi Caller-supplied argument used by this operation.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class T, class Compare>
void quick_sort_pointer(T* first, long lo, long hi, Compare comp) {
  intro_sort_indexed<T*, T, Compare>(first, lo, hi, sort_depth_limit(hi - lo + 1), comp);
}

/**
 * @brief Sort a random-access range in place using allocation-free introsort.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void sort(It first, It last, Compare comp) {
  const long count = static_cast<long>(last - first);
  if (count > 1) quick_sort_indexed(first, 0, count - 1, comp);
}

/**
 * @brief Sort the container contents in place without heap allocation.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class It>
void sort(It first, It last) { sort(first, last, less<typename It::value_type>()); }

/**
 * @brief Sort a random-access range in place using allocation-free introsort.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class T, class Compare>
void sort(T* first, T* last, Compare comp) {
  const long count = static_cast<long>(last - first);
  if (count > 1) quick_sort_pointer(first, 0, count - 1, comp);
}

/**
 * @brief Sort the container contents in place without heap allocation.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class T>
void sort(T* first, T* last) { sort(first, last, less<T>()); }

/**
 * @brief Return the first random-access position where `value` could be inserted before equivalent values.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @param comp Strict weak ordering used for comparisons.
 * @return The first random-access position where `value` could be inserted before equivalent values.
 */
template <class It, class T, class Compare>
It stable_lower_bound(It first, It last, const T& value, Compare comp) {
  while (first != last) {
    It mid = first + ((last - first) / 2);
    if (comp(*mid, value)) first = mid + 1;
    else last = mid;
  }
  return first;
}

/**
 * @brief Return the first random-access position where `value` could be inserted after equivalent values.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @param comp Strict weak ordering used for comparisons.
 * @return The first random-access position where `value` could be inserted after equivalent values.
 */
template <class It, class T, class Compare>
It stable_upper_bound(It first, It last, const T& value, Compare comp) {
  while (first != last) {
    It mid = first + ((last - first) / 2);
    if (!comp(value, *mid)) first = mid + 1;
    else last = mid;
  }
  return first;
}

/**
 * @brief Merge two adjacent sorted random-access ranges using recursive rotation and no scratch buffer.
 * @param first Start of the half-open range.
 * @param middle Rotation or sorted-prefix split point.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void stable_inplace_merge(It first, It middle, It last, Compare comp) {
  const long left_count = static_cast<long>(middle - first);
  const long right_count = static_cast<long>(last - middle);
  if (left_count == 0 || right_count == 0) return;
  if (left_count + right_count == 2) {
    if (comp(*middle, *first)) iter_swap(first, middle);
    return;
  }

  It first_cut = first;
  It second_cut = middle;
  if (left_count > right_count) {
    first_cut = first + (left_count / 2);
    second_cut = stable_lower_bound(middle, last, *first_cut, comp);
  } else {
    second_cut = middle + (right_count / 2);
    first_cut = stable_upper_bound(first, middle, *second_cut, comp);
  }

  It new_middle = rotate(first_cut, middle, second_cut);
  stable_inplace_merge(first, first_cut, new_middle, comp);
  stable_inplace_merge(new_middle, second_cut, last, comp);
}

/**
 * @brief Recursively stable-sort a random-access range with in-place merge sort.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void stable_sort_inplace(It first, It last, Compare comp) {
  const long count = static_cast<long>(last - first);
  if (count < 2) return;
  It middle = first + (count / 2);
  stable_sort_inplace(first, middle, comp);
  stable_sort_inplace(middle, last, comp);
  stable_inplace_merge(first, middle, last, comp);
}

/**
 * @brief Stable-sort a random-access range with allocation-free O(n log^2 n) in-place merge sort.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void stable_sort(It first, It last, Compare comp) {
  stable_sort_inplace(first, last, comp);
}
/**
 * @brief Sort the range stably using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class It>
void stable_sort(It first, It last) { stable_sort(first, last, less<typename It::value_type>()); }

/**
 * @brief Stable-sort a pointer range with allocation-free O(n log^2 n) in-place merge sort.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class T, class Compare>
void stable_sort(T* first, T* last, Compare comp) {
  stable_sort_inplace(first, last, comp);
}

/**
 * @brief Sort the range stably using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class T>
void stable_sort(T* first, T* last) { stable_sort(first, last, less<T>()); }

/**
 * @brief Arrange the smallest elements into sorted order in the front random-access subrange.
 * @param first Start of the half-open range.
 * @param middle Rotation or sorted-prefix split point.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void partial_sort(It first, It middle, It last, Compare comp) {
  if (first == middle) return;
  make_heap(first, middle, comp);
  for (It it = middle; it != last; ++it) {
    if (comp(*it, *first)) {
      iter_swap(first, it);
      heap_sift_down(first, static_cast<unsigned>(middle - first), 0u, comp);
    }
  }
  sort_heap(first, middle, comp);
}

/**
 * @brief Arrange the smallest elements into sorted order in the front random-access subrange.
 * @param first Start of the half-open range.
 * @param middle Rotation or sorted-prefix split point.
 * @param last One-past-end of the half-open range.
 */
template <class It>
void partial_sort(It first, It middle, It last) {
  partial_sort(first, middle, last, less<typename It::value_type>());
}

/**
 * @brief Arrange the smallest elements into sorted order in the front pointer subrange.
 * @param first Start of the half-open range.
 * @param middle Rotation or sorted-prefix split point.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class T, class Compare>
void partial_sort(T* first, T* middle, T* last, Compare comp) {
  if (first == middle) return;
  make_heap(first, middle, comp);
  for (T* it = middle; it != last; ++it) {
    if (comp(*it, *first)) {
      iter_swap(first, it);
      heap_sift_down(first, static_cast<unsigned>(middle - first), 0u, comp);
    }
  }
  sort_heap(first, middle, comp);
}

/**
 * @brief Partially sort a pointer range using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param middle Rotation or sorted-prefix split point.
 * @param last One-past-end of the half-open range.
 */
template <class T>
void partial_sort(T* first, T* middle, T* last) { partial_sort(first, middle, last, less<T>()); }

/**
 * @brief Internal random-access selection helper that positions the nth element.
 * @param first Start of the half-open range.
 * @param nth Position that should contain the nth ordered element.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void nth_element_indexed(It first, It nth, It last, Compare comp) {
  long lo = 0;
  long hi = static_cast<long>(last - first) - 1;
  const long nth_index = static_cast<long>(nth - first);
  while (lo < hi) {
    long i = lo;
    long j = hi;
    typename It::value_type pivot = first[lo + ((hi - lo) / 2)];
    while (i <= j) {
      while (comp(first[i], pivot)) ++i;
      while (comp(pivot, first[j])) --j;
      if (i <= j) {
        iter_swap(first + i, first + j);
        ++i;
        --j;
      }
    }
    if (nth_index <= j) hi = j;
    else if (nth_index >= i) lo = i;
    else return;
  }
}

/**
 * @brief Internal pointer selection helper that positions the nth element.
 * @param first Start of the half-open range.
 * @param nth Position that should contain the nth ordered element.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class T, class Compare>
void nth_element_pointer(T* first, T* nth, T* last, Compare comp) {
  long lo = 0;
  long hi = static_cast<long>(last - first) - 1;
  const long nth_index = static_cast<long>(nth - first);
  while (lo < hi) {
    long i = lo;
    long j = hi;
    T pivot = first[lo + ((hi - lo) / 2)];
    while (i <= j) {
      while (comp(first[i], pivot)) ++i;
      while (comp(pivot, first[j])) --j;
      if (i <= j) {
        iter_swap(first + i, first + j);
        ++i;
        --j;
      }
    }
    if (nth_index <= j) hi = j;
    else if (nth_index >= i) lo = i;
    else return;
  }
}

/**
 * @brief Partition a random-access range so nth contains the element that would appear there after sorting.
 * @param first Start of the half-open range.
 * @param nth Position that should contain the nth ordered element.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void nth_element(It first, It nth, It last, Compare comp) {
  nth_element_indexed(first, nth, last, comp);
}

/**
 * @brief Partition a random-access range so nth contains the element that would appear there after sorting.
 * @param first Start of the half-open range.
 * @param nth Position that should contain the nth ordered element.
 * @param last One-past-end of the half-open range.
 */
template <class It>
void nth_element(It first, It nth, It last) {
  nth_element(first, nth, last, less<typename It::value_type>());
}

/**
 * @brief Partition a pointer range so nth contains the element that would appear there after sorting.
 * @param first Start of the half-open range.
 * @param nth Position that should contain the nth ordered element.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class T, class Compare>
void nth_element(T* first, T* nth, T* last, Compare comp) {
  nth_element_pointer(first, nth, last, comp);
}

/**
 * @brief Position the nth element in a pointer range using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param nth Position that should contain the nth ordered element.
 * @param last One-past-end of the half-open range.
 */
template <class T>
void nth_element(T* first, T* nth, T* last) { nth_element(first, nth, last, less<T>()); }

/**
 * @brief Return true when the range is in nondecreasing order.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return True when the range is in nondecreasing order.
 */
template <class It>
bool is_sorted(It first, It last) {
  if (first == last) return true;
  It next = first;
  for (++next; next != last; ++first, ++next) if (*next < *first) return false;
  return true;
}

/**
 * @brief Return true when the range is sorted according to `comp`.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 * @return True when the range is sorted according to `comp`.
 */
template <class It, class Compare>
bool is_sorted(It first, It last, Compare comp) {
  if (first == last) return true;
  It next = first;
  for (++next; next != last; ++first, ++next) if (comp(*next, *first)) return false;
  return true;
}

/**
 * @brief Return the first iterator where sorted order fails, or last when fully sorted.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return The first iterator where sorted order fails, or last when fully sorted.
 */
template <class It>
It is_sorted_until(It first, It last) {
  if (first == last) return last;
  It next = first;
  for (++next; next != last; ++first, ++next) if (*next < *first) return next;
  return last;
}

/**
 * @brief Return the first iterator where `comp` sorted order fails, or last when fully sorted.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 * @return The first iterator where `comp` sorted order fails, or last when fully sorted.
 */
template <class It, class Compare>
It is_sorted_until(It first, It last, Compare comp) {
  if (first == last) return last;
  It next = first;
  for (++next; next != last; ++first, ++next) if (comp(*next, *first)) return next;
  return last;
}

/**
 * @brief Internal heap helper that restores ordering from a parent toward the leaves.
 * @param first Start of the half-open range.
 * @param count Requested element or character count.
 * @param parent Caller-supplied argument used by this operation.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void heap_sift_down(It first, unsigned count, unsigned parent, Compare comp) {
  for (;;) {
    const unsigned left = parent * 2u + 1u;
    const unsigned right = left + 1u;
    unsigned best = parent;
    if (left < count && comp(first[best], first[left])) best = left;
    if (right < count && comp(first[best], first[right])) best = right;
    if (best == parent) break;
    iter_swap(first + parent, first + best);
    parent = best;
  }
}

/**
 * @brief Rearrange a random-access range into a heap.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void make_heap(It first, It last, Compare comp) {
  const unsigned count = static_cast<unsigned>(last - first);
  for (unsigned i = count / 2u; i != 0u; --i) heap_sift_down(first, count, i - 1u, comp);
}

/**
 * @brief Build a heap using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class It>
void make_heap(It first, It last) { make_heap(first, last, less<typename It::value_type>()); }

/**
 * @brief Build a heap using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class T>
void make_heap(T* first, T* last) { make_heap(first, last, less<T>()); }

/**
 * @brief Restore heap ordering after appending an element at the end.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void push_heap(It first, It last, Compare comp) {
  unsigned child = static_cast<unsigned>(last - first);
  if (child < 2u) return;
  --child;
  while (child != 0u) {
    const unsigned parent = (child - 1u) / 2u;
    if (!comp(first[parent], first[child])) break;
    iter_swap(first + parent, first + child);
    child = parent;
  }
}

/**
 * @brief Restore heap order after appending an element using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class It>
void push_heap(It first, It last) { push_heap(first, last, less<typename It::value_type>()); }

/**
 * @brief Restore heap order after appending an element using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class T>
void push_heap(T* first, T* last) { push_heap(first, last, less<T>()); }

/**
 * @brief Move the top heap element to the end and restore heap ordering in the remaining range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void pop_heap(It first, It last, Compare comp) {
  const unsigned count = static_cast<unsigned>(last - first);
  if (count < 2u) return;
  iter_swap(first, last - 1);
  heap_sift_down(first, count - 1u, 0u, comp);
}

/**
 * @brief Move the heap top to the end using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class It>
void pop_heap(It first, It last) { pop_heap(first, last, less<typename It::value_type>()); }

/**
 * @brief Move the heap top to the end using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class T>
void pop_heap(T* first, T* last) { pop_heap(first, last, less<T>()); }

/**
 * @brief Repeatedly pop heap elements to turn a heap into a sorted range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 */
template <class It, class Compare>
void sort_heap(It first, It last, Compare comp) {
  while (last - first > 1) {
    pop_heap(first, last, comp);
    --last;
  }
}

/**
 * @brief Sort a heap range using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class It>
void sort_heap(It first, It last) { sort_heap(first, last, less<typename It::value_type>()); }

/**
 * @brief Sort a heap range using the default less-than comparator.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 */
template <class T>
void sort_heap(T* first, T* last) { sort_heap(first, last, less<T>()); }

/**
 * @brief Partition a range so predicate-satisfying elements appear first.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return Result described by the function brief.
 */
template <class It, class Pred>
It partition(It first, It last, Pred pred) {
  It out = first;
  for (It it = first; it != last; ++it) if (pred(*it)) { iter_swap(out, it); ++out; }
  return out;
}
/**
 * @brief Stably partition a bidirectional range without using auxiliary storage.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param pred Predicate used to test elements.
 * @return Result described by the function brief.
 */
template <class It, class Pred>
It stable_partition(It first, It last, Pred pred) {
  It boundary = first;
  while (boundary != last && pred(*boundary)) ++boundary;
  It it = boundary;
  while (it != last) {
    if (pred(*it)) {
      /*
       * Move the matching element left by adjacent swaps. This preserves the
       * relative order on both sides of the partition and uses only the caller's
       * range storage, at the cost of quadratic movement in the worst case.
       */
      It scan = it;
      while (scan != boundary) {
        It prev = scan;
        --prev;
        iter_swap(prev, scan);
        scan = prev;
      }
      ++boundary;
    }
    ++it;
  }
  return boundary;
}

/**
 * @brief Merge two sorted input ranges into an output range.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out>
Out merge(In1 a, In1 ae, In2 b, In2 be, Out out) {
  while (a != ae && b != be) {
    if (*b < *a) *out++ = *b++;
    else *out++ = *a++;
  }
  while (a != ae) *out++ = *a++;
  while (b != be) *out++ = *b++;
  return out;
}

/**
 * @brief Merge two comparator-sorted input ranges into an output range.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @param comp Strict weak ordering used for comparisons.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out, class Compare>
Out merge(In1 a, In1 ae, In2 b, In2 be, Out out, Compare comp) {
  while (a != ae && b != be) {
    if (comp(*b, *a)) *out++ = *b++;
    else *out++ = *a++;
  }
  while (a != ae) *out++ = *a++;
  while (b != be) *out++ = *b++;
  return out;
}

/**
 * @brief Return true when sorted range a contains every element from sorted range b.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @return True when sorted range a contains every element from sorted range b.
 */
template <class In1, class In2>
bool includes(In1 a, In1 ae, In2 b, In2 be) {
  while (b != be) {
    if (a == ae || *b < *a) return false;
    if (*a < *b) ++a;
    else { ++a; ++b; }
  }
  return true;
}

/**
 * @brief Return true when comparator-sorted range a contains every element from range b.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param comp Strict weak ordering used for comparisons.
 * @return True when comparator-sorted range a contains every element from range b.
 */
template <class In1, class In2, class Compare>
bool includes(In1 a, In1 ae, In2 b, In2 be, Compare comp) {
  while (b != be) {
    if (a == ae || comp(*b, *a)) return false;
    if (comp(*a, *b)) ++a;
    else { ++a; ++b; }
  }
  return true;
}

/**
 * @brief Write the sorted union of two sorted input ranges to output.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out>
Out set_union(In1 a, In1 ae, In2 b, In2 be, Out out) {
  while (a != ae && b != be) {
    if (*b < *a) *out++ = *b++;
    else if (*a < *b) *out++ = *a++;
    else { *out++ = *a++; ++b; }
  }
  return copy(b, be, copy(a, ae, out));
}

/**
 * @brief Write the sorted union of two comparator-sorted input ranges to output.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @param comp Strict weak ordering used for comparisons.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out, class Compare>
Out set_union(In1 a, In1 ae, In2 b, In2 be, Out out, Compare comp) {
  while (a != ae && b != be) {
    if (comp(*b, *a)) *out++ = *b++;
    else if (comp(*a, *b)) *out++ = *a++;
    else { *out++ = *a++; ++b; }
  }
  return copy(b, be, copy(a, ae, out));
}

/**
 * @brief Write the sorted intersection of two sorted input ranges to output.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out>
Out set_intersection(In1 a, In1 ae, In2 b, In2 be, Out out) {
  while (a != ae && b != be) {
    if (*a < *b) ++a;
    else if (*b < *a) ++b;
    else { *out++ = *a++; ++b; }
  }
  return out;
}

/**
 * @brief Write the sorted intersection of two comparator-sorted input ranges to output.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @param comp Strict weak ordering used for comparisons.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out, class Compare>
Out set_intersection(In1 a, In1 ae, In2 b, In2 be, Out out, Compare comp) {
  while (a != ae && b != be) {
    if (comp(*a, *b)) ++a;
    else if (comp(*b, *a)) ++b;
    else { *out++ = *a++; ++b; }
  }
  return out;
}

/**
 * @brief Write elements from the first sorted range that are absent from the second.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out>
Out set_difference(In1 a, In1 ae, In2 b, In2 be, Out out) {
  while (a != ae) {
    if (b == be) return copy(a, ae, out);
    if (*a < *b) *out++ = *a++;
    else if (*b < *a) ++b;
    else { ++a; ++b; }
  }
  return out;
}

/**
 * @brief Write elements from the first comparator-sorted range that are absent from the second.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @param comp Strict weak ordering used for comparisons.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out, class Compare>
Out set_difference(In1 a, In1 ae, In2 b, In2 be, Out out, Compare comp) {
  while (a != ae) {
    if (b == be) return copy(a, ae, out);
    if (comp(*a, *b)) *out++ = *a++;
    else if (comp(*b, *a)) ++b;
    else { ++a; ++b; }
  }
  return out;
}

/**
 * @brief Write elements that appear in exactly one of the two sorted ranges.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out>
Out set_symmetric_difference(In1 a, In1 ae, In2 b, In2 be, Out out) {
  while (a != ae && b != be) {
    if (*a < *b) *out++ = *a++;
    else if (*b < *a) *out++ = *b++;
    else { ++a; ++b; }
  }
  return copy(b, be, copy(a, ae, out));
}

/**
 * @brief Write elements that appear in exactly one comparator-sorted input range.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param be End of the second range.
 * @param out Caller-provided destination for produced values.
 * @param comp Strict weak ordering used for comparisons.
 * @return Result described by the function brief.
 */
template <class In1, class In2, class Out, class Compare>
Out set_symmetric_difference(In1 a, In1 ae, In2 b, In2 be, Out out, Compare comp) {
  while (a != ae && b != be) {
    if (comp(*a, *b)) *out++ = *a++;
    else if (comp(*b, *a)) *out++ = *b++;
    else { ++a; ++b; }
  }
  return copy(b, be, copy(a, ae, out));
}

/**
 * @brief Return the first sorted-range position where value could be inserted without moving earlier elements.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return The first sorted-range position where value could be inserted without moving earlier elements.
 */
template <class It, class T>
It lower_bound(It first, It last, const T& value) {
  while (first != last) {
    It mid = first + ((last - first) / 2);
    if (*mid < value) first = mid + 1;
    else last = mid;
  }
  return first;
}

/**
 * @brief Return the first comparator-sorted position where value could be inserted.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @param comp Strict weak ordering used for comparisons.
 * @return The first comparator-sorted position where value could be inserted.
 */
template <class It, class T, class Compare>
It lower_bound(It first, It last, const T& value, Compare comp) {
  while (first != last) {
    It mid = first + ((last - first) / 2);
    if (comp(*mid, value)) first = mid + 1;
    else last = mid;
  }
  return first;
}

/**
 * @brief Return the first sorted-range position after all elements equivalent to value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return The first sorted-range position after all elements equivalent to value.
 */
template <class It, class T>
It upper_bound(It first, It last, const T& value) {
  while (first != last) {
    It mid = first + ((last - first) / 2);
    if (!(value < *mid)) first = mid + 1;
    else last = mid;
  }
  return first;
}

/**
 * @brief Return the first comparator-sorted position after all elements equivalent to value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @param comp Strict weak ordering used for comparisons.
 * @return The first comparator-sorted position after all elements equivalent to value.
 */
template <class It, class T, class Compare>
It upper_bound(It first, It last, const T& value, Compare comp) {
  while (first != last) {
    It mid = first + ((last - first) / 2);
    if (!comp(value, *mid)) first = mid + 1;
    else last = mid;
  }
  return first;
}

/**
 * @brief Return the sorted-range subrange containing all elements equivalent to value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return The sorted-range subrange containing all elements equivalent to value.
 */
template <class It, class T>
pair<It, It> equal_range(It first, It last, const T& value) {
  return make_pair(lower_bound(first, last, value), upper_bound(first, last, value));
}

/**
 * @brief Return the comparator-sorted subrange containing all elements equivalent to value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @param comp Strict weak ordering used for comparisons.
 * @return The comparator-sorted subrange containing all elements equivalent to value.
 */
template <class It, class T, class Compare>
pair<It, It> equal_range(It first, It last, const T& value, Compare comp) {
  return make_pair(lower_bound(first, last, value, comp), upper_bound(first, last, value, comp));
}

/**
 * @brief Return true when value appears in a sorted range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return True when value appears in a sorted range.
 */
template <class It, class T>
bool binary_search(It first, It last, const T& value) {
  It it = lower_bound(first, last, value);
  return it != last && !(value < *it);
}

/**
 * @brief Return true when value appears in a comparator-sorted range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @param comp Strict weak ordering used for comparisons.
 * @return True when value appears in a comparator-sorted range.
 */
template <class It, class T, class Compare>
bool binary_search(It first, It last, const T& value, Compare comp) {
  It it = lower_bound(first, last, value, comp);
  return it != last && !comp(value, *it) && !comp(*it, value);
}

} // namespace sstl

#endif

