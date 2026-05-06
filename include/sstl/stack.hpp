/**
 * @file stack.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_STACK_HPP
/** @def SSTL_STACK_HPP
 * @brief Include guard for stack.hpp.
 */
#define SSTL_STACK_HPP
#include "vector.hpp"

namespace sstl {

template <class T, size_t N, class Container>
class stack;

template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator==(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator!=(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator<(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator<=(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator>(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator>=(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs);

/** @brief LIFO container adaptor backed by a fixed-capacity vector-compatible container. */
template <class T, size_t N, class Container = vector<T,N> >
class stack {
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator==(const stack<U, A, LContainer>& lhs, const stack<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator!=(const stack<U, A, LContainer>& lhs, const stack<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator<(const stack<U, A, LContainer>& lhs, const stack<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator<=(const stack<U, A, LContainer>& lhs, const stack<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator>(const stack<U, A, LContainer>& lhs, const stack<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator>=(const stack<U, A, LContainer>& lhs, const stack<U, B, RContainer>& rhs);
public:
  /** @brief Stored element type. */
  typedef T value_type;
  /** @brief Unsigned size type used by the underlying fixed-capacity container. */
  typedef size_t size_type;

  /**
   * @brief Push an element onto the top of the stack.
   * @param x Element value supplied by the caller.
   * @return `true` when the documented condition holds; otherwise `false`.
  */
  bool push(const T& x) { return c_.push_back(x); }
  /** @brief Try to push an element without invoking the active error policy. */
  bool try_push(const T& x) { return c_.try_push_back(x); }

  /** @brief Remove the current top element. */
  void pop() { c_.pop_back(); }
  /** @brief Try to pop the top element and optionally copy it to `out`. */
  bool try_pop(T* out) { return c_.try_pop_back(out); }

  /**
   * @brief Access the current top element.
   * @return Result described by the function brief.
  */
  T& top() { return empty() ? fail_reference<T>("stack::top empty") : c_.back(); } // LCOV_EXCL_BR_LINE
  /** @brief Return the top element pointer, or null when empty. */
  T* try_top() { return c_.try_back(); }
  /**
   * @brief Const access the current top element.
   * @return Result described by the function brief.
  */
  const T& top() const { return empty() ? fail_reference<const T>("stack::top empty") : c_.back(); } // LCOV_EXCL_BR_LINE
  /** @brief Return the const top element pointer, or null when empty. */
  const T* try_top() const { return c_.try_back(); }

  /**
   * @brief Report whether the stack currently contains no elements.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return c_.empty(); }

  /**
   * @brief Return the number of live elements held by the stack.
   * @return The number of live elements held by the stack.
   */
  size_type size() const { return c_.size(); }
  /**
   * @brief Return the fixed compile-time stack capacity without requiring an object.
   * @return The fixed compile-time stack capacity without requiring an object.
   */
  static size_type capacity() { return Container::capacity(); }
  /**
   * @brief Report whether the underlying fixed-capacity container is full.
   * @return `true` when the stack cannot accept another element.
   */
  bool full() const { return c_.full(); }
  /**
   * @brief Exchange contents with another stack of the same type.
   * @param other Other stack participating in the operation.
   */
  void swap(stack& other) { c_.swap(other.c_); }

private:
  /** @brief Underlying fixed-capacity sequence container. */
  Container c_;
};

/** @brief Exchange two stacks through the member swap operation. */
template <class T, size_t N, class Container>
inline void swap(stack<T, N, Container>& lhs, stack<T, N, Container>& rhs) { lhs.swap(rhs); }

/** @brief Compare two stack adaptors by their underlying sequences. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator==(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs) {
  return lhs.c_ == rhs.c_;
}

/** @brief Compare two stack adaptors for inequality. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator!=(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs) {
  return !(lhs == rhs);
}

/** @brief Lexicographically compare two stack adaptors by their underlying sequences. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator<(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs) {
  return lhs.c_ < rhs.c_;
}

/** @brief Return true when the left stack is not greater than the right stack. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator<=(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs) {
  return !(rhs < lhs);
}

/** @brief Return true when the left stack is greater than the right stack. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator>(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs) {
  return rhs < lhs;
}

/** @brief Return true when the left stack is not less than the right stack. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator>=(const stack<T, N, LContainer>& lhs, const stack<T, M, RContainer>& rhs) {
  return !(lhs < rhs);
}

}
#endif

