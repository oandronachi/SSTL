/**
 * @file queue.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_QUEUE_HPP
/** @def SSTL_QUEUE_HPP
 * @brief Include guard for queue.hpp.
 */
#define SSTL_QUEUE_HPP
#include "deque.hpp"

namespace sstl {

/** @brief FIFO container adaptor backed by a fixed-capacity deque-compatible container. */
template <class T, size_t N, class Container = deque<T,N> >
class queue {
public:
  /** @brief Stored element type. */
  typedef T value_type;
  /** @brief Unsigned size type used by the underlying fixed-capacity container. */
  typedef size_t size_type;

  /**
   * @brief Append an element at the logical back of the queue.
   * @param x Element value supplied by the caller.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool push(const T& x) { return c_.push_back(x); }

  /** @brief Remove the element at the logical front of the queue. */
  void pop() { c_.pop_front(); }

  /**
   * @brief Access the next element to be removed.
   * @return Result described by the function brief.
   */
  T& front() { return c_.front(); }
  /**
   * @brief Const access the next element to be removed.
   * @return Result described by the function brief.
   */
  const T& front() const { return c_.front(); }

  /**
   * @brief Access the most recently pushed element at the logical back.
   * @return Result described by the function brief.
   */
  T& back() { return c_.back(); }
  /**
   * @brief Const access the most recently pushed element at the logical back.
   * @return Result described by the function brief.
   */
  const T& back() const { return c_.back(); }

  /**
   * @brief Report whether the queue currently contains no elements.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return c_.empty(); }

  /**
   * @brief Return the number of live elements held by the queue.
   * @return The number of live elements held by the queue.
   */
  size_type size() const { return c_.size(); }

private:
  /** @brief Underlying fixed-capacity sequence container. */
  Container c_;
};

}
#endif

