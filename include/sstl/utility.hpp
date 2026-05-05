/**
 * @file utility.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_UTILITY_HPP
/** @def SSTL_UTILITY_HPP
 * @brief Include guard for utility.hpp.
 */
#define SSTL_UTILITY_HPP

#include "config.hpp"

namespace sstl {

/** @brief Two-value aggregate used by associative containers and result-returning helpers. */
template <class First, class Second>
struct pair {
  /** @brief Public type alias for the first stored value. */
  typedef First first_type;
  /** @brief Public type alias for the second stored value. */
  typedef Second second_type;

  /** @brief First value in the pair. */
  First first;
  /** @brief Second value in the pair. */
  Second second;

  /** @brief Default-construct both stored values. */
  pair() : first(), second() {}
  /**
   * @brief Construct both stored values from explicit inputs.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  pair(const First& a, const Second& b) : first(a), second(b) {}

  /**
   * @brief Cross-type copy constructor for pair-compatible values.
   * @param other Other object participating in the operation.
   */
  template <class F2, class S2>
  pair(const pair<F2, S2>& other) : first(other.first), second(other.second) {}
};

/**
 * @brief Build an `sstl::pair` while letting template argument deduction choose the stored types.
 * @param f Callable object or function wrapper.
 * @param s String or set instance.
 * @return Result described by the function brief.
 */
template <class F, class S>
inline pair<F, S> make_pair(const F& f, const S& s) {
  return pair<F, S>(f, s);
}

/**
 * @brief Compare both pair members for equality.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class F, class S>
inline bool operator==(const pair<F, S>& a, const pair<F, S>& b) {
  return a.first == b.first && a.second == b.second;
}

/**
 * @brief Compare both pair members for inequality.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class F, class S>
inline bool operator!=(const pair<F, S>& a, const pair<F, S>& b) {
  return !(a == b);
}

/**
 * @brief Lexicographically compare pair values by first member, then second member.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class F, class S>
inline bool operator<(const pair<F, S>& a, const pair<F, S>& b) {
  return a.first < b.first || (!(b.first < a.first) && a.second < b.second);
}

/**
 * @brief Return true when `a` is not greater than `b`.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return True when `a` is not greater than `b`.
 */
template <class F, class S>
inline bool operator<=(const pair<F, S>& a, const pair<F, S>& b) {
  return !(b < a);
}

/**
 * @brief Return true when `a` is greater than `b` using pair lexicographic ordering.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return True when `a` is greater than `b` using pair lexicographic ordering.
 */
template <class F, class S>
inline bool operator>(const pair<F, S>& a, const pair<F, S>& b) {
  return b < a;
}

/**
 * @brief Return true when `a` is not less than `b`.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return True when `a` is not less than `b`.
 */
template <class F, class S>
inline bool operator>=(const pair<F, S>& a, const pair<F, S>& b) {
  return !(a < b);
}

/**
 * @brief Exchange two objects using copy construction and assignment only.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 */
template <class T>
inline void swap(T& a, T& b) {
  T tmp(a);
  a = b;
  b = tmp;
}

/**
 * @brief Return a mutable iterator to the first element of a container.
 * @param c Character value.
 * @return A mutable iterator to the first element of a container.
 */
template <class Container>
inline typename Container::iterator begin(Container& c) {
  return c.begin();
}

/**
 * @brief Return a const iterator to the first element of a container.
 * @param c Character value.
 * @return A const iterator to the first element of a container.
 */
template <class Container>
inline typename Container::const_iterator begin(const Container& c) {
  return c.begin();
}

/**
 * @brief Return a mutable iterator one past the last element of a container.
 * @param c Character value.
 * @return A mutable iterator one past the last element of a container.
 */
template <class Container>
inline typename Container::iterator end(Container& c) {
  return c.end();
}

/**
 * @brief Return a const iterator one past the last element of a container.
 * @param c Character value.
 * @return A const iterator one past the last element of a container.
 */
template <class Container>
inline typename Container::const_iterator end(const Container& c) {
  return c.end();
}

/**
 * @brief Return the number of elements reported by a container.
 * @param c Character value.
 * @return The number of elements reported by a container.
 */
template <class Container>
inline size_t size(const Container& c) {
  return c.size();
}

/**
 * @brief Return true when a container reports no elements.
 * @param c Character value.
 * @return True when a container reports no elements.
 */
template <class Container>
inline bool empty(const Container& c) {
  return c.empty();
}

/**
 * @brief Return mutable contiguous storage for containers that expose `data()`.
 * @param c Character value.
 * @return Mutable contiguous storage for containers that expose `data()`.
 */
template <class Container>
inline typename Container::value_type* data(Container& c) {
  return c.data();
}

/**
 * @brief Return const contiguous storage for containers that expose `data()`.
 * @param c Character value.
 * @return Const contiguous storage for containers that expose `data()`.
 */
template <class Container>
inline const typename Container::value_type* data(const Container& c) {
  return c.data();
}

/**
 * @brief Return a mutable pointer to the first element of a raw C array.
 * @param a First operand or first range start.
 * @return A mutable pointer to the first element of a raw c array.
 */
template <class T, size_t N>
inline T* begin(T (&a)[N]) {
  return a;
}

/**
 * @brief Return a const pointer to the first element of a raw C array.
 * @param a First operand or first range start.
 * @return A const pointer to the first element of a raw c array.
 */
template <class T, size_t N>
inline const T* begin(const T (&a)[N]) {
  return a;
}

/**
 * @brief Return a mutable pointer one past the final element of a raw C array.
 * @param a First operand or first range start.
 * @return A mutable pointer one past the final element of a raw c array.
 */
template <class T, size_t N>
inline T* end(T (&a)[N]) {
  return a + N;
}

/**
 * @brief Return a const pointer one past the final element of a raw C array.
 * @param a First operand or first range start.
 * @return A const pointer one past the final element of a raw c array.
 */
template <class T, size_t N>
inline const T* end(const T (&a)[N]) {
  return a + N;
}

/**
 * @brief Return the compile-time extent of a raw C array.
 * @param a Raw C array whose compile-time extent is requested.
 * @return The compile-time extent of a raw c array.
 */
template <class T, size_t N>
inline size_t size(const T (&a)[N]) {
  (void)a;
  return N;
}

/**
 * @brief Raw C arrays with a positive C++ extent are never empty.
 * @param a Raw C array being queried.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T, size_t N>
inline bool empty(const T (&a)[N]) {
  (void)a;
  return false;
}

/**
 * @brief Return a mutable pointer to raw C array storage.
 * @param a First operand or first range start.
 * @return A mutable pointer to raw c array storage.
 */
template <class T, size_t N>
inline T* data(T (&a)[N]) {
  return a;
}

/**
 * @brief Return a const pointer to raw C array storage.
 * @param a First operand or first range start.
 * @return A const pointer to raw c array storage.
 */
template <class T, size_t N>
inline const T* data(const T (&a)[N]) {
  return a;
}

} // namespace sstl

#endif

