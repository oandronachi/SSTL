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

  /** @brief Construct an empty priority queue with the default comparator. */
  priority_queue() : c_(), comp_(Compare()) {}

  /** @brief Construct an empty priority queue with a caller-supplied comparator. */
  explicit priority_queue(const Compare& comp) : c_(), comp_(comp) {}

  /**
   * @brief Construct from an existing fixed-capacity container and heapify it.
   * @param comp Comparator used to determine priority.
   * @param cont Source container copied into the heap storage.
   */
  priority_queue(const Compare& comp, const Container& cont) : c_(cont), comp_(comp) {
    rebuild_heap();
  }

  /**
   * @brief Construct from an input range with the default comparator.
   * @param first Start of the half-open source range.
   * @param last One-past-end of the source range.
   */
  template <class InputIt>
  priority_queue(InputIt first, InputIt last) : c_(), comp_(Compare()) {
    append_range(first, last);
  }

  /**
   * @brief Construct from an input range with a caller-supplied comparator.
   * @param first Start of the half-open source range.
   * @param last One-past-end of the source range.
   * @param comp Comparator used to determine priority.
   */
  template <class InputIt>
  priority_queue(InputIt first, InputIt last, const Compare& comp) : c_(), comp_(comp) {
    append_range(first, last);
  }

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

  /** @brief Try to insert a value without invoking the active error policy. */
  bool try_push(const T& x) {
    if (!c_.try_push_back(x)) return false;
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

  /** @brief Try to remove the current top-priority value and optionally copy it to `out`. */
  bool try_pop(T* out) {
    if (c_.empty()) return false;
    if (out) *out = c_.front();
    if (c_.size() == 1u) {
      c_.pop_back();
      return true;
    }
    c_[0] = c_.back();
    c_.pop_back();
    sift_down(0u);
    return true;
  }

  /**
   * @brief Access the current top-priority value.
   * @return Result described by the function brief.
  */
  T& top() { return empty() ? fail_reference<T>("priority_queue::top empty") : c_.front(); } // LCOV_EXCL_BR_LINE
  /** @brief Return the top-priority value pointer, or null when empty. */
  T* try_top() { return c_.try_front(); }
  /**
   * @brief Const access the current top-priority value.
   * @return Result described by the function brief.
  */
  const T& top() const { return empty() ? fail_reference<const T>("priority_queue::top empty") : c_.front(); } // LCOV_EXCL_BR_LINE
  /** @brief Return the const top-priority value pointer, or null when empty. */
  const T* try_top() const { return c_.try_front(); }
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
  /**
   * @brief Return the fixed compile-time heap capacity without requiring an object.
   * @return The fixed compile-time heap capacity without requiring an object.
   */
  static size_type capacity() { return Container::capacity(); }
  /**
   * @brief Report whether the underlying fixed-capacity container is full.
   * @return `true` when another push would fail.
   */
  bool full() const { return c_.full(); }
  /**
   * @brief Exchange contents with another priority queue of the same type.
   * @param other Other priority queue participating in the operation.
   */
  void swap(priority_queue& other) {
    c_.swap(other.c_);
    sstl::swap(comp_, other.comp_);
  }

private:
  /** @brief Underlying fixed-capacity random-access container. */
  Container c_;
  /** @brief Comparator used to determine heap priority. */
  Compare comp_;

  /** @brief Push every range element until the range ends or capacity rejects one. */
  template <class InputIt>
  void append_range(InputIt first, InputIt last) {
    for (; first != last; ++first) {
      if (!push(*first)) break;
    }
  }

  /** @brief Restore heap ordering for the current contents. */
  void rebuild_heap() {
    if (c_.size() < 2u) return;
    size_type parent = c_.size() / 2u;
    while (parent != 0u) {
      --parent;
      sift_down(parent);
    }
  }

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

/** @brief Exchange two priority queues through the member swap operation. */
template <class T, size_t N, class Container, class Compare>
inline void swap(priority_queue<T, N, Container, Compare>& lhs,
                 priority_queue<T, N, Container, Compare>& rhs) {
  lhs.swap(rhs);
}

} // namespace sstl

#endif

