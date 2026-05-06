/**
 * @file vector.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_VECTOR_HPP
/** @def SSTL_VECTOR_HPP
 * @brief Include guard for vector.hpp.
 */
#define SSTL_VECTOR_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

namespace sstl {

/**
 * @brief Contiguous fixed-capacity vector with C++03-compatible API.
 * @tparam T Element type stored in inline raw storage.
 * @tparam N Maximum number of live elements.
 *
 * Iterators are raw pointers into the inline storage. Any insert/erase after an
 * iterator is taken should be treated like normal vector invalidation: pointers
 * at or after the modified position may no longer name the same element.
 */
template <class T, size_t N>
class vector {
public:
  /** @brief Element type stored by the fixed-capacity vector. */
  typedef T value_type;
  /** @brief Unsigned size and index type used by the vector. */
  typedef size_t size_type;
  /** @brief Mutable contiguous iterator over live elements. */
  typedef T* iterator;
  /** @brief Const contiguous iterator over live elements. */
  typedef const T* const_iterator;
  /** @brief Mutable reverse iterator over the live contiguous range. */
  typedef reverse_pointer_iterator<T> reverse_iterator;
  /** @brief Const reverse iterator over the live contiguous range. */
  typedef reverse_pointer_iterator<const T> const_reverse_iterator;

  /** @brief Construct an empty vector with no live elements. */
  vector() : size_(0) {}

  /**
   * @brief Copy-construct by appending each live element from `other`.
   * @param other Other object participating in the operation.
   */
  vector(const vector& other) : size_(0) {
    for (size_type i = 0; i != other.size_; ++i) push_back(other[i]);
  }

  /**
   * @brief Cross-capacity copy constructor enabled when the source capacity fits.
   * @param other Other object participating in the operation.
   */
  template <size_t M>
  vector(const vector<T, M>& other, typename enable_if<(M <= N), int>::type = 0) : size_(0) {
    for (size_type i = 0; i != other.size(); ++i) push_back(other[i]);
  }

  /** @brief Destroy all live elements in inline storage. */
  ~vector() { clear(); }

  /**
   * @brief Replace this vector's contents with a copy of `other`.
   * @param other Other object participating in the operation.
   * @return Result described by the function brief.
   */
  vector& operator=(const vector& other) {
    if (this != &other) {
      clear();
      for (size_type i = 0; i != other.size_; ++i) push_back(other[i]);
    }
    return *this;
  }

  /**
   * @brief Cross-capacity assignment enabled when the source capacity fits.
   * @param other Other object participating in the operation.
   * @return Result described by the function brief.
   */
  template <size_t M>
  typename enable_if<(M <= N), vector&>::type operator=(const vector<T, M>& other) {
    clear();
    for (size_type i = 0; i != other.size(); ++i) push_back(other[i]);
    return *this;
  }

  /**
   * @brief Return the current number of live elements.
   * @return The current number of live elements.
   */
  size_type size() const { return size_; }
  /**
   * @brief Return the fixed compile-time maximum element count.
   * @return The fixed compile-time maximum element count.
   */
  size_type max_size() const { return N; }
  /**
   * @brief Return the fixed compile-time storage capacity without requiring an object.
   * @return The fixed compile-time storage capacity without requiring an object.
   */
  static size_type capacity() { return N; }
  /**
   * @brief Report whether no elements are live.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return size_ == 0; }
  /**
   * @brief Report whether `size()` has reached `capacity()`.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool full() const { return size_ == N; }

  /**
   * @brief Access element `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& operator[](size_type i) {
    if (i >= size_) return fail_reference<T>("vector::operator[]"); // LCOV_EXCL_BR_LINE
    return *storage_.ptr(i);
  }
  /**
   * @brief Access element `i` without bounds checking through a const vector.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const T& operator[](size_type i) const {
    if (i >= size_) return fail_reference<const T>("vector::operator[]"); // LCOV_EXCL_BR_LINE
    return *storage_.ptr(i);
  }

  /**
   * @brief Access element `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& at(size_type i) {
    if (i >= size_) return fail_reference<T>("vector::at"); // LCOV_EXCL_BR_LINE
    return (*this)[i];
  }
  /**
   * @brief Const access element `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const T& at(size_type i) const {
    if (i >= size_) return fail_reference<const T>("vector::at"); // LCOV_EXCL_BR_LINE
    return (*this)[i];
  }

  /**
   * @brief Return a pointer to element `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return A pointer to element `i`, or null when out of range.
   */
  T* try_at(size_type i) { return i < size_ ? storage_.ptr(i) : 0; }
  /**
   * @brief Return a const pointer to element `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return A const pointer to element `i`, or null when out of range.
   */
  const T* try_at(size_type i) const { return i < size_ ? storage_.ptr(i) : 0; }

  /**
   * @brief Access the first element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  T& front() {
    if (empty()) return fail_reference<T>("vector::front"); // LCOV_EXCL_BR_LINE
    return (*this)[0];
  }
  /**
   * @brief Const access the first element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  const T& front() const {
    if (empty()) return fail_reference<const T>("vector::front"); // LCOV_EXCL_BR_LINE
    return (*this)[0];
  }
  /**
   * @brief Access the last element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  T& back() {
    if (empty()) return fail_reference<T>("vector::back"); // LCOV_EXCL_BR_LINE
    return (*this)[size_ - 1];
  }
  /**
   * @brief Const access the last element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  const T& back() const {
    if (empty()) return fail_reference<const T>("vector::back"); // LCOV_EXCL_BR_LINE
    return (*this)[size_ - 1];
  }
  /**
   * @brief Return a pointer to the first element, or null when empty.
   * @return A pointer to the first element, or null when empty.
   */
  T* try_front() { return empty() ? 0 : storage_.ptr(0); }
  /**
   * @brief Return a const pointer to the first element, or null when empty.
   * @return A const pointer to the first element, or null when empty.
   */
  const T* try_front() const { return empty() ? 0 : storage_.ptr(0); }
  /**
   * @brief Return a pointer to the last element, or null when empty.
   * @return A pointer to the last element, or null when empty.
   */
  T* try_back() { return empty() ? 0 : storage_.ptr(size_ - 1); }
  /**
   * @brief Return a const pointer to the last element, or null when empty.
   * @return A const pointer to the last element, or null when empty.
   */
  const T* try_back() const { return empty() ? 0 : storage_.ptr(size_ - 1); }

  /**
   * @brief Return a mutable pointer to the inline contiguous storage.
   * @return A mutable pointer to the inline contiguous storage.
   */
  T* data() { return storage_.ptr(0); }
  /**
   * @brief Return a const pointer to the inline contiguous storage.
   * @return A const pointer to the inline contiguous storage.
   */
  const T* data() const { return storage_.ptr(0); }
  /**
   * @brief Return an iterator to the first live element.
   * @return An iterator to the first live element.
   */
  iterator begin() { return iterator(data()); }
  /**
   * @brief Return a const iterator to the first live element.
   * @return A const iterator to the first live element.
   */
  const_iterator begin() const { return const_iterator(data()); }
  /**
   * @brief Return a const iterator to the first live element.
   * @return A const iterator to the first live element.
   */
  const_iterator cbegin() const { return const_iterator(data()); }
  /**
   * @brief Return an iterator one past the final live element.
   * @return An iterator one past the final live element.
   */
  iterator end() { return iterator(data() + size_); }
  /**
   * @brief Return a const iterator one past the final live element.
   * @return A const iterator one past the final live element.
   */
  const_iterator end() const { return const_iterator(data() + size_); }
  /**
   * @brief Return a const iterator one past the final live element.
   * @return A const iterator one past the final live element.
   */
  const_iterator cend() const { return const_iterator(data() + size_); }
  /**
   * @brief Return a mutable reverse iterator to the final live element.
   * @return A mutable reverse iterator to the final live element.
   */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final live element.
   * @return A const reverse iterator to the final live element.
   */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final live element.
   * @return A const reverse iterator to the final live element.
   */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a mutable reverse iterator one before the first live element.
   * @return A mutable reverse iterator one before the first live element.
   */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first live element.
   * @return A const reverse iterator one before the first live element.
   */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first live element.
   * @return A const reverse iterator one before the first live element.
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

  /**
   * @brief Validate that a mutable iterator lies in this vector's live range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const { return it >= data() && it <= data() + size_; }

  /**
   * @brief Validate that a const iterator lies in this vector's live range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(const_iterator it) const { return it >= data() && it <= data() + size_; }

  /**
   * @brief Append one element if capacity remains.
   * @param value Element copied into the next storage slot.
   * @return `true` when appended; `false` when the vector is already full.
   *
   * In `SSTL_PANIC` mode, the full case first calls the panic hook. In RETURN
   * mode it simply returns `false`. The vector is unchanged on failure.
   */
  bool push_back(const T& value) {
    if (full()) {
      handle_error("vector::push_back full");
      return false;
    }
    SSTL_CONSTRUCT_AT(storage_.ptr(size_), value);
    ++size_;
    return true;
  }

  /**
   * @brief Try to append one element without invoking the active error policy.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_push_back(const T& value) {
    if (full()) return false;
    SSTL_CONSTRUCT_AT(storage_.ptr(size_), value);
    ++size_;
    return true;
  }

  /**
   * @brief Change the live size while preserving fixed capacity.
   * @param n Requested number of live elements.
   * @return `true` when the resize completed; `false` when `n` exceeds capacity.
   *
   * Growing constructs default values in newly live slots. Shrinking destroys
   * tail elements. In RETURN mode an over-capacity request leaves the vector
   * unchanged; PANIC mode also reports the policy violation before returning.
   */
  bool resize(size_type n) {
    if (n > N) {
      handle_error("vector::resize full");
      return false;
    }
    while (size_ > n) {
      --size_;
      SSTL_DESTROY_AT(storage_.ptr(size_));
    }
    while (size_ < n) {
      SSTL_CONSTRUCT_AT(storage_.ptr(size_), T());
      ++size_;
    }
    return true;
  }

  /**
   * @brief Change the live size, value-initializing new slots from `value`.
   * @param n Requested number of live elements.
   * @param value Value copied into each newly constructed element.
   * @return `true` when the resize completed; `false` when `n` exceeds capacity.
   */
  bool resize(size_type n, const T& value) {
    if (n > N) {
      handle_error("vector::resize full");
      return false;
    }
    while (size_ > n) {
      --size_;
      SSTL_DESTROY_AT(storage_.ptr(size_));
    }
    while (size_ < n) {
      SSTL_CONSTRUCT_AT(storage_.ptr(size_), value);
      ++size_;
    }
    return true;
  }

  /**
   * @brief Destroy the last element.
   *
   * Empty-vector behavior follows `SSTL_ON_ERROR`; under RETURN the operation
   * leaves the vector unchanged.
   */
  void pop_back() {
    if (empty()) {
      handle_error("vector::pop_back empty");
      return;
    }
    --size_;
    SSTL_DESTROY_AT(storage_.ptr(size_));
  }

  /**
   * @brief Remove the last element and optionally copy it to the caller.
   * @param out Optional destination for the removed value; may be null.
   * @return `true` when an element was removed, `false` when the vector was empty.
   */
  bool try_pop_back(T* out) {
    if (empty()) return false;
    if (out) *out = back();
    pop_back();
    return true;
  }

  /**
   * @brief Insert an element before `pos`.
   * @param pos Iterator position within `[begin(), end()]`.
   * @param value Element copied into the vector.
   * @return Iterator to the inserted element, or `end()` on RETURN-policy failure.
   */
  iterator insert(const_iterator pos, const T& value) {
    size_type index = static_cast<size_type>(pos - begin());
    if (index > size_ || full()) { // LCOV_EXCL_BR_LINE
      handle_error("vector::insert");
      return end();
    }
    insert_fill(index, 1u, value);
    return begin() + index;
  }

  /**
   * @brief Insert `count` copies of `value` before `pos`.
   * @param pos Zero-based logical position.
   * @param count Requested element or character count.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return Result described by the function brief.
   */
  iterator insert(const_iterator pos, size_type count, const T& value) {
    size_type index = static_cast<size_type>(pos - begin());
    if (index > size_ || count > N - size_) {
      handle_error("vector::insert count");
      return end();
    }
    if (count == 0u) return begin() + index;
    insert_fill(index, count, value);
    return begin() + index;
  }

  /**
   * @brief Insert a range of values before `pos`.
   * @param pos Zero-based logical position.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Result described by the function brief.
   */
  template <class InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    return insert_dispatch(pos, first, last, bool_constant<is_integral<InputIt>::value>());
  }

  /**
   * @brief Try to insert one element before `pos` without invoking the active error policy.
   * @param pos Zero-based logical position.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  iterator try_insert(const_iterator pos, const T& value) {
    size_type index = static_cast<size_type>(pos - begin());
    if (index > size_ || full()) return end();
    insert_fill(index, 1u, value);
    return begin() + index;
  }

  /**
   * @brief Try to insert `count` copies before `pos` without invoking the active error policy.
   * @param pos Zero-based logical position.
   * @param count Requested element or character count.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  iterator try_insert(const_iterator pos, size_type count, const T& value) {
    size_type index = static_cast<size_type>(pos - begin());
    if (index > size_ || count > N - size_) return end();
    if (count == 0u) return begin() + index;
    insert_fill(index, count, value);
    return begin() + index;
  }

  /**
   * @brief Try to insert a range before `pos` without invoking the active error policy.
   * @param pos Zero-based logical position.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  template <class InputIt>
  iterator try_insert(const_iterator pos, InputIt first, InputIt last) {
    return try_insert_dispatch(pos, first, last, bool_constant<is_integral<InputIt>::value>());
  }

  /**
   * @brief Remove the element at `pos`.
   * @param pos Iterator naming the element to remove.
   * @return Iterator to the element that shifted into `pos`, or `end()`.
   */
  iterator erase(const_iterator pos) {
    if (!is_valid_iterator(pos) || pos == end()) {
      handle_error("vector::erase iterator");
      return end();
    }
    size_type index = static_cast<size_type>(pos - begin());
    for (size_type i = index; i + 1 < size_; ++i) (*this)[i] = (*this)[i + 1];
    pop_back();
    return begin() + index;
  }

  /**
   * @brief Remove the half-open range `[first,last)`.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Result described by the function brief.
   */
  iterator erase(const_iterator first, const_iterator last) {
    if (!is_valid_iterator(first) || !is_valid_iterator(last) || last < first) {
      handle_error("vector::erase range");
      return end();
    }
    size_type begin_index = static_cast<size_type>(first - begin());
    size_type end_index = static_cast<size_type>(last - begin());
    const size_type count = end_index - begin_index;
    const size_type new_size = size_ - count;
    for (size_type i = begin_index; i + count < size_; ++i) (*this)[i] = (*this)[i + count];
    while (size_ > new_size) {
      pop_back();
    }
    return begin() + begin_index;
  }

  /** @brief Destroy all live elements while preserving fixed capacity. */
  void clear() {
    while (size_ != 0) pop_back();
  }

  /**
   * @brief Replace contents from a raw array when all values fit.
   * @param values Pointer to the first source element.
   * @param count Number of source elements available.
   * @return True when assignment completed; false when `count` exceeds capacity.
   */
  bool assign(const T* values, size_type count) {
    if (count > N) {
      handle_error("vector::assign full");
      return false;
    }
    clear();
    for (size_type i = 0; i != count; ++i) push_back(values[i]);
    return true;
  }

  /**
   * @brief Replace contents with `count` copies of `value`.
   * @param count Requested element or character count.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return Result described by the function brief.
   */
  bool assign(size_type count, const T& value) {
    if (count > N) {
      handle_error("vector::assign full");
      return false;
    }
    clear();
    for (size_type i = 0; i != count; ++i) push_back(value);
    return true;
  }

  /**
   * @brief Replace contents from a range when all values fit.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Result described by the function brief.
   */
  template <class InputIt>
  bool assign(InputIt first, InputIt last) {
    return assign_dispatch(first, last, bool_constant<is_integral<InputIt>::value>());
  }

  /**
   * @brief Exchange contents with another vector of the same type.
   * @param other Other object participating in the operation.
   */
  void swap(vector& other) {
    if (this == &other) return;
    const size_type original_size = size_;
    const size_type other_original_size = other.size_;
    const size_type common = original_size < other_original_size ? original_size : other_original_size;
    for (size_type i = 0u; i != common; ++i) sstl::swap((*this)[i], other[i]);
    if (original_size > other_original_size) {
      for (size_type i = common; i != original_size; ++i) SSTL_CONSTRUCT_AT(other.storage_.ptr(i), (*this)[i]);
      for (size_type i = original_size; i != common; --i) SSTL_DESTROY_AT(storage_.ptr(i - 1u));
    } else {
      for (size_type i = common; i != other_original_size; ++i) SSTL_CONSTRUCT_AT(storage_.ptr(i), other[i]);
      for (size_type i = other_original_size; i != common; --i) SSTL_DESTROY_AT(other.storage_.ptr(i - 1u));
    }
    size_ = other_original_size;
    other.size_ = original_size;
  }

private:
  /** @brief Inline uninitialized storage slots used for element lifetimes. */
  raw_storage<T, N> storage_;
  /** @brief Number of currently constructed elements in `storage_`. */
  size_type size_;

  /**
   * @brief Insert `count` copies of `value` at logical `index` after prechecks pass.
   * @param index Caller-supplied argument used by this operation.
   * @param count Requested element or character count.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   */
  void insert_fill(size_type index, size_type count, const T& value) {
    T value_copy(value);
    const size_type old_size = size_;
    for (size_type i = old_size; i != index; --i) {
      const size_type src = i - 1u;
      const size_type dst = src + count;
      if (dst >= old_size) SSTL_CONSTRUCT_AT(storage_.ptr(dst), (*this)[src]);
      else (*this)[dst] = (*this)[src];
    }
    for (size_type i = 0u; i != count; ++i) {
      const size_type dst = index + i;
      if (dst >= old_size) SSTL_CONSTRUCT_AT(storage_.ptr(dst), value_copy);
      else (*this)[dst] = value_copy;
    }
    size_ = old_size + count;
  }

  /**
   * @brief Count a multipass range without mutating the vector.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Result described by the function brief.
   */
  template <class InputIt>
  size_type range_count(InputIt first, InputIt last) const {
    size_type count = 0u;
    for (; first != last; ++first) ++count;
    return count;
  }

  /**
   * @brief Insert an iterator range after integral-dispatch has selected range semantics.
   * @param pos Zero-based logical position.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Result described by the function brief.
   */
  template <class InputIt>
  iterator insert_range_impl(const_iterator pos, InputIt first, InputIt last) {
    const size_type index = static_cast<size_type>(pos - begin());
    const size_type count = range_count(first, last);
    if (index > size_ || count > N - size_) {
      handle_error("vector::insert range");
      return end();
    }
    if (count == 0u) return begin() + index;
    const size_type old_size = size_;
    for (size_type i = old_size; i != index; --i) {
      const size_type src = i - 1u;
      const size_type dst = src + count;
      if (dst >= old_size) SSTL_CONSTRUCT_AT(storage_.ptr(dst), (*this)[src]);
      else (*this)[dst] = (*this)[src];
    }
    size_type out = index;
    for (; first != last; ++first, ++out) {
      if (out >= old_size) SSTL_CONSTRUCT_AT(storage_.ptr(out), *first);
      else (*this)[out] = *first;
    }
    size_ = old_size + count;
    return begin() + index;
  }

  /**
   * @brief Insert a counted range after all quiet `try_insert` prechecks pass.
   * @param pos Zero-based logical position.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  template <class InputIt>
  iterator try_insert_range_impl(const_iterator pos, InputIt first, InputIt last) {
    const size_type index = static_cast<size_type>(pos - begin());
    const size_type count = range_count(first, last);
    if (index > size_ || count > N - size_) return end();
    if (count == 0u) return begin() + index;
    const size_type old_size = size_;
    for (size_type i = old_size; i != index; --i) {
      const size_type src = i - 1u;
      const size_type dst = src + count;
      if (dst >= old_size) SSTL_CONSTRUCT_AT(storage_.ptr(dst), (*this)[src]);
      else (*this)[dst] = (*this)[src];
    }
    size_type out = index;
    for (; first != last; ++first, ++out) {
      if (out >= old_size) SSTL_CONSTRUCT_AT(storage_.ptr(out), *first);
      else (*this)[out] = *first;
    }
    size_ = old_size + count;
    return begin() + index;
  }

  /**
   * @brief Dispatch integer `insert(pos, n, value)` calls away from range semantics.
   * @param pos Zero-based logical position.
   * @param count Requested element or character count.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return Result described by the function brief.
   */
  template <class Count, class Value>
  iterator insert_dispatch(const_iterator pos, Count count, Value value, bool_constant<true>) {
    return insert(pos, static_cast<size_type>(count), static_cast<T>(value));
  }

  /**
   * @brief Dispatch real iterator ranges to the range insertion implementation.
   * @param pos Zero-based logical position.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Result described by the function brief.
   */
  template <class InputIt>
  iterator insert_dispatch(const_iterator pos, InputIt first, InputIt last, bool_constant<false>) {
    return insert_range_impl(pos, first, last);
  }

  /**
   * @brief Dispatch integer `try_insert(pos, n, value)` calls away from range semantics.
   * @param pos Zero-based logical position.
   * @param count Requested element or character count.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  template <class Count, class Value>
  iterator try_insert_dispatch(const_iterator pos, Count count, Value value, bool_constant<true>) {
    return try_insert(pos, static_cast<size_type>(count), static_cast<T>(value));
  }

  /**
   * @brief Dispatch real iterator ranges to quiet checked range insertion.
   * @param pos Zero-based logical position.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  template <class InputIt>
  iterator try_insert_dispatch(const_iterator pos, InputIt first, InputIt last, bool_constant<false>) {
    return try_insert_range_impl(pos, first, last);
  }

  /**
   * @brief Dispatch integer `assign(n, value)` calls away from range semantics.
   * @param count Requested element or character count.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  template <class Count, class Value>
  bool assign_dispatch(Count count, Value value, bool_constant<true>) {
    return assign(static_cast<size_type>(count), static_cast<T>(value));
  }

  /**
   * @brief Dispatch real iterator ranges to checked range assignment.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  template <class InputIt>
  bool assign_dispatch(InputIt first, InputIt last, bool_constant<false>) {
    const size_type count = range_count(first, last);
    if (count > N) {
      handle_error("vector::assign range full");
      return false;
    }
    clear();
    for (; first != last; ++first) push_back(*first);
    return true;
  }
};

/**
 * @brief Build a fixed-capacity vector from a raw element buffer.
 * @param values Caller-supplied argument used by this operation.
 * @param count Requested element or character count.
 * @return Result described by the function brief.
 */
template <class T, size_t N>
inline vector<T, N> make_vector(const T* values, unsigned count) {
  vector<T, N> out;
  (void)out.assign(values, count);
  return out;
}

/**
 * @brief Build a fixed-capacity vector from a raw C array with explicit capacity.
 * @param values Caller-supplied argument used by this operation.
 * @return Result described by the function brief.
 */
template <class T, size_t N, size_t M>
inline vector<T, N> from_array(const T (&values)[M]) {
  SSTL_STATIC_ASSERT(M <= N, source_array_must_fit_destination_vector);
  vector<T, N> out;
  (void)out.assign(values, M);
  return out;
}

/**
 * @brief Build a vector whose capacity exactly matches the raw C array extent.
 * @param values Caller-supplied argument used by this operation.
 * @return Result described by the function brief.
 */
template <class T, size_t N>
inline vector<T, N> from_array(const T (&values)[N]) {
  vector<T, N> out;
  (void)out.assign(values, N);
  return out;
}

/**
 * @brief Compare two vectors for element-wise equality across capacities.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T, size_t N, size_t M>
inline bool operator==(const vector<T, N>& lhs, const vector<T, M>& rhs) {
  if (lhs.size() != rhs.size()) return false;
  for (unsigned i = 0u; i != lhs.size(); ++i) {
    if (!(lhs[i] == rhs[i])) return false;
  }
  return true;
}

/**
 * @brief Compare two vectors for element-wise inequality across capacities.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T, size_t N, size_t M>
inline bool operator!=(const vector<T, N>& lhs, const vector<T, M>& rhs) {
  return !(lhs == rhs);
}

/**
 * @brief Lexicographically compare two vectors across capacities.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T, size_t N, size_t M>
inline bool operator<(const vector<T, N>& lhs, const vector<T, M>& rhs) {
  const unsigned common = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
  for (unsigned i = 0u; i != common; ++i) {
    if (lhs[i] < rhs[i]) return true;
    if (rhs[i] < lhs[i]) return false;
  }
  return lhs.size() < rhs.size();
}

/**
 * @brief Return true when `lhs` is not lexicographically greater than `rhs`.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when `lhs` is not lexicographically greater than `rhs`.
 */
template <class T, size_t N, size_t M>
inline bool operator<=(const vector<T, N>& lhs, const vector<T, M>& rhs) {
  return !(rhs < lhs);
}

/**
 * @brief Return true when `lhs` is lexicographically greater than `rhs`.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when `lhs` is lexicographically greater than `rhs`.
 */
template <class T, size_t N, size_t M>
inline bool operator>(const vector<T, N>& lhs, const vector<T, M>& rhs) {
  return rhs < lhs;
}

/**
 * @brief Return true when `lhs` is not lexicographically less than `rhs`.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when `lhs` is not lexicographically less than `rhs`.
 */
template <class T, size_t N, size_t M>
inline bool operator>=(const vector<T, N>& lhs, const vector<T, M>& rhs) {
  return !(lhs < rhs);
}

/**
 * @brief Exchange two vectors by delegating to the member swap overload.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class T, size_t N>
inline void swap(vector<T, N>& lhs, vector<T, N>& rhs) {
  lhs.swap(rhs);
}

} // namespace sstl

#endif

