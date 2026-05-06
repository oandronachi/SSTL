/**
 * @file array.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_ARRAY_HPP
/** @def SSTL_ARRAY_HPP
 * @brief Include guard for array.hpp.
 */
#define SSTL_ARRAY_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "type_traits.hpp"

namespace sstl {

/**
 * @brief Fixed-size aggregate-style array with STL-like accessors.
 * @tparam T Element type.
 * @tparam N Number of elements in the public sequence.
 *
 * `array<T, 0>` is a valid type. Internally it carries a private dummy slot to
 * satisfy C++ array rules, but `size()`, `max_size()`, and iteration still
 * report an empty public range.
 */
template <class T, size_t N>
class array {
public:
  /** @brief Element type stored by the fixed-size array. */
  typedef T value_type;
  /** @brief Unsigned size and index type used by the array. */
  typedef size_t size_type;
  /** @brief Mutable contiguous iterator over the array storage. */
  typedef T* iterator;
  /** @brief Const contiguous iterator over the array storage. */
  typedef const T* const_iterator;
  /** @brief Mutable reverse iterator over the contiguous array storage. */
  typedef reverse_pointer_iterator<T> reverse_iterator;
  /** @brief Const reverse iterator over the contiguous array storage. */
  typedef reverse_pointer_iterator<const T> const_reverse_iterator;

  /** @brief Public compile-time element count. */
  enum { static_size = N };

  /** @brief Construct all public elements with `T()`. */
  array() {
    for (size_type i = 0; i != N; ++i) SSTL_CONSTRUCT_AT(storage_.ptr(i), T());
  }

  /**
   * @brief Copy-construct every public element from `other`.
   * @param other Other object participating in the operation.
   */
  array(const array& other) {
    for (size_type i = 0; i != N; ++i) SSTL_CONSTRUCT_AT(storage_.ptr(i), other[i]);
  }

  /** @brief Destroy every public element. */
  ~array() {
    for (size_type i = 0; i != N; ++i) SSTL_DESTROY_AT(storage_.ptr(i));
  }

  /**
   * @brief Assign every public element from `other`.
   * @param other Other object participating in the operation.
   * @return Result described by the function brief.
   */
  array& operator=(const array& other) {
    if (this != &other) {
      for (size_type i = 0; i != N; ++i) (*this)[i] = other[i];
    }
    return *this;
  }

  /**
   * @brief Return the fixed public element count.
   * @return The fixed public element count.
   */
  size_type size() const { return N; }
  /**
   * @brief Return the fixed public element count.
   * @return The fixed public element count.
   */
  size_type max_size() const { return N; }
  /**
   * @brief Return the fixed public element count as a static query.
   * @return The fixed public element count as a static query.
   */
  static size_type capacity() { return N; }
  /**
   * @brief Report whether this array has zero public elements.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return N == 0; }

  /**
   * @brief Access element `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& operator[](size_type i) {
    if (i >= N) return fail_reference<T>("array::operator[]"); // LCOV_EXCL_BR_LINE
    return *storage_.ptr(i);
  }
  /**
   * @brief Const access element `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const T& operator[](size_type i) const {
    if (i >= N) return fail_reference<const T>("array::operator[]"); // LCOV_EXCL_BR_LINE
    return *storage_.ptr(i);
  }

  /**
   * @brief Access element `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& at(size_type i) {
    if (i >= N) return fail_reference<T>("array::at"); // LCOV_EXCL_BR_LINE
    return (*this)[i];
  }
  /**
   * @brief Const access element `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const T& at(size_type i) const {
    if (i >= N) return fail_reference<const T>("array::at"); // LCOV_EXCL_BR_LINE
    return (*this)[i];
  }
  /**
   * @brief Return a pointer to element `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return Pointer to element `i`, or null when out of range.
   */
  T* try_at(size_type i) { return i < N ? storage_.ptr(i) : 0; }
  /**
   * @brief Return a const pointer to element `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return Const pointer to element `i`, or null when out of range.
   */
  const T* try_at(size_type i) const { return i < N ? storage_.ptr(i) : 0; }

  /**
   * @brief Access the first public element.
   * @return Result described by the function brief.
   */
  T& front() { return (*this)[0]; }
  /**
   * @brief Const access the first public element.
   * @return Result described by the function brief.
   */
  const T& front() const { return (*this)[0]; }
  /**
   * @brief Access the last public element.
   * @return Result described by the function brief.
   */
  T& back() { return (*this)[N - 1]; }
  /**
   * @brief Const access the last public element.
   * @return Result described by the function brief.
   */
  const T& back() const { return (*this)[N - 1]; }
  /**
   * @brief Return a pointer to the first public element, or null when `N == 0`.
   * @return A pointer to the first public element, or null when `n == 0`.
   */
  T* try_front() { return N == 0u ? 0 : data(); }
  /**
   * @brief Return a const pointer to the first public element, or null when `N == 0`.
   * @return A const pointer to the first public element, or null when `n == 0`.
   */
  const T* try_front() const { return N == 0u ? 0 : data(); }
  /**
   * @brief Return a pointer to the last public element, or null when `N == 0`.
   * @return A pointer to the last public element, or null when `n == 0`.
   */
  T* try_back() { return N == 0u ? 0 : data() + N - 1u; }
  /**
   * @brief Return a const pointer to the last public element, or null when `N == 0`.
   * @return A const pointer to the last public element, or null when `n == 0`.
   */
  const T* try_back() const { return N == 0u ? 0 : data() + N - 1u; }
  /**
   * @brief Return a mutable pointer to the first storage slot.
   * @return A mutable pointer to the first storage slot.
   */
  T* data() { return storage_.ptr(0); }
  /**
   * @brief Return a const pointer to the first storage slot.
   * @return A const pointer to the first storage slot.
   */
  const T* data() const { return storage_.ptr(0); }

  /**
   * @brief Return an iterator to the first element.
   * @return An iterator to the first element.
   */
  iterator begin() { return iterator(data()); }
  /**
   * @brief Return a const iterator to the first element.
   * @return A const iterator to the first element.
   */
  const_iterator begin() const { return const_iterator(data()); }
  /**
   * @brief Return a const iterator to the first element.
   * @return A const iterator to the first element.
   */
  const_iterator cbegin() const { return const_iterator(data()); }
  /**
   * @brief Return an iterator one past the last element.
   * @return An iterator one past the last element.
   */
  iterator end() { return iterator(data() + N); }
  /**
   * @brief Return a const iterator one past the last element.
   * @return A const iterator one past the last element.
   */
  const_iterator end() const { return const_iterator(data() + N); }
  /**
   * @brief Return a const iterator one past the last element.
   * @return A const iterator one past the last element.
   */
  const_iterator cend() const { return const_iterator(data() + N); }
  /**
   * @brief Return a mutable reverse iterator to the last element.
   * @return A mutable reverse iterator to the last element.
   */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the last element.
   * @return A const reverse iterator to the last element.
   */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the last element.
   * @return A const reverse iterator to the last element.
   */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a mutable reverse iterator one before the first element.
   * @return A mutable reverse iterator one before the first element.
   */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first element.
   * @return A const reverse iterator one before the first element.
   */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first element.
   * @return A const reverse iterator one before the first element.
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

  /**
   * @brief Validate that a mutable iterator lies in this array's public range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const { return it >= data() && it <= data() + N; } // LCOV_EXCL_BR_LINE

  /**
   * @brief Validate that a const iterator lies in this array's public range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  template <class It>
  typename enable_if<is_same<It, const_iterator>::value && !is_same<iterator, const_iterator>::value, bool>::type
  is_valid_iterator(It it) const { return it >= data() && it <= data() + N; } // LCOV_EXCL_BR_LINE

  /**
   * @brief Assign the same value to every public element.
   * @param value Value copied into each slot.
   */
  void fill(const T& value) {
    for (size_type i = 0; i != N; ++i) (*this)[i] = value;
  }

  /**
   * @brief Exchange all public elements with another array of the same type.
   * @param other Array whose elements are swapped with this one.
   */
  void swap(array& other) {
    for (size_type i = 0; i != N; ++i) {
      T tmp((*this)[i]);
      (*this)[i] = other[i];
      other[i] = tmp;
    }
  }

private:
  /** @brief Inline storage containing exactly the public array elements. */
  raw_storage<T, N> storage_;
};

/**
 * @brief Zero-capacity specialization that never forms a `T()`.
 *
 * The primary template default-constructs each public element. For `N == 0`
 * there are no public elements, so this specialization avoids even naming a
 * default constructor. That keeps `array<non_default_constructible, 0>` a valid
 * type and prevents hidden lifetime side effects.
 */
template <class T>
class array<T, 0> {
public:
  /** @brief Element type named by the zero-capacity array API. */
  typedef T value_type;
  /** @brief Unsigned size and index type used by the zero-capacity specialization. */
  typedef size_t size_type;
  /** @brief Mutable iterator type for API parity; no public element is dereferenceable. */
  typedef T* iterator;
  /** @brief Const iterator type for API parity; no public element is dereferenceable. */
  typedef const T* const_iterator;
  /** @brief Mutable empty reverse iterator type for API parity. */
  typedef reverse_pointer_iterator<T> reverse_iterator;
  /** @brief Const empty reverse iterator type for API parity. */
  typedef reverse_pointer_iterator<const T> const_reverse_iterator;

  /** @brief Public compile-time element count for the zero-capacity specialization. */
  enum { static_size = 0 };

  /** @brief Construct an empty zero-capacity array. */
  array() {}
  /**
   * @brief Copy-construct an empty zero-capacity array.
   */
  array(const array&) {}
  /**
   * @brief Assigning a zero-capacity array has no element work to perform.
   * @return Result described by the function brief.
   */
  array& operator=(const array&) { return *this; }

  /**
   * @brief Return zero because the specialization has no public elements.
   * @return Zero because the specialization has no public elements.
   */
  size_type size() const { return 0u; }
  /**
   * @brief Return zero because the specialization has no public elements.
   * @return Zero because the specialization has no public elements.
   */
  size_type max_size() const { return 0u; }
  /**
   * @brief Return zero because the specialization has no public elements.
   * @return Zero because the specialization has no public elements.
   */
  static size_type capacity() { return 0u; }
  /**
   * @brief Always true for the zero-capacity specialization.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return true; }

  /**
   * @brief Policy path for unchecked access on a zero-capacity array.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& operator[](size_type i) { (void)i; return fail_reference<T>("array::operator[]"); } // LCOV_EXCL_LINE
  /**
   * @brief Policy path for unchecked const access on a zero-capacity array.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const T& operator[](size_type i) const { (void)i; return fail_reference<const T>("array::operator[]"); } // LCOV_EXCL_LINE
  /**
   * @brief Report out-of-range access through the active error policy.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& at(size_type i) { (void)i; return fail_reference<T>("array::at"); } // LCOV_EXCL_LINE
  /**
   * @brief Report const out-of-range access through the active error policy.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const T& at(size_type i) const { (void)i; return fail_reference<const T>("array::at"); } // LCOV_EXCL_LINE
  /**
   * @brief Return null because no zero-capacity array index is valid.
   * @param i Zero-based logical index.
   * @return Null because no element is present.
   */
  T* try_at(size_type i) { (void)i; return 0; }
  /**
   * @brief Return null because no zero-capacity array index is valid.
   * @param i Zero-based logical index.
   * @return Null because no element is present.
   */
  const T* try_at(size_type i) const { (void)i; return 0; }
  /**
   * @brief Report empty front access through the active error policy.
   * @return Result described by the function brief.
   */
  T& front() { return fail_reference<T>("array::front"); } // LCOV_EXCL_LINE
  /**
   * @brief Report empty const front access through the active error policy.
   * @return Result described by the function brief.
   */
  const T& front() const { return fail_reference<const T>("array::front"); } // LCOV_EXCL_LINE
  /**
   * @brief Report empty back access through the active error policy.
   * @return Result described by the function brief.
   */
  T& back() { return fail_reference<T>("array::back"); } // LCOV_EXCL_LINE
  /**
   * @brief Report empty const back access through the active error policy.
   * @return Result described by the function brief.
   */
  const T& back() const { return fail_reference<const T>("array::back"); } // LCOV_EXCL_LINE
  /**
   * @brief Return null because there is no public first element.
   * @return Null because there is no public first element.
   */
  T* try_front() { return 0; }
  /**
   * @brief Return null because there is no public first element.
   * @return Null because there is no public first element.
   */
  const T* try_front() const { return 0; }
  /**
   * @brief Return null because there is no public last element.
   * @return Null because there is no public last element.
   */
  T* try_back() { return 0; }
  /**
   * @brief Return null because there is no public last element.
   * @return Null because there is no public last element.
   */
  const T* try_back() const { return 0; }

  /**
   * @brief Return the dummy storage pointer; no public element is live.
   * @return The dummy storage pointer; no public element is live.
   */
  T* data() { return storage_.ptr(0); }
  /**
   * @brief Return the const dummy storage pointer; no public element is live.
   * @return The const dummy storage pointer; no public element is live.
   */
  const T* data() const { return storage_.ptr(0); }
  /**
   * @brief Return the empty begin iterator.
   * @return The empty begin iterator.
   */
  iterator begin() { return iterator(data()); }
  /**
   * @brief Return the empty const begin iterator.
   * @return The empty const begin iterator.
   */
  const_iterator begin() const { return const_iterator(data()); }
  /**
   * @brief Return the empty const begin iterator.
   * @return The empty const begin iterator.
   */
  const_iterator cbegin() const { return const_iterator(data()); }
  /**
   * @brief Return the empty end iterator.
   * @return The empty end iterator.
   */
  iterator end() { return iterator(data()); }
  /**
   * @brief Return the empty const end iterator.
   * @return The empty const end iterator.
   */
  const_iterator end() const { return const_iterator(data()); }
  /**
   * @brief Return the empty const end iterator.
   * @return The empty const end iterator.
   */
  const_iterator cend() const { return const_iterator(data()); }
  /**
   * @brief Return the empty mutable reverse begin iterator.
   * @return The empty mutable reverse begin iterator.
   */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /**
   * @brief Return the empty const reverse begin iterator.
   * @return The empty const reverse begin iterator.
   */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return the empty const reverse begin iterator.
   * @return The empty const reverse begin iterator.
   */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return the empty mutable reverse end iterator.
   * @return The empty mutable reverse end iterator.
   */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /**
   * @brief Return the empty const reverse end iterator.
   * @return The empty const reverse end iterator.
   */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return the empty const reverse end iterator.
   * @return The empty const reverse end iterator.
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

  /**
   * @brief Validate the only mutable iterator value exposed by the zero-capacity array.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const { return it == data(); }

  /**
   * @brief Validate the only const iterator value exposed by the zero-capacity array.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  template <class It>
  typename enable_if<is_same<It, const_iterator>::value && !is_same<iterator, const_iterator>::value, bool>::type
  is_valid_iterator(It it) const { return it == data(); }

  /** @brief No-op because no public elements exist. */
  void fill(const T&) {}
  /**
   * @brief No-op because no public elements exist.
   */
  void swap(array&) {}

private:
  /** @brief Dummy aligned storage used only to keep pointer-returning APIs well formed. */
  raw_storage<T, 0> storage_;
};

/**
 * @brief Compare two arrays for element-wise equality.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T, size_t N>
inline bool operator==(const array<T, N>& lhs, const array<T, N>& rhs) {
  for (unsigned i = 0u; i != N; ++i) {
    if (!(lhs[i] == rhs[i])) return false;
  }
  return true;
}

/**
 * @brief Compare two arrays for element-wise inequality.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T, size_t N>
inline bool operator!=(const array<T, N>& lhs, const array<T, N>& rhs) {
  return !(lhs == rhs);
}

/**
 * @brief Return true when `lhs` is lexicographically less than `rhs`.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when `lhs` is lexicographically less than `rhs`.
 */
template <class T, size_t N>
inline bool operator<(const array<T, N>& lhs, const array<T, N>& rhs) {
  for (unsigned i = 0u; i != N; ++i) {
    if (lhs[i] < rhs[i]) return true;
    if (rhs[i] < lhs[i]) return false;
  }
  return false;
}

/**
 * @brief Return true when `lhs` is lexicographically less than or equal to `rhs`.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when `lhs` is lexicographically less than or equal to `rhs`.
 */
template <class T, size_t N>
inline bool operator<=(const array<T, N>& lhs, const array<T, N>& rhs) {
  return !(rhs < lhs);
}

/**
 * @brief Return true when `lhs` is lexicographically greater than `rhs`.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when `lhs` is lexicographically greater than `rhs`.
 */
template <class T, size_t N>
inline bool operator>(const array<T, N>& lhs, const array<T, N>& rhs) {
  return rhs < lhs;
}

/**
 * @brief Return true when `lhs` is lexicographically greater than or equal to `rhs`.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when `lhs` is lexicographically greater than or equal to `rhs`.
 */
template <class T, size_t N>
inline bool operator>=(const array<T, N>& lhs, const array<T, N>& rhs) {
  return !(lhs < rhs);
}

} // namespace sstl

#endif

