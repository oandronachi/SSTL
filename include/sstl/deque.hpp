/**
 * @file deque.hpp
 * @brief Fixed-capacity ring-buffer deque.
 *
 * Elements live in inline raw storage and are addressed through a logical
 * `[0, size())` view layered on top of a circular physical buffer. Push and pop
 * at either end are constant time and never allocate.
 */
#ifndef SSTL_DEQUE_HPP
/** @def SSTL_DEQUE_HPP
 * @brief Include guard for deque.hpp.
 */
#define SSTL_DEQUE_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

namespace sstl {

/** @brief Fixed-capacity double-ended queue implemented as a circular inline buffer. */
template <class T, size_t N>
class deque {
public:
  /** @brief Stored element type. */
  typedef T value_type;
  /** @brief Unsigned size and index type used by the deque. */
  typedef size_t size_type;

  /** @brief Mutable iterator over the deque's logical sequence. */
  class iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef T value_type;
    /** @brief Mutable reference returned by dereference. */
    typedef T& reference;
    /** @brief Mutable pointer returned by arrow access. */
    typedef T* pointer;
    /** @brief Signed distance type for random-access arithmetic. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef random_access_iterator_tag iterator_category;

    /**
     * @brief Construct an iterator tied to `owner`, logical `index`, and validity epoch.
     * @param owner Caller-supplied argument used by this operation.
     * @param index Caller-supplied argument used by this operation.
     * @param generation Caller-supplied argument used by this operation.
     */
    iterator(deque* owner = 0, size_type index = 0u, unsigned generation = 0u)
      : owner_(owner), index_(index), generation_(generation) {}

    /**
     * @brief Dereference the current logical element.
     * @return Result described by the function brief.
     */
    T& operator*() const { return (*owner_)[index_]; }
    /**
     * @brief Return a pointer to the current logical element.
     * @return A pointer to the current logical element.
     */
    T* operator->() const { return &(*owner_)[index_]; }
    /**
     * @brief Advance to the next logical index.
     * @return Result described by the function brief.
     */
    iterator& operator++() { ++index_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
    /**
     * @brief Move to the previous logical index.
     * @return Result described by the function brief.
     */
    iterator& operator--() { --index_; return *this; }
    /**
     * @brief Post-decrement and return the previous iterator value.
     * @return Result described by the function brief.
     */
    iterator operator--(int) { iterator old(*this); --*this; return old; }
    /**
     * @brief Advance by `n` logical positions.
     * @param n Requested count or size.
     * @return Result described by the function brief.
     */
    iterator& operator+=(difference_type n) { index_ = static_cast<size_type>(static_cast<difference_type>(index_) + n); return *this; }
    /**
     * @brief Move backward by `n` logical positions.
     * @param n Requested count or size.
     * @return Result described by the function brief.
     */
    iterator& operator-=(difference_type n) { return *this += -n; }
    /**
     * @brief Return an iterator offset forward by `n` logical positions.
     * @param n Requested count or size.
     * @return An iterator offset forward by `n` logical positions.
     */
    iterator operator+(difference_type n) const { iterator out(*this); out += n; return out; }
    /**
     * @brief Return an iterator offset forward by `n` logical positions.
     * @param n Requested count or size.
     * @param it Caller-supplied argument used by this operation.
     * @return An iterator offset forward by `n` logical positions.
     */
    friend iterator operator+(difference_type n, const iterator& it) { return it + n; }
    /**
     * @brief Return an iterator offset backward by `n` logical positions.
     * @param n Requested count or size.
     * @return An iterator offset backward by `n` logical positions.
     */
    iterator operator-(difference_type n) const { iterator out(*this); out -= n; return out; }
    /**
     * @brief Return the logical distance from `other` to this iterator.
     * @param other Other object participating in the operation.
     * @return The logical distance from `other` to this iterator.
     */
    difference_type operator-(const iterator& other) const { return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_); }
    /**
     * @brief Access the element `n` positions after this iterator.
     * @param n Requested count or size.
     * @return Result described by the function brief.
     */
    T& operator[](difference_type n) const { return *(*this + n); }
    /**
     * @brief Compare owner and logical index for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const iterator& other) const { return owner_ == other.owner_ && index_ == other.index_; }
    /**
     * @brief Compare owner and logical index for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const iterator& other) const { return !(*this == other); }
    /**
     * @brief Compare logical positions for strict weak ordering within the same deque.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator<(const iterator& other) const { return owner_ == other.owner_ && index_ < other.index_; }
    /**
     * @brief Compare logical positions for less-than-or-equal within the same deque.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator<=(const iterator& other) const { return *this == other || *this < other; }
    /**
     * @brief Compare logical positions for greater-than within the same deque.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator>(const iterator& other) const { return other < *this; }
    /**
     * @brief Compare logical positions for greater-than-or-equal within the same deque.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator>=(const iterator& other) const { return other <= *this; }

  private:
    /** @brief Allow deque specializations to construct and inspect mutable iterators. */
    template <class, size_t> friend class deque;
    /** @brief Owning deque used to translate logical indices to elements. */
    deque* owner_;
    /** @brief Logical index from `begin()` within the current epoch. */
    size_type index_;
    /** @brief Iterator-validity epoch captured when the iterator was created. */
    unsigned generation_;
  };

  /** @brief Const iterator over the deque's logical sequence. */
  class const_iterator {
  public:
    /** @brief Value type exposed by this const iterator. */
    typedef T value_type;
    /** @brief Const reference returned by dereference. */
    typedef const T& reference;
    /** @brief Const pointer returned by arrow access. */
    typedef const T* pointer;
    /** @brief Signed distance type for random-access arithmetic. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef random_access_iterator_tag iterator_category;

    /**
     * @brief Construct a const iterator tied to `owner`, logical `index`, and validity epoch.
     * @param owner Caller-supplied argument used by this operation.
     * @param index Caller-supplied argument used by this operation.
     * @param generation Caller-supplied argument used by this operation.
     */
    const_iterator(const deque* owner = 0, size_type index = 0u, unsigned generation = 0u)
      : owner_(owner), index_(index), generation_(generation) {}
    /**
     * @brief Convert a mutable iterator to a const iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    const_iterator(const iterator& it)
      : owner_(it.owner_), index_(it.index_), generation_(it.generation_) {}

    /**
     * @brief Dereference the current logical element.
     * @return Result described by the function brief.
     */
    const T& operator*() const { return (*owner_)[index_]; }
    /**
     * @brief Return a const pointer to the current logical element.
     * @return A const pointer to the current logical element.
     */
    const T* operator->() const { return &(*owner_)[index_]; }
    /**
     * @brief Advance to the next logical index.
     * @return Result described by the function brief.
     */
    const_iterator& operator++() { ++index_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
    /**
     * @brief Move to the previous logical index.
     * @return Result described by the function brief.
     */
    const_iterator& operator--() { --index_; return *this; }
    /**
     * @brief Post-decrement and return the previous iterator value.
     * @return Result described by the function brief.
     */
    const_iterator operator--(int) { const_iterator old(*this); --*this; return old; }
    /**
     * @brief Advance by `n` logical positions.
     * @param n Requested count or size.
     * @return Result described by the function brief.
     */
    const_iterator& operator+=(difference_type n) { index_ = static_cast<size_type>(static_cast<difference_type>(index_) + n); return *this; }
    /**
     * @brief Move backward by `n` logical positions.
     * @param n Requested count or size.
     * @return Result described by the function brief.
     */
    const_iterator& operator-=(difference_type n) { return *this += -n; }
    /**
     * @brief Return an iterator offset forward by `n` logical positions.
     * @param n Requested count or size.
     * @return An iterator offset forward by `n` logical positions.
     */
    const_iterator operator+(difference_type n) const { const_iterator out(*this); out += n; return out; }
    /**
     * @brief Return an iterator offset forward by `n` logical positions.
     * @param n Requested count or size.
     * @param it Caller-supplied argument used by this operation.
     * @return An iterator offset forward by `n` logical positions.
     */
    friend const_iterator operator+(difference_type n, const const_iterator& it) { return it + n; }
    /**
     * @brief Return an iterator offset backward by `n` logical positions.
     * @param n Requested count or size.
     * @return An iterator offset backward by `n` logical positions.
     */
    const_iterator operator-(difference_type n) const { const_iterator out(*this); out -= n; return out; }
    /**
     * @brief Return the logical distance from `other` to this iterator.
     * @param other Other object participating in the operation.
     * @return The logical distance from `other` to this iterator.
     */
    difference_type operator-(const const_iterator& other) const { return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_); }
    /**
     * @brief Access the element `n` positions after this iterator.
     * @param n Requested count or size.
     * @return Result described by the function brief.
     */
    const T& operator[](difference_type n) const { return *(*this + n); }
    /**
     * @brief Compare owner and logical index for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const const_iterator& other) const { return owner_ == other.owner_ && index_ == other.index_; }
    /**
     * @brief Compare owner and logical index for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const const_iterator& other) const { return !(*this == other); }
    /**
     * @brief Compare logical positions for strict weak ordering within the same deque.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator<(const const_iterator& other) const { return owner_ == other.owner_ && index_ < other.index_; }
    /**
     * @brief Compare logical positions for less-than-or-equal within the same deque.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator<=(const const_iterator& other) const { return *this == other || *this < other; }
    /**
     * @brief Compare logical positions for greater-than within the same deque.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator>(const const_iterator& other) const { return other < *this; }
    /**
     * @brief Compare logical positions for greater-than-or-equal within the same deque.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator>=(const const_iterator& other) const { return other <= *this; }

  private:
    /** @brief Allow deque specializations to construct and inspect const iterators. */
    template <class, size_t> friend class deque;
    /** @brief Owning deque used to translate logical indices to elements. */
    const deque* owner_;
    /** @brief Logical index from `begin()` within the current epoch. */
    size_type index_;
    /** @brief Iterator-validity epoch captured when the iterator was created. */
    unsigned generation_;
  };

  /** @brief Mutable reverse iterator over the logical deque sequence. */
  typedef reverse_iterator_adaptor<iterator> reverse_iterator;
  /** @brief Const reverse iterator over the logical deque sequence. */
  typedef reverse_iterator_adaptor<const_iterator> const_reverse_iterator;

  /** @brief Construct an empty deque with a valid initial iterator epoch. */
  deque() : head_(0u), size_(0u), generation_(1u) {}
  /**
   * @brief Copy-construct by pushing each logical element from `other`.
   * @param other Other object participating in the operation.
   */
  deque(const deque& other) : head_(0u), size_(0u), generation_(1u) {
    for (size_type i = 0u; i != other.size_; ++i) push_back(other[i]);
  }
  /** @brief Destroy all live elements. */
  ~deque() { clear(); }

  /**
   * @brief Replace this deque's contents with a copy of `other`.
   * @param other Other object participating in the operation.
   * @return Result described by the function brief.
   */
  deque& operator=(const deque& other) {
    if (this != &other) {
      clear();
      for (size_type i = 0u; i != other.size_; ++i) push_back(other[i]);
    }
    return *this;
  }

  /**
   * @brief Return the number of live elements.
   * @return The number of live elements.
   */
  size_type size() const { return size_; }
  /**
   * @brief Return the largest number of elements this fixed-capacity deque can hold.
   * @return The largest number of elements this fixed-capacity deque can hold.
   */
  size_type max_size() const { return N; }
  /**
   * @brief Return the fixed compile-time capacity without requiring an object.
   * @return The fixed compile-time capacity without requiring an object.
   */
  static size_type capacity() { return N; }
  /**
   * @brief Report whether no elements are live.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return size_ == 0u; }
  /**
   * @brief Report whether the deque is at fixed capacity.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool full() const { return size_ == N; }

  /**
   * @brief Access logical element `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& operator[](size_type i) {
    if (i >= size_) return fail_reference<T>("deque::operator[]"); // LCOV_EXCL_BR_LINE
    return *storage_.ptr(physical(i));
  }
  /**
   * @brief Const access logical element `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const T& operator[](size_type i) const {
    if (i >= size_) return fail_reference<const T>("deque::operator[]"); // LCOV_EXCL_BR_LINE
    return *storage_.ptr(physical(i));
  }

  /**
   * @brief Access logical element `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  T& at(size_type i) {
    if (i >= size_) return fail_reference<T>("deque::at"); // LCOV_EXCL_BR_LINE
    return (*this)[i];
  }
  /**
   * @brief Const access logical element `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const T& at(size_type i) const {
    if (i >= size_) return fail_reference<const T>("deque::at"); // LCOV_EXCL_BR_LINE
    return (*this)[i];
  }
  /**
   * @brief Return a pointer to logical element `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return Pointer to logical element `i`, or null when out of range.
   */
  T* try_at(size_type i) { return i < size_ ? storage_.ptr(physical(i)) : 0; }
  /**
   * @brief Return a const pointer to logical element `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return Const pointer to logical element `i`, or null when out of range.
   */
  const T* try_at(size_type i) const { return i < size_ ? storage_.ptr(physical(i)) : 0; }

  /**
   * @brief Access the front element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  T& front() {
    if (empty()) return fail_reference<T>("deque::front empty"); // LCOV_EXCL_BR_LINE
    return (*this)[0u];
  }
  /**
   * @brief Const access the front element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  const T& front() const {
    if (empty()) return fail_reference<const T>("deque::front empty"); // LCOV_EXCL_BR_LINE
    return (*this)[0u];
  }
  /**
   * @brief Access the back element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  T& back() {
    if (empty()) return fail_reference<T>("deque::back empty"); // LCOV_EXCL_BR_LINE
    return (*this)[size_ - 1u];
  }
  /**
   * @brief Const access the back element, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  const T& back() const {
    if (empty()) return fail_reference<const T>("deque::back empty"); // LCOV_EXCL_BR_LINE
    return (*this)[size_ - 1u];
  }
  /**
   * @brief Return a pointer to the front element, or null when empty.
   * @return A pointer to the front element, or null when empty.
   */
  T* try_front() { return empty() ? 0 : storage_.ptr(physical(0u)); }
  /**
   * @brief Return a const pointer to the front element, or null when empty.
   * @return A const pointer to the front element, or null when empty.
   */
  const T* try_front() const { return empty() ? 0 : storage_.ptr(physical(0u)); }
  /**
   * @brief Return a pointer to the back element, or null when empty.
   * @return A pointer to the back element, or null when empty.
   */
  T* try_back() { return empty() ? 0 : storage_.ptr(physical(size_ - 1u)); }
  /**
   * @brief Return a const pointer to the back element, or null when empty.
   * @return A const pointer to the back element, or null when empty.
   */
  const T* try_back() const { return empty() ? 0 : storage_.ptr(physical(size_ - 1u)); }

  /**
   * @brief Return an iterator to the first logical element.
   * @return An iterator to the first logical element.
   */
  iterator begin() { return iterator(this, 0u, generation_); }
  /**
   * @brief Return an iterator one past the final logical element.
   * @return An iterator one past the final logical element.
   */
  iterator end() { return iterator(this, size_, generation_); }
  /**
   * @brief Return a const iterator to the first logical element.
   * @return A const iterator to the first logical element.
   */
  const_iterator begin() const { return const_iterator(this, 0u, generation_); }
  /**
   * @brief Return a const iterator to the first logical element.
   * @return A const iterator to the first logical element.
   */
  const_iterator cbegin() const { return begin(); }
  /**
   * @brief Return a const iterator one past the final logical element.
   * @return A const iterator one past the final logical element.
   */
  const_iterator end() const { return const_iterator(this, size_, generation_); }
  /**
   * @brief Return a const iterator one past the final logical element.
   * @return A const iterator one past the final logical element.
   */
  const_iterator cend() const { return end(); }
  /**
   * @brief Return a mutable reverse iterator to the final logical element.
   * @return A mutable reverse iterator to the final logical element.
   */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final logical element.
   * @return A const reverse iterator to the final logical element.
   */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final logical element.
   * @return A const reverse iterator to the final logical element.
   */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a mutable reverse iterator one before the first logical element.
   * @return A mutable reverse iterator one before the first logical element.
   */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first logical element.
   * @return A const reverse iterator one before the first logical element.
   */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first logical element.
   * @return A const reverse iterator one before the first logical element.
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

  /**
   * @brief Append an element at the logical back.
   * @param x Element value supplied by the caller.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool push_back(const T& x) {
    if (full()) {
      handle_error("deque::push_back full");
      return false;
    }
    SSTL_CONSTRUCT_AT(storage_.ptr(physical(size_)), x);
    ++size_;
    invalidate_iterators();
    return true;
  }

  /**
   * @brief Try to append at the logical back without invoking the active error policy.
   * @param x Element value supplied by the caller.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_push_back(const T& x) {
    if (full()) return false;
    return push_back(x);
  }

  /**
   * @brief Prepend an element at the logical front.
   * @param x Element value supplied by the caller.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool push_front(const T& x) {
    if (full()) {
      handle_error("deque::push_front full");
      return false;
    }
    head_ = N == 0u ? 0u : (head_ + N - 1u) % N;
    SSTL_CONSTRUCT_AT(storage_.ptr(head_), x);
    ++size_;
    invalidate_iterators();
    return true;
  }

  /**
   * @brief Try to prepend at the logical front without invoking the active error policy.
   * @param x Element value supplied by the caller.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_push_front(const T& x) {
    if (full()) return false;
    return push_front(x);
  }

  /**
   * @brief Insert an element before `pos`.
   * @param pos Logical position in `[begin(), end()]`.
   * @param value Element copied into the deque.
   * @return Iterator to the inserted element, or `end()` when insertion fails.
   */
  iterator insert(const_iterator pos, const T& value) {
    const size_type index = pos.index_;
    if (!is_valid_iterator(pos) || index > size_ || full()) {
      handle_error("deque::insert");
      return end();
    }
    T value_copy(value);
    if (index == 0u) {
      return push_front(value_copy) ? begin() : end();
    }
    if (index == size_) {
      if (!push_back(value_copy)) return end();
      return begin() + static_cast<int>(index);
    }
    const size_type old_size = size_;
    if (!push_back((*this)[old_size - 1u])) return end(); // LCOV_EXCL_BR_LINE
    for (size_type i = old_size - 1u; i != index; --i) {
      (*this)[i] = (*this)[i - 1u];
    }
    (*this)[index] = value_copy;
    return begin() + static_cast<int>(index);
  }

  /**
   * @brief Try to insert an element before `pos` without invoking the active error policy.
   * @param pos Logical position in `[begin(), end()]`.
   * @param value Element copied into the deque.
   * @return Iterator to the inserted element, or `end()` when insertion fails.
   */
  iterator try_insert(const_iterator pos, const T& value) {
    const size_type index = pos.index_;
    if (!is_valid_iterator(pos) || index > size_ || full()) return end();
    return insert(pos, value);
  }

  /**
   * @brief Insert `count` copies of `value` before `pos`.
   * @param pos Logical position in `[begin(), end()]`.
   * @param count Requested element count.
   * @param value Element copied into the deque.
   * @return Iterator to the first inserted element, or `end()` when insertion fails.
   */
  iterator insert(const_iterator pos, size_type count, const T& value) {
    const size_type index = pos.index_;
    if (!is_valid_iterator(pos) || index > size_ || count > N - size_) {
      handle_error("deque::insert count");
      return end();
    }
    if (count == 0u) return begin() + static_cast<int>(index);
    for (size_type i = 0u; i != count; ++i) {
      iterator inserted = insert(begin() + static_cast<int>(index + i), value);
      if (inserted == end()) return end(); // LCOV_EXCL_BR_LINE
    }
    return begin() + static_cast<int>(index);
  }

  /**
   * @brief Try to insert `count` copies of `value` before `pos` without invoking the active error policy.
   * @param pos Logical position in `[begin(), end()]`.
   * @param count Requested element count.
   * @param value Element copied into the deque.
   * @return Iterator to the first inserted element, or `end()` when insertion fails.
   */
  iterator try_insert(const_iterator pos, size_type count, const T& value) {
    const size_type index = pos.index_;
    if (!is_valid_iterator(pos) || index > size_ || count > N - size_) return end();
    if (count == 0u) return begin() + static_cast<int>(index);
    for (size_type i = 0u; i != count; ++i) {
      iterator inserted = insert(begin() + static_cast<int>(index + i), value);
      if (inserted == end()) return end(); // LCOV_EXCL_BR_LINE
    }
    return begin() + static_cast<int>(index);
  }

  /**
   * @brief Insert a range of values before `pos`.
   * @param pos Logical position in `[begin(), end()]`.
   * @param first Start of the half-open input range.
   * @param last One-past-end of the input range.
   * @return Iterator to the first inserted element, or `end()` when insertion fails.
   */
  template <class InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    return insert_dispatch(pos, first, last, bool_constant<is_integral<InputIt>::value>());
  }

  /**
   * @brief Try to insert a range before `pos` without invoking the active error policy.
   * @param pos Logical position in `[begin(), end()]`.
   * @param first Start of the half-open input range.
   * @param last One-past-end of the input range.
   * @return Iterator to the first inserted element, or `end()` when insertion fails.
   */
  template <class InputIt>
  iterator try_insert(const_iterator pos, InputIt first, InputIt last) {
    return try_insert_dispatch(pos, first, last, bool_constant<is_integral<InputIt>::value>());
  }

  /** @brief Remove the logical back element. */
  void pop_back() {
    if (empty()) {
      handle_error("deque::pop_back empty");
      return;
    }
    SSTL_DESTROY_AT(storage_.ptr(physical(size_ - 1u)));
    --size_;
    if (empty()) head_ = 0u;
    invalidate_iterators();
  }

  /** @brief Remove the logical front element. */
  void pop_front() {
    if (empty()) {
      handle_error("deque::pop_front empty");
      return;
    }
    SSTL_DESTROY_AT(storage_.ptr(head_));
    head_ = N == 0u ? 0u : (head_ + 1u) % N;
    --size_;
    if (empty()) head_ = 0u;
    invalidate_iterators();
  }

  /**
   * @brief Remove the back element and optionally copy it to `out`.
   * @param out Caller-provided destination for produced values.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_pop_back(T* out) {
    if (empty()) return false;
    if (out) *out = back();
    pop_back();
    return true;
  }

  /**
   * @brief Remove the front element and optionally copy it to `out`.
   * @param out Caller-provided destination for produced values.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_pop_front(T* out) {
    if (empty()) return false;
    if (out) *out = front();
    pop_front();
    return true;
  }

  /**
   * @brief Erase the element named by `pos`.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  iterator erase(const_iterator pos) {
    const size_type index = pos.index_;
    if (!is_valid_iterator(pos) || index >= size_) {
      handle_error("deque::erase");
      return end();
    }
    if (index == 0u) {
      pop_front();
      return begin();
    }
    if (index + 1u == size_) {
      pop_back();
      return end();
    }
    for (size_type i = index; i + 1u < size_; ++i) {
      (*this)[i] = (*this)[i + 1u];
    }
    pop_back();
    return begin() + static_cast<int>(index);
  }

  /**
   * @brief Erase the half-open logical range `[first,last)`.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Iterator to the element that followed the erased range.
   */
  iterator erase(const_iterator first, const_iterator last) {
    if (!is_valid_iterator(first) || !is_valid_iterator(last) || first.index_ > last.index_ || last.index_ > size_) {
      handle_error("deque::erase range");
      return end();
    }
    const size_type index = first.index_;
    const size_type count = last.index_ - first.index_;
    for (size_type i = 0u; i != count; ++i) {
      (void)erase(begin() + static_cast<int>(index));
    }
    return begin() + static_cast<int>(index);
  }

  /**
   * @brief Change the live size, default-constructing new elements as needed.
   * @param n Requested count or size.
   * @return Result described by the function brief.
   */
  bool resize(size_type n) {
    if (n > N) {
      handle_error("deque::resize full");
      return false;
    }
    while (size_ > n) pop_back();
    while (size_ < n) {
      if (!push_back(T())) return false; // LCOV_EXCL_BR_LINE
    }
    return true;
  }

  /**
   * @brief Change the live size, copying `value` into newly constructed slots.
   * @param n Requested count or size.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return Result described by the function brief.
   */
  bool resize(size_type n, const T& value) {
    if (n > N) {
      handle_error("deque::resize full");
      return false;
    }
    while (size_ > n) pop_back();
    while (size_ < n) {
      if (!push_back(value)) return false; // LCOV_EXCL_BR_LINE
    }
    return true;
  }

  /** @brief Destroy all live elements and reset logical head to zero. */
  void clear() {
    while (!empty()) pop_back();
    head_ = 0u;
  }

  /**
   * @brief Exchange contents with another deque of the same type.
   * @param other Other object participating in the operation.
   */
  void swap(deque& other) {
    if (this == &other) return;
    const size_type original_size = size_;
    const size_type other_original_size = other.size_;
    const size_type common = original_size < other_original_size ? original_size : other_original_size;
    for (size_type i = 0u; i != common; ++i) sstl::swap((*this)[i], other[i]);
    if (original_size > other_original_size) {
      for (size_type i = common; i != original_size; ++i) other.push_back((*this)[i]);
      while (size_ != common) pop_back();
    } else {
      for (size_type i = common; i != other_original_size; ++i) push_back(other[i]);
      while (other.size_ != common) other.pop_back();
    }
  }

  /**
   * @brief Validate a mutable iterator against this deque and its current epoch.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const {
    return it.owner_ == this && it.generation_ == generation_ && it.index_ <= size_;
  }

  /**
   * @brief Validate a const iterator against this deque and its current epoch.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(const_iterator it) const {
    return it.owner_ == this && it.generation_ == generation_ && it.index_ <= size_;
  }

private:
  /** @brief Inline raw storage for all possible deque elements. */
  raw_storage<T, N> storage_;
  /** @brief Physical index of logical element zero. */
  size_type head_;
  /** @brief Number of currently live elements. */
  size_type size_;
  /** @brief Monotonic epoch used to invalidate outstanding iterators after mutation. */
  unsigned generation_;

  /**
   * @brief Translate a logical index into the circular-buffer physical slot.
   * @param logical Caller-supplied argument used by this operation.
   * @return Result described by the function brief.
   */
  size_type physical(size_type logical) const {
    return N == 0u ? 0u : (head_ + logical) % N;
  }

  /** @brief Advance the iterator-validity epoch after structural mutation. */
  void invalidate_iterators() {
    ++generation_;
    if (generation_ == 0u) generation_ = 1u; // LCOV_EXCL_BR_LINE
  }

  /**
   * @brief Count a multipass input range before mutating the deque.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Number of elements in the range.
   */
  template <class InputIt>
  size_type range_count(InputIt first, InputIt last) const {
    size_type count = 0u;
    for (; first != last; ++first) ++count;
    return count;
  }

  /**
   * @brief Dispatch integer insert calls to counted insertion.
   * @param pos Logical insertion position.
   * @param count Requested element count.
   * @param value Element copied into the deque.
   * @return Iterator to the first inserted element, or `end()` on failure.
   */
  template <class Count, class Value>
  iterator insert_dispatch(const_iterator pos, Count count, Value value, bool_constant<true>) {
    return insert(pos, static_cast<size_type>(count), static_cast<T>(value));
  }

  /**
   * @brief Dispatch iterator insert calls to range insertion.
   * @param pos Logical insertion position.
   * @param first Start of the half-open range.
   * @param last One-past-end of the half-open range.
   * @return Iterator to the first inserted element, or `end()` on failure.
   */
  template <class InputIt>
  iterator insert_dispatch(const_iterator pos, InputIt first, InputIt last, bool_constant<false>) {
    const size_type index = pos.index_;
    const size_type count = range_count(first, last);
    if (!is_valid_iterator(pos) || index > size_ || count > N - size_) {
      handle_error("deque::insert range");
      return end();
    }
    if (count == 0u) return begin() + static_cast<int>(index);
    size_type offset = 0u;
    for (; first != last; ++first, ++offset) {
      iterator inserted = insert(begin() + static_cast<int>(index + offset), *first);
      if (inserted == end()) return end(); // LCOV_EXCL_BR_LINE
    }
    return begin() + static_cast<int>(index);
  }

  /** @brief Dispatch integer quiet insert calls to counted insertion. */
  template <class Count, class Value>
  iterator try_insert_dispatch(const_iterator pos, Count count, Value value, bool_constant<true>) {
    return try_insert(pos, static_cast<size_type>(count), static_cast<T>(value));
  }

  /** @brief Dispatch iterator quiet insert calls to range insertion. */
  template <class InputIt>
  iterator try_insert_dispatch(const_iterator pos, InputIt first, InputIt last, bool_constant<false>) {
    const size_type index = pos.index_;
    const size_type count = range_count(first, last);
    if (!is_valid_iterator(pos) || index > size_ || count > N - size_) return end();
    if (count == 0u) return begin() + static_cast<int>(index);
    size_type offset = 0u;
    for (; first != last; ++first, ++offset) {
      iterator inserted = insert(begin() + static_cast<int>(index + offset), *first);
      if (inserted == end()) return end(); // LCOV_EXCL_BR_LINE
    }
    return begin() + static_cast<int>(index);
  }
};

/**
 * @brief Exchange two deques by delegating to the member swap overload.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class T, size_t N>
inline void swap(deque<T, N>& lhs, deque<T, N>& rhs) {
  lhs.swap(rhs);
}

/**
 * @brief Compare two deques for element-wise equality across capacities.
 * @param lhs First deque.
 * @param rhs Second deque.
 * @return `true` when both logical sequences are equal.
 */
template <class T, size_t N, size_t M>
inline bool operator==(const deque<T, N>& lhs, const deque<T, M>& rhs) {
  if (lhs.size() != rhs.size()) return false;
  for (size_t i = 0u; i != lhs.size(); ++i) {
    if (!(lhs[i] == rhs[i])) return false;
  }
  return true;
}

/** @brief Compare two deques for inequality. */
template <class T, size_t N, size_t M>
inline bool operator!=(const deque<T, N>& lhs, const deque<T, M>& rhs) { return !(lhs == rhs); }

/** @brief Lexicographically compare two deque sequences. */
template <class T, size_t N, size_t M>
inline bool operator<(const deque<T, N>& lhs, const deque<T, M>& rhs) {
  const size_t common = lhs.size() < rhs.size() ? lhs.size() : rhs.size();
  for (size_t i = 0u; i != common; ++i) {
    if (lhs[i] < rhs[i]) return true;
    if (rhs[i] < lhs[i]) return false;
  }
  return lhs.size() < rhs.size();
}

/** @brief Return true when `lhs` is not lexicographically greater than `rhs`. */
template <class T, size_t N, size_t M>
inline bool operator<=(const deque<T, N>& lhs, const deque<T, M>& rhs) { return !(rhs < lhs); }

/** @brief Return true when `lhs` is lexicographically greater than `rhs`. */
template <class T, size_t N, size_t M>
inline bool operator>(const deque<T, N>& lhs, const deque<T, M>& rhs) { return rhs < lhs; }

/** @brief Return true when `lhs` is not lexicographically less than `rhs`. */
template <class T, size_t N, size_t M>
inline bool operator>=(const deque<T, N>& lhs, const deque<T, M>& rhs) { return !(lhs < rhs); }

} // namespace sstl

#endif

