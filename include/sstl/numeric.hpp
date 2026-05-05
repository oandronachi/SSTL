/**
 * @file numeric.hpp
 * @brief C++03 SSTL numeric algorithms with static, allocation-free behavior.
 *
 * The numeric subset lives in its own header so code can include the canonical
 * numeric surface without pulling in the rest of `algorithm.hpp`. The
 * implementations are intentionally simple and use only caller-provided output
 * storage.
 */
#ifndef SSTL_NUMERIC_HPP
/** @def SSTL_NUMERIC_HPP
 * @brief Include guard for numeric.hpp.
 */
#define SSTL_NUMERIC_HPP

#include "utility.hpp"

namespace sstl {

/**
 * @brief Fold a range by repeatedly adding each element to the running value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param init Caller-supplied argument used by this operation.
 * @return Result described by the function brief.
 */
template <class It, class T>
T accumulate(It first, It last, T init) {
  for (; first != last; ++first) init = init + *first;
  return init;
}

/**
 * @brief Fold two ranges by adding each pairwise product to the running value.
 * @param a First operand or first range start.
 * @param ae End of the first range.
 * @param b Second operand or second range start.
 * @param init Caller-supplied argument used by this operation.
 * @return Result described by the function brief.
 */
template <class It1, class It2, class T>
T inner_product(It1 a, It1 ae, It2 b, T init) {
  for (; a != ae; ++a, ++b) init = init + (*a * *b);
  return init;
}

/**
 * @brief Write prefix sums of the input range to the output range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In, class Out>
Out partial_sum(In first, In last, Out out) {
  if (first == last) return out;
  typename Out::value_type sum = *first;
  *out++ = sum;
  while (++first != last) { sum = sum + *first; *out++ = sum; }
  return out;
}

/**
 * @brief Write prefix sums of a pointer input range to a pointer output range.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out Caller-provided destination for produced values.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
template <class T>
T* partial_sum(T* first, T* last, T* out) {
  if (first == last) return out;
  T sum = *first;
  *out++ = sum;
  while (++first != last) { sum = sum + *first; *out++ = sum; }
  return out;
}

/**
 * @brief Write the first value and then pairwise adjacent differences to output.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
template <class In, class Out>
Out adjacent_difference(In first, In last, Out out) {
  if (first == last) return out;
  typename Out::value_type prev = *first;
  *out++ = prev;
  while (++first != last) { typename Out::value_type cur = *first; *out++ = cur - prev; prev = cur; }
  return out;
}

/**
 * @brief Write the first value and adjacent differences for pointer ranges.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param out Caller-provided destination for produced values.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
template <class T>
T* adjacent_difference(T* first, T* last, T* out) {
  if (first == last) return out;
  T prev = *first;
  *out++ = prev;
  while (++first != last) { T cur = *first; *out++ = cur - prev; prev = cur; }
  return out;
}

/**
 * @brief Fill the range with sequentially increasing values starting at value.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 */
template <class It, class T>
void iota(It first, It last, T value) {
  for (; first != last; ++first, ++value) *first = value;
}

} // namespace sstl

#endif

