/**
 * @file iterator.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_ITERATOR_HPP
/** @def SSTL_ITERATOR_HPP
 * @brief Include guard for iterator.hpp.
 */
#define SSTL_ITERATOR_HPP

#include <stddef.h>

namespace sstl {

/** @brief Remove a top-level const qualifier from a contiguous iterator value type. */
template <class T>
struct iterator_remove_const {
  /** @brief The unqualified value type. */
  typedef T type;
};

/** @brief Remove a top-level const qualifier from a contiguous iterator value type. */
template <class T>
struct iterator_remove_const<const T> {
  /** @brief The unqualified value type. */
  typedef T type;
};

/** @brief Marker tag for single-pass input-style iterators. */
struct input_iterator_tag {};

/** @brief Marker tag for multipass forward-style iterators. */
struct forward_iterator_tag : input_iterator_tag {};

/** @brief Marker tag for iterators that can move both forward and backward. */
struct bidirectional_iterator_tag : forward_iterator_tag {};

/** @brief Marker tag for iterators supporting constant-time indexed access. */
struct random_access_iterator_tag : bidirectional_iterator_tag {};

/** @brief Iterator traits for SSTL iterator classes with nested typedefs. */
template <class Iterator>
struct iterator_traits {
  /** @brief Iterator difference type. */
  typedef typename Iterator::difference_type difference_type;
  /** @brief Value type named by the iterator. */
  typedef typename Iterator::value_type value_type;
  /** @brief Pointer type returned by the iterator. */
  typedef typename Iterator::pointer pointer;
  /** @brief Reference type returned by dereference. */
  typedef typename Iterator::reference reference;
  /** @brief Iterator category tag. */
  typedef typename Iterator::iterator_category iterator_category;
};

/** @brief Iterator traits specialization for mutable raw pointers. */
template <class T>
struct iterator_traits<T*> {
  /** @brief Pointer difference type. */
  typedef ptrdiff_t difference_type;
  /** @brief Pointed-to value type. */
  typedef T value_type;
  /** @brief Mutable pointer type. */
  typedef T* pointer;
  /** @brief Mutable reference type. */
  typedef T& reference;
  /** @brief Raw pointers are random-access iterators. */
  typedef random_access_iterator_tag iterator_category;
};

/** @brief Iterator traits specialization for const raw pointers. */
template <class T>
struct iterator_traits<const T*> {
  /** @brief Pointer difference type. */
  typedef ptrdiff_t difference_type;
  /** @brief Pointed-to value type. */
  typedef T value_type;
  /** @brief Const pointer type. */
  typedef const T* pointer;
  /** @brief Const reference type. */
  typedef const T& reference;
  /** @brief Raw pointers are random-access iterators. */
  typedef random_access_iterator_tag iterator_category;
};

/** @brief Count the number of increments needed to reach `last`. */
template <class InputIt>
inline typename iterator_traits<InputIt>::difference_type
distance_impl(InputIt first, InputIt last, input_iterator_tag) {
  typename iterator_traits<InputIt>::difference_type n = 0;
  for (; first != last; ++first) ++n;
  return n;
}

/** @brief Count the distance between two random-access iterators. */
template <class RandomIt>
inline typename iterator_traits<RandomIt>::difference_type
distance_impl(RandomIt first, RandomIt last, random_access_iterator_tag) {
  return last - first;
}

/** @brief Return the distance between two iterators. */
template <class InputIt>
inline typename iterator_traits<InputIt>::difference_type distance(InputIt first, InputIt last) {
  return distance_impl(first, last, typename iterator_traits<InputIt>::iterator_category());
}

/** @brief Advance an input iterator forward by `n` steps. */
template <class InputIt, class Distance>
inline void advance_impl(InputIt& it, Distance n, input_iterator_tag) {
  while (n > 0) {
    ++it;
    --n;
  }
}

/** @brief Advance a bidirectional iterator by a signed offset. */
template <class BidirectionalIt, class Distance>
inline void advance_impl(BidirectionalIt& it, Distance n, bidirectional_iterator_tag) {
  if (n >= 0) {
    while (n > 0) {
      ++it;
      --n;
    }
  } else {
    while (n < 0) {
      --it;
      ++n;
    }
  }
}

/** @brief Advance a random-access iterator by a signed offset. */
template <class RandomIt, class Distance>
inline void advance_impl(RandomIt& it, Distance n, random_access_iterator_tag) {
  it += n;
}

/** @brief Advance an iterator by `n` positions. */
template <class InputIt, class Distance>
inline void advance(InputIt& it, Distance n) {
  advance_impl(it, n, typename iterator_traits<InputIt>::iterator_category());
}

/** @brief Return an iterator advanced by `n` positions. */
template <class InputIt>
inline InputIt next(InputIt it, typename iterator_traits<InputIt>::difference_type n = 1) {
  sstl::advance(it, n);
  return it;
}

/** @brief Return an iterator moved backward by `n` positions. */
template <class BidirectionalIt>
inline BidirectionalIt prev(BidirectionalIt it, typename iterator_traits<BidirectionalIt>::difference_type n = 1) {
  sstl::advance(it, -n);
  return it;
}

/**
 * @brief Pointer-backed random-access iterator with standard nested typedefs.
 * @tparam T Pointed-to element type, including const-qualified element types.
 *
 * Contiguous SSTL containers store elements in fixed arrays, so a raw pointer
 * has the correct runtime representation. This wrapper keeps that pointer-like
 * behavior while making the public iterator type self-describing for generic
 * C++03 code that expects nested iterator typedefs.
 */
template <class T>
class contiguous_iterator {
public:
  /** @brief Element type exposed through iterator traits-style code. */
  typedef typename iterator_remove_const<T>::type value_type;
  /** @brief Signed distance type for pointer differences. */
  typedef ptrdiff_t difference_type;
  /** @brief Pointer type returned by `operator->()` and `base()`. */
  typedef T* pointer;
  /** @brief Reference type returned by `operator*()`. */
  typedef T& reference;
  /** @brief Iterator category tag matching pointer random-access behavior. */
  typedef random_access_iterator_tag iterator_category;

  /** @brief Construct a null iterator. */
  contiguous_iterator() : current_(0) {}
  /**
   * @brief Construct an iterator from a contiguous raw pointer.
   * @param current Caller-supplied argument used by this operation.
   */
  contiguous_iterator(pointer current) : current_(current) {}
  /**
   * @brief Construct a const iterator from a compatible mutable iterator.
   * @param other Other object participating in the operation.
   */
  template <class U>
  contiguous_iterator(const contiguous_iterator<U>& other) : current_(other.base()) {}

  /**
   * @brief Return the wrapped raw pointer.
   * @return The wrapped raw pointer.
   */
  pointer base() const { return current_; }
  /** @brief Provide source compatibility for code paths that expect raw pointers. */
  operator pointer() const { return current_; }

  /**
   * @brief Return the pointed-to element.
   * @return The pointed-to element.
   */
  reference operator*() const { return *current_; }
  /**
   * @brief Return a pointer to the pointed-to element.
   * @return A pointer to the pointed-to element.
   */
  pointer operator->() const { return current_; }

  /**
   * @brief Advance to the next contiguous element.
   * @return Result described by the function brief.
   */
  contiguous_iterator& operator++() { ++current_; return *this; }
  /**
   * @brief Return the old iterator, then advance to the next element.
   * @return The old iterator, then advance to the next element.
   */
  contiguous_iterator operator++(int) {
    contiguous_iterator old(*this);
    ++current_;
    return old;
  }
  /**
   * @brief Move to the previous contiguous element.
   * @return Result described by the function brief.
   */
  contiguous_iterator& operator--() { --current_; return *this; }
  /**
   * @brief Return the old iterator, then move to the previous element.
   * @return The old iterator, then move to the previous element.
   */
  contiguous_iterator operator--(int) {
    contiguous_iterator old(*this);
    --current_;
    return old;
  }

  /**
   * @brief Advance by `n` contiguous positions.
   * @param n Requested count or size.
   * @return Result described by the function brief.
   */
  contiguous_iterator& operator+=(difference_type n) { current_ += n; return *this; }
  /**
   * @brief Retreat by `n` contiguous positions.
   * @param n Requested count or size.
   * @return Result described by the function brief.
   */
  contiguous_iterator& operator-=(difference_type n) { current_ -= n; return *this; }
  /**
   * @brief Return an iterator advanced by `n` contiguous positions.
   * @param n Requested count or size.
   * @return An iterator advanced by `n` contiguous positions.
   */
  contiguous_iterator operator+(difference_type n) const {
    contiguous_iterator out(*this);
    out += n;
    return out;
  }
  /**
   * @brief Return an iterator retreated by `n` contiguous positions.
   * @param n Requested count or size.
   * @return An iterator retreated by `n` contiguous positions.
   */
  contiguous_iterator operator-(difference_type n) const {
    contiguous_iterator out(*this);
    out -= n;
    return out;
  }
  /**
   * @brief Return the distance from another contiguous iterator.
   * @param other Other object participating in the operation.
   * @return The distance from another contiguous iterator.
   */
  template <class U>
  difference_type operator-(const contiguous_iterator<U>& other) const {
    return static_cast<difference_type>(current_ - other.base());
  }
  /**
   * @brief Return the element `n` contiguous positions ahead.
   * @param n Requested count or size.
   * @return The element `n` contiguous positions ahead.
   */
  reference operator[](difference_type n) const { return current_[n]; }

  /**
   * @brief Compare whether two iterators point to the same element.
   * @param other Other object participating in the operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  template <class U>
  bool operator==(const contiguous_iterator<U>& other) const { return current_ == other.base(); }
  /**
   * @brief Compare whether two iterators point to different elements.
   * @param other Other object participating in the operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  template <class U>
  bool operator!=(const contiguous_iterator<U>& other) const { return !(*this == other); }
  /**
   * @brief Return true when this iterator appears before `other`.
   * @param other Other object participating in the operation.
   * @return True when this iterator appears before `other`.
   */
  template <class U>
  bool operator<(const contiguous_iterator<U>& other) const { return current_ < other.base(); }
  /**
   * @brief Return true when this iterator appears after `other`.
   * @param other Other object participating in the operation.
   * @return True when this iterator appears after `other`.
   */
  template <class U>
  bool operator>(const contiguous_iterator<U>& other) const { return other < *this; }
  /**
   * @brief Return true when this iterator is not after `other`.
   * @param other Other object participating in the operation.
   * @return True when this iterator is not after `other`.
   */
  template <class U>
  bool operator<=(const contiguous_iterator<U>& other) const { return !(other < *this); }
  /**
   * @brief Return true when this iterator is not before `other`.
   * @param other Other object participating in the operation.
   * @return True when this iterator is not before `other`.
   */
  template <class U>
  bool operator>=(const contiguous_iterator<U>& other) const { return !(*this < other); }

private:
  /** @brief Allow const and mutable iterator specializations to inspect each other. */
  template <class U>
  friend class contiguous_iterator;

  /** @brief Wrapped raw pointer into contiguous fixed-capacity storage. */
  pointer current_;
};

/**
 * @brief Return a contiguous iterator advanced by `n` positions.
 * @param n Requested count or size.
 * @param it Caller-supplied argument used by this operation.
 * @return A contiguous iterator advanced by `n` positions.
 */
template <class T>
inline contiguous_iterator<T> operator+(typename contiguous_iterator<T>::difference_type n, const contiguous_iterator<T>& it) {
  return it + n;
}

/**
 * @brief Reverse iterator adapter for SSTL iterator classes.
 * @tparam Iterator Forward iterator type used as the reverse iterator base.
 *
 * The adapter follows the standard reverse-iterator convention: the stored
 * base iterator points one position past the element returned by dereference.
 */
template <class Iterator>
class reverse_iterator_adaptor {
public:
  /** @brief Wrapped forward iterator type. */
  typedef Iterator iterator_type;
  /** @brief Value type produced by dereferencing this iterator. */
  typedef typename iterator_traits<Iterator>::value_type value_type;
  /** @brief Signed distance type used by iterator arithmetic. */
  typedef typename iterator_traits<Iterator>::difference_type difference_type;
  /** @brief Pointer type returned by `operator->()`. */
  typedef typename iterator_traits<Iterator>::pointer pointer;
  /** @brief Reference type returned by `operator*()`. */
  typedef typename iterator_traits<Iterator>::reference reference;
  /** @brief Iterator category matching the wrapped iterator. */
  typedef typename iterator_traits<Iterator>::iterator_category iterator_category;

  /** @brief Construct a reverse iterator with a default base iterator. */
  reverse_iterator_adaptor() : current_() {}
  /**
   * @brief Construct a reverse iterator from a forward base iterator.
   * @param current Forward iterator one past the reverse element.
   */
  explicit reverse_iterator_adaptor(iterator_type current) : current_(current) {}

  /**
   * @brief Return the wrapped forward base iterator.
   * @return The wrapped forward base iterator.
   */
  iterator_type base() const { return current_; }

  /**
   * @brief Return the element immediately before the wrapped base iterator.
   * @return The element immediately before the wrapped base iterator.
   */
  reference operator*() const {
    iterator_type tmp = current_;
    --tmp;
    return *tmp;
  }

  /**
   * @brief Return a pointer to the reverse element.
   * @return A pointer to the reverse element.
   */
  pointer operator->() const { return &operator*(); }

  /**
   * @brief Advance to the next reverse element.
   * @return Result described by the function brief.
   */
  reverse_iterator_adaptor& operator++() { --current_; return *this; }
  /**
   * @brief Post-increment and return the previous reverse iterator.
   * @return The previous reverse iterator value.
   */
  reverse_iterator_adaptor operator++(int) {
    reverse_iterator_adaptor old(*this);
    --current_;
    return old;
  }
  /**
   * @brief Move to the previous reverse element.
   * @return Result described by the function brief.
   */
  reverse_iterator_adaptor& operator--() { ++current_; return *this; }
  /**
   * @brief Post-decrement and return the previous reverse iterator.
   * @return The previous reverse iterator value.
   */
  reverse_iterator_adaptor operator--(int) {
    reverse_iterator_adaptor old(*this);
    ++current_;
    return old;
  }

  /**
   * @brief Advance by `n` reverse positions when the base iterator supports it.
   * @param n Requested count or size.
   * @return Result described by the function brief.
   */
  reverse_iterator_adaptor& operator+=(difference_type n) { current_ -= n; return *this; }
  /**
   * @brief Retreat by `n` reverse positions when the base iterator supports it.
   * @param n Requested count or size.
   * @return Result described by the function brief.
   */
  reverse_iterator_adaptor& operator-=(difference_type n) { current_ += n; return *this; }
  /**
   * @brief Return a reverse iterator advanced by `n` positions.
   * @param n Requested count or size.
   * @return A reverse iterator advanced by `n` positions.
   */
  reverse_iterator_adaptor operator+(difference_type n) const {
    reverse_iterator_adaptor out(*this);
    out += n;
    return out;
  }
  /**
   * @brief Return a reverse iterator retreated by `n` positions.
   * @param n Requested count or size.
   * @return A reverse iterator retreated by `n` positions.
   */
  reverse_iterator_adaptor operator-(difference_type n) const {
    reverse_iterator_adaptor out(*this);
    out -= n;
    return out;
  }
  /**
   * @brief Return the reverse distance from `other` to this iterator.
   * @param other Other object participating in the operation.
   * @return The reverse distance from `other` to this iterator.
   */
  difference_type operator-(const reverse_iterator_adaptor& other) const {
    return other.current_ - current_;
  }
  /**
   * @brief Access the element `n` reverse positions ahead.
   * @param n Requested count or size.
   * @return The element `n` reverse positions ahead.
   */
  reference operator[](difference_type n) const { return *(*this + n); }

  /**
   * @brief Compare two reverse iterators for equality.
   * @param other Other object participating in the operation.
   * @return `true` when both iterators have the same base.
   */
  bool operator==(const reverse_iterator_adaptor& other) const { return current_ == other.current_; }
  /**
   * @brief Compare two reverse iterators for inequality.
   * @param other Other object participating in the operation.
   * @return `true` when the iterators have different bases.
   */
  bool operator!=(const reverse_iterator_adaptor& other) const { return current_ != other.current_; }
  /**
   * @brief Compare reverse iterator ordering when the base iterator supports it.
   * @param other Other object participating in the operation.
   * @return `true` when this reverse iterator appears before `other`.
   */
  bool operator<(const reverse_iterator_adaptor& other) const { return other.current_ < current_; }
  /**
   * @brief Compare reverse iterator ordering when the base iterator supports it.
   * @param other Other object participating in the operation.
   * @return `true` when this reverse iterator appears after `other`.
   */
  bool operator>(const reverse_iterator_adaptor& other) const { return other < *this; }
  /**
   * @brief Compare reverse iterator ordering when the base iterator supports it.
   * @param other Other object participating in the operation.
   * @return `true` when this reverse iterator is not after `other`.
   */
  bool operator<=(const reverse_iterator_adaptor& other) const { return !(other < *this); }
  /**
   * @brief Compare reverse iterator ordering when the base iterator supports it.
   * @param other Other object participating in the operation.
   * @return `true` when this reverse iterator is not before `other`.
   */
  bool operator>=(const reverse_iterator_adaptor& other) const { return !(*this < other); }

private:
  /** @brief Wrapped forward iterator one past the reverse element. */
  iterator_type current_;
};

/**
 * @brief Reverse iterator adapter for raw pointer iterators.
 * @tparam T Pointed-to element type, including const-qualified element types.
 *
 * Contiguous SSTL containers expose raw pointers as their forward iterators.
 * This adapter provides the matching reverse-iterator surface without relying
 * on `<iterator>` or any hosted standard-library machinery. The stored pointer
 * follows the standard reverse-iterator convention: it is the forward iterator
 * one position past the element returned by `operator*()`.
 */
template <class T>
class reverse_pointer_iterator {
public:
  /** @brief Forward pointer type wrapped by this reverse iterator. */
  typedef T* iterator_type;
  /** @brief Element type produced by dereferencing this iterator. */
  typedef T value_type;
  /** @brief Signed distance type for pointer differences. */
  typedef ptrdiff_t difference_type;
  /** @brief Pointer type returned by `operator->()`. */
  typedef T* pointer;
  /** @brief Reference type returned by `operator*()`. */
  typedef T& reference;
  /** @brief Iterator category tag matching pointer random-access behavior. */
  typedef random_access_iterator_tag iterator_category;

  /** @brief Construct a reverse iterator with a null base pointer. */
  reverse_pointer_iterator() : current_(0) {}
  /**
   * @brief Construct a reverse iterator from the forward base pointer.
   * @param current Caller-supplied argument used by this operation.
   */
  explicit reverse_pointer_iterator(iterator_type current) : current_(current) {}

  /**
   * @brief Return the wrapped forward iterator one past the reverse element.
   * @return The wrapped forward iterator one past the reverse element.
   */
  iterator_type base() const { return current_; }

  /**
   * @brief Return the element immediately before the wrapped forward base.
   * @return The element immediately before the wrapped forward base.
   */
  reference operator*() const {
    iterator_type tmp = current_;
    --tmp;
    return *tmp;
  }

  /**
   * @brief Return a pointer to the element exposed by `operator*()`.
   * @return A pointer to the element exposed by `operator*()`.
   */
  pointer operator->() const { return &operator*(); }

  /**
   * @brief Move to the next reverse element, which decrements the base pointer.
   * @return Result described by the function brief.
   */
  reverse_pointer_iterator& operator++() { --current_; return *this; }
  /**
   * @brief Return the old iterator, then move to the next reverse element.
   * @return The old iterator, then move to the next reverse element.
   */
  reverse_pointer_iterator operator++(int) {
    reverse_pointer_iterator old(*this);
    --current_;
    return old;
  }
  /**
   * @brief Move to the previous reverse element, which increments the base pointer.
   * @return Result described by the function brief.
   */
  reverse_pointer_iterator& operator--() { ++current_; return *this; }
  /**
   * @brief Return the old iterator, then move to the previous reverse element.
   * @return The old iterator, then move to the previous reverse element.
   */
  reverse_pointer_iterator operator--(int) {
    reverse_pointer_iterator old(*this);
    ++current_;
    return old;
  }

  /**
   * @brief Advance by `n` reverse positions.
   * @param n Requested count or size.
   * @return Result described by the function brief.
   */
  reverse_pointer_iterator& operator+=(difference_type n) { current_ -= n; return *this; }
  /**
   * @brief Retreat by `n` reverse positions.
   * @param n Requested count or size.
   * @return Result described by the function brief.
   */
  reverse_pointer_iterator& operator-=(difference_type n) { current_ += n; return *this; }
  /**
   * @brief Return an iterator advanced by `n` reverse positions.
   * @param n Requested count or size.
   * @return An iterator advanced by `n` reverse positions.
   */
  reverse_pointer_iterator operator+(difference_type n) const {
    reverse_pointer_iterator out(*this);
    out += n;
    return out;
  }
  /**
   * @brief Return an iterator retreated by `n` reverse positions.
   * @param n Requested count or size.
   * @return An iterator retreated by `n` reverse positions.
   */
  reverse_pointer_iterator operator-(difference_type n) const {
    reverse_pointer_iterator out(*this);
    out -= n;
    return out;
  }
  /**
   * @brief Return the reverse distance from `other` to this iterator.
   * @param other Other object participating in the operation.
   * @return The reverse distance from `other` to this iterator.
   */
  difference_type operator-(const reverse_pointer_iterator& other) const {
    return static_cast<difference_type>(other.current_ - current_);
  }
  /**
   * @brief Return the element `n` reverse positions ahead.
   * @param n Requested count or size.
   * @return The element `n` reverse positions ahead.
   */
  reference operator[](difference_type n) const { return *(*this + n); }

  /**
   * @brief Compare whether two reverse iterators wrap the same base pointer.
   * @param other Other object participating in the operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool operator==(const reverse_pointer_iterator& other) const { return current_ == other.current_; }
  /**
   * @brief Compare whether two reverse iterators wrap different base pointers.
   * @param other Other object participating in the operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool operator!=(const reverse_pointer_iterator& other) const { return current_ != other.current_; }
  /**
   * @brief Return true when this reverse iterator appears before `other`.
   * @param other Other object participating in the operation.
   * @return True when this reverse iterator appears before `other`.
   */
  bool operator<(const reverse_pointer_iterator& other) const { return other.current_ < current_; }
  /**
   * @brief Return true when this reverse iterator appears after `other`.
   * @param other Other object participating in the operation.
   * @return True when this reverse iterator appears after `other`.
   */
  bool operator>(const reverse_pointer_iterator& other) const { return other < *this; }
  /**
   * @brief Return true when this reverse iterator is not after `other`.
   * @param other Other object participating in the operation.
   * @return True when this reverse iterator is not after `other`.
   */
  bool operator<=(const reverse_pointer_iterator& other) const { return !(other < *this); }
  /**
   * @brief Return true when this reverse iterator is not before `other`.
   * @param other Other object participating in the operation.
   * @return True when this reverse iterator is not before `other`.
   */
  bool operator>=(const reverse_pointer_iterator& other) const { return !(*this < other); }

private:
  /** @brief Wrapped forward iterator one past the reverse element. */
  iterator_type current_;
};

/**
 * @brief Return a reverse pointer iterator advanced by `n` positions.
 * @param n Requested count or size.
 * @param it Caller-supplied argument used by this operation.
 * @return A reverse pointer iterator advanced by `n` positions.
 */
template <class T>
inline reverse_pointer_iterator<T> operator+(int n, const reverse_pointer_iterator<T>& it) {
  return it + n;
}

}
#endif

