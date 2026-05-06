/**
 * @file span.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_SPAN_HPP
/** @def SSTL_SPAN_HPP
 * @brief Include guard for span.hpp.
 */
#define SSTL_SPAN_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "type_traits.hpp"

namespace sstl {

/** @brief Forward declaration used by span constructors without including array.hpp. */
template <class T, size_t N> class array;
/** @brief Forward declaration used by span constructors without including vector.hpp. */
template <class T, size_t N> class vector;

/** @brief Non-owning view over a contiguous mutable sequence. */
template <class T>
class span {
public:
  /** @brief Element type exposed by the span view. */
  typedef T value_type;
  /** @brief Unsigned size type used for span extents and indexes. */
  typedef size_t size_type;
  /** @brief Mutable contiguous iterator type. */
  typedef T* iterator;
  /** @brief Const contiguous iterator type. */
  typedef const T* const_iterator;
  /** @brief Mutable reverse pointer iterator type. */
  typedef reverse_pointer_iterator<T> reverse_iterator;
  /** @brief Const reverse pointer iterator type. */
  typedef reverse_pointer_iterator<const T> const_reverse_iterator;

  /** @brief Construct an empty span. */
  span() : ptr_(0), size_(0) {}
  /**
   * @brief Construct a span from a pointer and element count.
   * @param p Erased storage pointer.
   * @param n Requested count or size.
   */
  span(T* p, size_type n) : ptr_(p), size_(n) {}
  /**
   * @brief Construct a span from a half-open pointer range.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   */
  span(T* first, T* last) : ptr_(first), size_(static_cast<size_type>(last - first)) {}
  /**
   * @brief Construct a span over a mutable C array.
   * @param a First operand or first range start.
   */
  template <size_t N>
  span(T (&a)[N]) : ptr_(a), size_(N) {}
  /**
   * @brief Construct a const-element span over a C array.
   * @param a First operand or first range start.
   */
  template <class U, size_t N>
  span(const U (&a)[N]) : ptr_(a), size_(N) {}
  /**
   * @brief Construct a span over an SSTL array's contiguous storage.
   * @param a First operand or first range start.
   */
  template <size_t N>
  span(array<T, N>& a) : ptr_(a.data()), size_(N) {}
  /**
   * @brief Construct a const-element span over an SSTL array's contiguous storage.
   * @param a First operand or first range start.
   */
  template <class U, size_t N>
  span(const array<U, N>& a) : ptr_(a.data()), size_(N) {}
  /**
   * @brief Construct a span over the live prefix of an SSTL vector's contiguous storage.
   * @param v Generated object or variant instance.
   */
  template <size_t N>
  span(vector<T, N>& v) : ptr_(v.data()), size_(v.size()) {}
  /**
   * @brief Construct a const-element span over the live prefix of an SSTL vector's contiguous storage.
   * @param v Generated object or variant instance.
   */
  template <class U, size_t N>
  span(const vector<U, N>& v) : ptr_(v.data()), size_(v.size()) {}

  /**
   * @brief Return the number of elements in the viewed range.
   * @return The number of elements in the viewed range.
   */
  size_type size() const { return size_; }
  /**
   * @brief Return the number of bytes covered by the viewed range.
   * @return The number of bytes covered by the viewed range.
   */
  size_type size_bytes() const { return size_ * sizeof(T); }
  /**
   * @brief Report whether the viewed range is empty.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return size_ == 0; }
  /**
   * @brief Return the mutable pointer to the first viewed element.
   * @return The mutable pointer to the first viewed element.
   */
  T* data() { return ptr_; }
  /**
   * @brief Return the const pointer to the first viewed element.
   * @return The const pointer to the first viewed element.
   */
  T* data() const { return ptr_; }
  /**
   * @brief Access element `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& operator[](size_type i) {
    if (i >= size_) return fail_reference<T>("span::operator[]"); // LCOV_EXCL_BR_LINE
    return ptr_[i];
  }
  /**
   * @brief Access element `i` without bounds checking through a const span.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& operator[](size_type i) const {
    if (i >= size_) return fail_reference<T>("span::operator[]"); // LCOV_EXCL_BR_LINE
    return ptr_[i];
  }
  /**
   * @brief Access element `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& at(size_type i) {
    if (i >= size_) return fail_reference<T>("span::at"); // LCOV_EXCL_BR_LINE
    return ptr_[i];
  }
  /**
   * @brief Const access element `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& at(size_type i) const {
    if (i >= size_) return fail_reference<T>("span::at"); // LCOV_EXCL_BR_LINE
    return ptr_[i];
  }
  /**
   * @brief Access the first viewed element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  T& front() {
    if (empty()) return fail_reference<T>("span::front"); // LCOV_EXCL_BR_LINE
    return ptr_[0];
  }
  /**
   * @brief Const access the first viewed element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  T& front() const {
    if (empty()) return fail_reference<T>("span::front"); // LCOV_EXCL_BR_LINE
    return ptr_[0];
  }
  /**
   * @brief Access the final viewed element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  T& back() {
    if (empty()) return fail_reference<T>("span::back"); // LCOV_EXCL_BR_LINE
    return ptr_[size_ - 1u];
  }
  /**
   * @brief Const access the final viewed element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  T& back() const {
    if (empty()) return fail_reference<T>("span::back"); // LCOV_EXCL_BR_LINE
    return ptr_[size_ - 1u];
  }
  /**
   * @brief Return a pointer to element `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return A pointer to element `i`, or null when out of range.
   */
  T* try_at(size_type i) const { return i < size_ ? ptr_ + i : 0; }
  /**
   * @brief Return a pointer to the first viewed element, or null when empty.
   * @return A pointer to the first viewed element, or null when empty.
   */
  T* try_front() const { return empty() ? 0 : ptr_; }
  /**
   * @brief Return a pointer to the final viewed element, or null when empty.
   * @return A pointer to the final viewed element, or null when empty.
   */
  T* try_back() const { return empty() ? 0 : ptr_ + size_ - 1u; }
  /**
   * @brief Return an iterator to the first viewed element.
   * @return An iterator to the first viewed element.
   */
  iterator begin() { return iterator(ptr_); }
  /**
   * @brief Return a const iterator to the first viewed element.
   * @return A const iterator to the first viewed element.
   */
  iterator begin() const { return iterator(ptr_); }
  /**
   * @brief Return a const iterator to the first viewed element.
   * @return A const iterator to the first viewed element.
   */
  const_iterator cbegin() const { return const_iterator(ptr_); }
  /**
   * @brief Return an iterator one past the final viewed element.
   * @return An iterator one past the final viewed element.
   */
  iterator end() { return ptr_ ? iterator(ptr_ + size_) : iterator(ptr_); }
  /**
   * @brief Return a const iterator one past the final viewed element.
   * @return A const iterator one past the final viewed element.
   */
  iterator end() const { return ptr_ ? iterator(ptr_ + size_) : iterator(ptr_); }
  /**
   * @brief Return a const iterator one past the final viewed element.
   * @return A const iterator one past the final viewed element.
   */
  const_iterator cend() const { return ptr_ ? const_iterator(ptr_ + size_) : const_iterator(ptr_); }
  /**
   * @brief Return a mutable reverse iterator to the final viewed element.
   * @return A mutable reverse iterator to the final viewed element.
   */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final viewed element.
   * @return A const reverse iterator to the final viewed element.
   */
  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final viewed element.
   * @return A const reverse iterator to the final viewed element.
   */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a mutable reverse iterator one before the first viewed element.
   * @return A mutable reverse iterator one before the first viewed element.
   */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first viewed element.
   * @return A const reverse iterator one before the first viewed element.
   */
  reverse_iterator rend() const { return reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first viewed element.
   * @return A const reverse iterator one before the first viewed element.
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Validate that a mutable iterator lies in this span's viewed range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const { return ptr_ ? (it >= ptr_ && it <= ptr_ + size_) : it == 0; }
  /**
   * @brief Validate that a const iterator lies in this span's viewed range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  template <class It>
  typename enable_if<is_same<It, const_iterator>::value && !is_same<iterator, const_iterator>::value, bool>::type
  is_valid_iterator(It it) const { return ptr_ ? (it >= ptr_ && it <= ptr_ + size_) : it == 0; }
  /**
   * @brief Return a span over the first `n` elements, clamping to the current size.
   * @param n Requested count or size.
   * @return A span over the first `n` elements, clamping to the current size.
   */
  span first(size_type n) const { return span(ptr_, n < size_ ? n : size_); }
  /**
   * @brief Return a span over the last `n` elements, clamping to the current size.
   * @param n Requested count or size.
   * @return A span over the last `n` elements, clamping to the current size.
   */
  span last(size_type n) const {
    size_type count = n < size_ ? n : size_;
    if (!ptr_) return span(ptr_, size_type());
    return span(ptr_ + size_ - count, count);
  }
  /**
   * @brief Return a span over a suffix subrange, clamping offset and count to the current size.
   * @param offset Caller-supplied argument used by this operation.
   * @param count Requested element or character count.
   * @return A span over a suffix subrange, clamping offset and count to the current size.
   */
  span subspan(size_type offset, size_type count) const {
    size_type start = offset < size_ ? offset : size_;
    size_type available = size_ - start;
    size_type n = count < available ? count : available;
    if (!ptr_) return span(ptr_, size_type());
    return span(ptr_ + start, n);
  }
  /**
   * @brief Return a span over the suffix beginning at `offset`, clamping offset to the current size.
   * @param offset Caller-supplied argument used by this operation.
   * @return A span over the suffix beginning at `offset`, clamping offset to the current size.
   */
  span subspan(size_type offset) const {
    size_type start = offset < size_ ? offset : size_;
    if (!ptr_) return span(ptr_, size_type());
    return span(ptr_ + start, size_ - start);
  }

private:
  /** @brief Pointer to the first viewed element, or null for an empty default span. */
  T* ptr_;
  /** @brief Number of elements visible through the span. */
  size_type size_;
};

} // namespace sstl

#endif

