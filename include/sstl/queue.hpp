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

template <class T, size_t N, class Container>
class queue;

template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator==(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator!=(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator<(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator<=(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator>(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs);
template <class T, size_t N, size_t M, class LContainer, class RContainer>
bool operator>=(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs);

/** @brief FIFO container adaptor backed by a fixed-capacity deque-compatible container. */
template <class T, size_t N, class Container = deque<T,N> >
class queue {
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator==(const queue<U, A, LContainer>& lhs, const queue<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator!=(const queue<U, A, LContainer>& lhs, const queue<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator<(const queue<U, A, LContainer>& lhs, const queue<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator<=(const queue<U, A, LContainer>& lhs, const queue<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator>(const queue<U, A, LContainer>& lhs, const queue<U, B, RContainer>& rhs);
  template <class U, size_t A, size_t B, class LContainer, class RContainer>
  friend bool operator>=(const queue<U, A, LContainer>& lhs, const queue<U, B, RContainer>& rhs);
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
  /** @brief Try to append an element without invoking the active error policy. */
  bool try_push(const T& x) { return c_.try_push_back(x); }

  /** @brief Remove the element at the logical front of the queue. */
  void pop() { c_.pop_front(); }
  /** @brief Try to remove the front element and optionally copy it to `out`. */
  bool try_pop(T* out) { return c_.try_pop_front(out); }

  /**
   * @brief Access the next element to be removed.
   * @return Result described by the function brief.
  */
  T& front() { return empty() ? fail_reference<T>("queue::front empty") : c_.front(); } // LCOV_EXCL_BR_LINE
  /** @brief Return the front element pointer, or null when empty. */
  T* try_front() { return c_.try_front(); }
  /**
   * @brief Const access the next element to be removed.
   * @return Result described by the function brief.
  */
  const T& front() const { return empty() ? fail_reference<const T>("queue::front empty") : c_.front(); } // LCOV_EXCL_BR_LINE
  /** @brief Return the const front element pointer, or null when empty. */
  const T* try_front() const { return c_.try_front(); }

  /**
   * @brief Access the most recently pushed element at the logical back.
   * @return Result described by the function brief.
  */
  T& back() { return empty() ? fail_reference<T>("queue::back empty") : c_.back(); } // LCOV_EXCL_BR_LINE
  /** @brief Return the back element pointer, or null when empty. */
  T* try_back() { return c_.try_back(); }
  /**
   * @brief Const access the most recently pushed element at the logical back.
   * @return Result described by the function brief.
  */
  const T& back() const { return empty() ? fail_reference<const T>("queue::back empty") : c_.back(); } // LCOV_EXCL_BR_LINE
  /** @brief Return the const back element pointer, or null when empty. */
  const T* try_back() const { return c_.try_back(); }

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
  /**
   * @brief Return the fixed compile-time queue capacity without requiring an object.
   * @return The fixed compile-time queue capacity without requiring an object.
   */
  static size_type capacity() { return Container::capacity(); }
  /**
   * @brief Report whether the underlying fixed-capacity container is full.
   * @return `true` when the queue cannot accept another element.
   */
  bool full() const { return c_.full(); }
  /**
   * @brief Exchange contents with another queue of the same type.
   * @param other Other queue participating in the operation.
   */
  void swap(queue& other) { c_.swap(other.c_); }

private:
  /** @brief Underlying fixed-capacity sequence container. */
  Container c_;
};

/** @brief Exchange two queues through the member swap operation. */
template <class T, size_t N, class Container>
inline void swap(queue<T, N, Container>& lhs, queue<T, N, Container>& rhs) { lhs.swap(rhs); }

/** @brief Compare two queue adaptors by their underlying sequences. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator==(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs) {
  return lhs.c_ == rhs.c_;
}

/** @brief Compare two queue adaptors for inequality. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator!=(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs) {
  return !(lhs == rhs);
}

/** @brief Lexicographically compare two queue adaptors by their underlying sequences. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator<(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs) {
  return lhs.c_ < rhs.c_;
}

/** @brief Return true when the left queue is not greater than the right queue. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator<=(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs) {
  return !(rhs < lhs);
}

/** @brief Return true when the left queue is greater than the right queue. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator>(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs) {
  return rhs < lhs;
}

/** @brief Return true when the left queue is not less than the right queue. */
template <class T, size_t N, size_t M, class LContainer, class RContainer>
inline bool operator>=(const queue<T, N, LContainer>& lhs, const queue<T, M, RContainer>& rhs) {
  return !(lhs < rhs);
}

}
#endif

