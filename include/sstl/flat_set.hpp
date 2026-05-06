/**
 * @file flat_set.hpp
 * @brief Fixed-capacity sorted-vector set.
 */
#ifndef SSTL_FLAT_SET_HPP
/** @def SSTL_FLAT_SET_HPP
 * @brief Include guard for flat_set.hpp.
 */
#define SSTL_FLAT_SET_HPP

#include "flat_map.hpp"

namespace sstl {

/** @brief Fixed-capacity sorted set implemented through `flat_map<K, char>`. */
template <class K, size_t N, class Compare = less<K> >
class flat_set {
  /** @brief Underlying flat map type that stores each key once with a dummy value. */
  typedef flat_map<K, char, N, Compare> impl_type;

public:
  class const_iterator;

  /** @brief Forward iterator over stored flat-set keys. */
  class iterator {
  public:
    /** @brief Stored key type exposed by this iterator. */
    typedef K value_type;
    /** @brief Const key reference exposed by this iterator. */
    typedef const K& reference;
    /** @brief Const key pointer exposed by this iterator. */
    typedef const K* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef bidirectional_iterator_tag iterator_category;

    /** @brief Construct a null iterator. */
    iterator() : it_() {}
    /**
     * @brief Dereference the current key.
     * @return Result described by the function brief.
     */
    const K& operator*() const { return it_->first; }
    /**
     * @brief Return a pointer to the current key.
     * @return A pointer to the current key.
     */
    const K* operator->() const { return &it_->first; }
    /** @brief Convert to a const key pointer for legacy pointer-style flat-set code. */
    operator const K*() const { return &it_->first; }
    /**
     * @brief Advance to the next key.
     * @return Result described by the function brief.
     */
    iterator& operator++() { ++it_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
    /** @brief Move to the previous key. */
    iterator& operator--() { --it_; return *this; }
    /** @brief Post-decrement and return the previous iterator value. */
    iterator operator--(int) { iterator old(*this); --*this; return old; }
    /**
     * @brief Compare underlying flat-map iterators for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const iterator& other) const { return it_ == other.it_; }
    /**
     * @brief Compare underlying flat-map iterators for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const iterator& other) const { return !(*this == other); }

  private:
    /** @brief Allow the owning flat_set to construct and inspect mutable iterators. */
    friend class flat_set;
    /** @brief Allow const iterators to copy mutable iterator internals. */
    friend class const_iterator;
    /**
     * @brief Construct from the underlying flat-map iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    explicit iterator(typename impl_type::iterator it) : it_(it) {}
    /** @brief Underlying flat-map iterator naming the key/dummy-value pair. */
    typename impl_type::iterator it_;
  };

  /** @brief Forward const iterator over stored flat-set keys. */
  class const_iterator {
  public:
    /** @brief Stored key type exposed by this iterator. */
    typedef K value_type;
    /** @brief Const key reference exposed by this iterator. */
    typedef const K& reference;
    /** @brief Const key pointer exposed by this iterator. */
    typedef const K* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef bidirectional_iterator_tag iterator_category;

    /** @brief Construct a null const iterator. */
    const_iterator() : it_() {}
    /**
     * @brief Convert a mutable iterator to a const iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    const_iterator(const iterator& it) : it_(it.it_) {}
    /**
     * @brief Dereference the current key.
     * @return Result described by the function brief.
     */
    const K& operator*() const { return it_->first; }
    /**
     * @brief Return a pointer to the current key.
     * @return A pointer to the current key.
     */
    const K* operator->() const { return &it_->first; }
    /** @brief Convert to a key pointer for legacy pointer-style flat-set code. */
    operator const K*() const { return &it_->first; }
    /**
     * @brief Advance to the next key.
     * @return Result described by the function brief.
     */
    const_iterator& operator++() { ++it_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
    /** @brief Move to the previous key. */
    const_iterator& operator--() { --it_; return *this; }
    /** @brief Post-decrement and return the previous iterator value. */
    const_iterator operator--(int) { const_iterator old(*this); --*this; return old; }
    /**
     * @brief Compare underlying flat-map iterators for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const const_iterator& other) const { return it_ == other.it_; }
    /**
     * @brief Compare underlying flat-map iterators for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const const_iterator& other) const { return !(*this == other); }

  private:
    /** @brief Allow the owning flat_set to construct and inspect const iterators. */
    friend class flat_set;
    /**
     * @brief Construct from the underlying flat-map const iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    explicit const_iterator(typename impl_type::const_iterator it) : it_(it) {}
    /** @brief Underlying flat-map const iterator naming the key/dummy-value pair. */
    typename impl_type::const_iterator it_;
  };

  /** @brief Unsigned size type for fixed-capacity counts. */
  typedef size_t size_type;
  /** @brief Key type stored by the flat set. */
  typedef K key_type;
  /** @brief Value type exposed by the flat set. */
  typedef K value_type;
  /** @brief Key comparison predicate type. */
  typedef Compare key_compare;
  /** @brief Value comparison predicate type. */
  typedef Compare value_compare;
  /** @brief Mutable reverse iterator over sorted keys. */
  typedef reverse_iterator_adaptor<iterator> reverse_iterator;
  /** @brief Const reverse iterator over sorted keys. */
  typedef reverse_iterator_adaptor<const_iterator> const_reverse_iterator;

  /**
   * @brief Insert `key` if absent and return iterator plus success flag.
   * @param key Lookup or insertion key.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  pair<iterator, bool> insert(const K& key) {
    pair<typename impl_type::iterator, bool> result = impl_.insert(make_pair(key, char()));
    return make_pair(iterator(result.first), result.second);
  }

  /** @brief Insert `key` using `hint` as a non-binding placement hint. */
  iterator insert(iterator hint, const K& key) {
    (void)hint;
    return insert(key).first;
  }

  /** @brief Insert every key in the half-open input range. */
  template <class InputIt>
  void insert(InputIt first, InputIt last) {
    for (; first != last; ++first) (void)insert(*first);
  }

  /**
   * @brief Find `key`, returning null-style `end()` when absent.
   * @param key Lookup or insertion key.
   * @return Iterator to `key`, or `end()` when absent.
 */
  iterator find(const K& key) { return iterator(impl_.find(key)); }

  /**
   * @brief Find `key` through a const flat set, returning `end()` when absent.
   * @param key Lookup or insertion key.
   * @return Const iterator to `key`, or `end()` when absent.
 */
  const_iterator find(const K& key) const { return const_iterator(impl_.find(key)); }

  /** @brief Return an iterator to the first key not less than `key`. */
  iterator lower_bound(const K& key) { return iterator(impl_.lower_bound(key)); }

  /** @brief Return a const iterator to the first key not less than `key`. */
  const_iterator lower_bound(const K& key) const { return const_iterator(impl_.lower_bound(key)); }

  /** @brief Return an iterator to the first key greater than `key`. */
  iterator upper_bound(const K& key) { return iterator(impl_.upper_bound(key)); }

  /** @brief Return a const iterator to the first key greater than `key`. */
  const_iterator upper_bound(const K& key) const { return const_iterator(impl_.upper_bound(key)); }

  /** @brief Return the half-open range of keys equivalent to `key`. */
  pair<iterator, iterator> equal_range(const K& key) {
    pair<typename impl_type::iterator, typename impl_type::iterator> r = impl_.equal_range(key);
    return make_pair(iterator(r.first), iterator(r.second));
  }

  /** @brief Return the const half-open range of keys equivalent to `key`. */
  pair<const_iterator, const_iterator> equal_range(const K& key) const {
    pair<typename impl_type::const_iterator, typename impl_type::const_iterator> r = impl_.equal_range(key);
    return make_pair(const_iterator(r.first), const_iterator(r.second));
  }

  /**
   * @brief Erase one key and return the number of removed elements.
   * @param key Lookup or insertion key.
   * @return Result described by the function brief.
   */
  size_type erase(const K& key) { return impl_.erase(key); }

  /**
   * @brief Erase the key named by `pos` and return the logical successor.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  iterator erase(iterator pos) {
    if (pos.it_ == impl_.end()) {
      handle_error("flat_set::erase iterator");
      return end();
    }
    return iterator(impl_.erase(pos.it_));
  }

  /** @brief Erase the half-open range `[first,last)`. */
  iterator erase(iterator first, iterator last) {
    return iterator(impl_.erase(first.it_, last.it_));
  }

  /**
   * @brief Return an iterator to the first sorted key.
   * @return An iterator to the first sorted key.
   */
  iterator begin() { return iterator(impl_.begin()); }

  /**
   * @brief Return a const iterator to the first sorted key.
   * @return A const iterator to the first sorted key.
   */
  const_iterator begin() const { return const_iterator(impl_.begin()); }
  /** @brief Return a const iterator to the first sorted key. */
  const_iterator cbegin() const { return begin(); }

  /**
   * @brief Return the null-style end iterator.
   * @return The null-style end iterator.
   */
  iterator end() { return iterator(impl_.end()); }
  /**
   * @brief Return the const end iterator.
   * @return The const end iterator.
   */
  const_iterator end() const { return const_iterator(impl_.end()); }
  /** @brief Return a const iterator one past the final sorted key. */
  const_iterator cend() const { return end(); }
  /** @brief Return a mutable reverse iterator to the final sorted key. */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /** @brief Return a const reverse iterator to the final sorted key. */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /** @brief Return a const reverse iterator to the final sorted key. */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /** @brief Return a mutable reverse iterator one before the first sorted key. */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /** @brief Return a const reverse iterator one before the first sorted key. */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /** @brief Return a const reverse iterator one before the first sorted key. */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return the number of stored keys.
   * @return The number of stored keys.
   */
  size_type size() const { return impl_.size(); }
  /**
   * @brief Return the fixed compile-time key capacity without requiring an object.
   * @return The fixed compile-time key capacity without requiring an object.
   */
  static size_type capacity() { return N; }
  /**
   * @brief Return the fixed compile-time key capacity.
   * @return The fixed compile-time key capacity.
   */
  size_type max_size() const { return impl_.max_size(); }
  /**
   * @brief Report whether no keys are stored.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return impl_.empty(); }
  /**
   * @brief Report whether insertion capacity is exhausted.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool full() const { return impl_.full(); }
  /** @brief Destroy all stored keys. */
  void clear() { impl_.clear(); }
  /** @brief Return one when `key` is present and zero otherwise. */
  size_type count(const K& key) const { return impl_.count(key); }
  /** @brief Return the key comparison predicate. */
  key_compare key_comp() const { return impl_.key_comp(); }
  /** @brief Return the value comparison predicate. */
  value_compare value_comp() const { return impl_.key_comp(); }
  /**
   * @brief Exchange contents with another same-capacity flat set without external allocation.
   * @param other Other object participating in the operation.
   */
  void swap(flat_set& other) { impl_.swap(other.impl_); }

private:
  /** @brief Underlying fixed-capacity sorted map storage. */
  impl_type impl_;
};

/**
 * @brief Exchange two same-capacity flat sets through the member swap operation.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class K, size_t N, class Compare>
inline void swap(flat_set<K, N, Compare>& lhs, flat_set<K, N, Compare>& rhs) {
  lhs.swap(rhs);
}

/** @brief Compare two flat sets for element-wise equality across capacities. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator==(const flat_set<K, N, Compare>& lhs, const flat_set<K, M, Compare>& rhs) {
  if (lhs.size() != rhs.size()) return false;
  typename flat_set<K, N, Compare>::const_iterator a = lhs.begin();
  typename flat_set<K, M, Compare>::const_iterator b = rhs.begin();
  for (; a != lhs.end(); ++a, ++b) {
    if (!(*a == *b)) return false;
  }
  return true;
}

/** @brief Compare two flat sets for inequality. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator!=(const flat_set<K, N, Compare>& lhs, const flat_set<K, M, Compare>& rhs) { return !(lhs == rhs); }

/** @brief Lexicographically compare two flat sets. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator<(const flat_set<K, N, Compare>& lhs, const flat_set<K, M, Compare>& rhs) {
  typename flat_set<K, N, Compare>::const_iterator a = lhs.begin();
  typename flat_set<K, M, Compare>::const_iterator b = rhs.begin();
  for (; a != lhs.end() && b != rhs.end(); ++a, ++b) {
    if (*a < *b) return true;
    if (*b < *a) return false;
  }
  return a == lhs.end() && b != rhs.end();
}

/** @brief Return true when `lhs` is not lexicographically greater than `rhs`. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator<=(const flat_set<K, N, Compare>& lhs, const flat_set<K, M, Compare>& rhs) { return !(rhs < lhs); }

/** @brief Return true when `lhs` is lexicographically greater than `rhs`. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator>(const flat_set<K, N, Compare>& lhs, const flat_set<K, M, Compare>& rhs) { return rhs < lhs; }

/** @brief Return true when `lhs` is not lexicographically less than `rhs`. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator>=(const flat_set<K, N, Compare>& lhs, const flat_set<K, M, Compare>& rhs) { return !(lhs < rhs); }

} // namespace sstl

#endif

