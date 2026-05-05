/**
 * @file priority_queue.hpp
 * @brief Fixed-capacity binary-heap adaptor.
 *
 * The adaptor keeps the highest-priority element at index zero and restores the
 * heap with logarithmic sift operations after each mutation. The default
 * comparator follows the usual priority-queue convention: `less<T>` produces a
 * max-heap, while `greater<T>` produces a min-heap.
 */
#ifndef SSTL_PRIORITY_QUEUE_HPP
/** @def SSTL_PRIORITY_QUEUE_HPP
 * @brief Include guard for priority_queue.hpp.
 */
#define SSTL_PRIORITY_QUEUE_HPP

#include "vector.hpp"

namespace sstl {

/** @brief Fixed-capacity priority queue adaptor using a binary heap. */
template <class T, size_t N, class Container = vector<T, N>, class Compare = less<T> >
class priority_queue {
public:
  /** @brief Stored element type. */
  typedef T value_type;
  /** @brief Unsigned size type for heap counts and indices. */
  typedef size_t size_type;

  /**
   * @brief Insert a value and restore heap ordering.
   * @param x Element value supplied by the caller.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool push(const T& x) {
    if (!c_.push_back(x)) return false;
    sift_up(c_.size() - 1u);
    return true;
  }

  /** @brief Remove the current top-priority value. */
  void pop() {
    if (c_.empty()) {
      handle_error("priority_queue::pop empty");
      return;
    }
    if (c_.size() == 1u) {
      c_.pop_back();
      return;
    }
    c_[0] = c_.back();
    c_.pop_back();
    sift_down(0u);
  }

  /**
   * @brief Access the current top-priority value.
   * @return Result described by the function brief.
   */
  T& top() { return c_.front(); }
  /**
   * @brief Const access the current top-priority value.
   * @return Result described by the function brief.
   */
  const T& top() const { return c_.front(); }
  /**
   * @brief Report whether the heap contains no elements.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return c_.empty(); }
  /**
   * @brief Return the number of elements in the heap.
   * @return The number of elements in the heap.
   */
  size_type size() const { return c_.size(); }

private:
  /** @brief Underlying fixed-capacity random-access container. */
  Container c_;
  /** @brief Comparator used to determine heap priority. */
  Compare comp_;

  /**
   * @brief Return true when `a` should appear above `b` in the heap.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   * @return True when `a` should appear above `b` in the heap.
   */
  bool higher_priority(const T& a, const T& b) const {
    return comp_(b, a);
  }

  /**
   * @brief Move the element at `child` upward until the heap property holds.
   * @param child Caller-supplied argument used by this operation.
   */
  void sift_up(size_type child) {
    while (child != 0u) {
      const size_type parent = (child - 1u) / 2u;
      if (!higher_priority(c_[child], c_[parent])) break;
      sstl::swap(c_[child], c_[parent]);
      child = parent;
    }
  }

  /**
   * @brief Move the element at `parent` downward until the heap property holds.
   * @param parent Caller-supplied argument used by this operation.
   */
  void sift_down(size_type parent) {
    for (;;) {
      const size_type left = parent * 2u + 1u;
      const size_type right = left + 1u;
      size_type best = parent;
      if (left < c_.size() && higher_priority(c_[left], c_[best])) best = left;
      if (right < c_.size() && higher_priority(c_[right], c_[best])) best = right;
      if (best == parent) break;
      sstl::swap(c_[parent], c_[best]);
      parent = best;
    }
  }
};

} // namespace sstl

#endif

