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

/** @brief LIFO container adaptor backed by a fixed-capacity vector-compatible container. */
template <class T, size_t N, class Container = vector<T,N> >
class stack {
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

  /** @brief Remove the current top element. */
  void pop() { c_.pop_back(); }

  /**
   * @brief Access the current top element.
   * @return Result described by the function brief.
   */
  T& top() { return c_.back(); }
  /**
   * @brief Const access the current top element.
   * @return Result described by the function brief.
   */
  const T& top() const { return c_.back(); }

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

private:
  /** @brief Underlying fixed-capacity sequence container. */
  Container c_;
};

}
#endif

