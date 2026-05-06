/**
 * @file set.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_SET_HPP
/** @def SSTL_SET_HPP
 * @brief Include guard for set.hpp.
 */
#define SSTL_SET_HPP

#include "map.hpp"

namespace sstl {

/** @brief Fixed-capacity ordered set implemented through `map<K, char>`. */
template <class K, size_t N, class Compare = less<K> >
class set {
  /** @brief Underlying ordered map type that stores each key once with a dummy value. */
  typedef map<K, char, N, Compare> impl_type;
public:
  class const_iterator;

  /** @brief Forward iterator over stored set keys. */
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
    /** @brief Convert to a const key pointer for legacy pointer-style set code. */
    operator const K*() const { return &it_->first; }
    /**
     * @brief Advance to the next ordered key.
     * @return Result described by the function brief.
     */
    iterator& operator++() { ++it_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
    /**
     * @brief Move to the previous ordered key.
     * @return Result described by the function brief.
     */
    iterator& operator--() { --it_; return *this; }
    /**
     * @brief Post-decrement and return the previous iterator value.
     * @return Result described by the function brief.
     */
    iterator operator--(int) { iterator old(*this); --*this; return old; }
    /**
     * @brief Compare underlying map iterators for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const iterator& other) const { return it_ == other.it_; }
    /**
     * @brief Compare underlying map iterators for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const iterator& other) const { return !(*this == other); }

  private:
    /** @brief Allow the owning set to construct and inspect mutable iterators. */
    friend class set;
    /** @brief Allow const iterators to copy mutable iterator internals. */
    friend class const_iterator;
    /**
     * @brief Construct from the underlying map iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    explicit iterator(typename impl_type::iterator it) : it_(it) {}
    /** @brief Underlying map iterator naming the key/dummy-value pair. */
    typename impl_type::iterator it_;
  };

  /** @brief Forward const iterator over stored set keys. */
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
     * @brief Convert a mutable iterator into a const iterator.
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
    /** @brief Convert to a key pointer for legacy pointer-style set code. */
    operator const K*() const { return &it_->first; }
    /**
     * @brief Advance to the next ordered key.
     * @return Result described by the function brief.
     */
    const_iterator& operator++() { ++it_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
    /**
     * @brief Move to the previous ordered key.
     * @return Result described by the function brief.
     */
    const_iterator& operator--() { --it_; return *this; }
    /**
     * @brief Post-decrement and return the previous iterator value.
     * @return Result described by the function brief.
     */
    const_iterator operator--(int) { const_iterator old(*this); --*this; return old; }
    /**
     * @brief Compare underlying map iterators for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const const_iterator& other) const { return it_ == other.it_; }
    /**
     * @brief Compare underlying map iterators for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const const_iterator& other) const { return !(*this == other); }

  private:
    /** @brief Allow the owning set to construct and inspect const iterators. */
    friend class set;
    /**
     * @brief Construct from the underlying map const iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    explicit const_iterator(typename impl_type::const_iterator it) : it_(it) {}
    /** @brief Underlying map const iterator naming the key/dummy-value pair. */
    typename impl_type::const_iterator it_;
  };

  /** @brief Unsigned size type for fixed-capacity counts. */
  typedef size_t size_type;
  /** @brief Key type stored by the set. */
  typedef K key_type;
  /** @brief Value type exposed by the set. */
  typedef K value_type;
  /** @brief Key comparison predicate type. */
  typedef Compare key_compare;
  /** @brief Value comparison predicate type. */
  typedef Compare value_compare;
  /** @brief Mutable reverse iterator over ordered keys. */
  typedef reverse_iterator_adaptor<iterator> reverse_iterator;
  /** @brief Const reverse iterator over ordered keys. */
  typedef reverse_iterator_adaptor<const_iterator> const_reverse_iterator;

  /**
   * @brief Insert `k` if absent and return iterator plus success flag.
   * @param k Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  pair<iterator, bool> insert(const K& k) {
    pair<typename impl_type::iterator, bool> r = impl_.insert(make_pair(k, char()));
    return make_pair(iterator(r.first), r.second);
  }
  /**
   * @brief Insert `k` using `hint` as a non-binding placement hint.
   * @param hint Suggested insertion position.
   * @param k Key value to insert.
   * @return Iterator to the stored key.
   */
  iterator insert(iterator hint, const K& k) {
    (void)hint;
    return insert(k).first;
  }
  /**
   * @brief Insert every key in the half-open input range.
   * @param first Start of the half-open range.
   * @param last One-past-end of the range.
   */
  template <class InputIt>
  void insert(InputIt first, InputIt last) {
    for (; first != last; ++first) (void)insert(*first);
  }
  /**
   * @brief Find `k`, returning null-style `end()` when absent.
   * @param k Caller-supplied argument used by this operation.
   * @return Iterator to `k`, or `end()` when absent.
 */
  iterator find(const K& k) { return iterator(impl_.find(k)); }
  /**
   * @brief Find `k` through a const set, returning `end()` when absent.
   * @param k Caller-supplied argument used by this operation.
   * @return Const iterator to `k`, or `end()` when absent.
 */
  const_iterator find(const K& k) const { return const_iterator(impl_.find(k)); }
  /**
   * @brief Return one when `k` is present and zero otherwise.
   * @param k Caller-supplied argument used by this operation.
   * @return One when `k` is present and zero otherwise.
   */
  size_type count(const K& k) const { return impl_.count(k); }
  /**
   * @brief Return the first iterator whose key is not less than `k`.
   * @param k Caller-supplied argument used by this operation.
   * @return The first iterator whose key is not less than `k`.
   */
  iterator lower_bound(const K& k) { return iterator(impl_.lower_bound(k)); }
  /**
   * @brief Return the first const iterator whose key is not less than `k`.
   * @param k Caller-supplied argument used by this operation.
   * @return The first const iterator whose key is not less than `k`.
   */
  const_iterator lower_bound(const K& k) const { return const_iterator(impl_.lower_bound(k)); }
  /**
   * @brief Return the first iterator whose key is greater than `k`.
   * @param k Caller-supplied argument used by this operation.
   * @return The first iterator whose key is greater than `k`.
   */
  iterator upper_bound(const K& k) { return iterator(impl_.upper_bound(k)); }
  /**
   * @brief Return the first const iterator whose key is greater than `k`.
   * @param k Caller-supplied argument used by this operation.
   * @return The first const iterator whose key is greater than `k`.
   */
  const_iterator upper_bound(const K& k) const { return const_iterator(impl_.upper_bound(k)); }
  /**
   * @brief Return the half-open iterator range containing keys equal to `k`.
   * @param k Caller-supplied argument used by this operation.
   * @return The half-open iterator range containing keys equal to `k`.
   */
  pair<iterator, iterator> equal_range(const K& k) {
    pair<typename impl_type::iterator, typename impl_type::iterator> r = impl_.equal_range(k);
    return make_pair(iterator(r.first), iterator(r.second));
  }
  /**
   * @brief Return the const half-open iterator range containing keys equal to `k`.
   * @param k Caller-supplied argument used by this operation.
   * @return The const half-open iterator range containing keys equal to `k`.
   */
  pair<const_iterator, const_iterator> equal_range(const K& k) const {
    pair<typename impl_type::const_iterator, typename impl_type::const_iterator> r = impl_.equal_range(k);
    return make_pair(const_iterator(r.first), const_iterator(r.second));
  }
  /**
   * @brief Erase one key and return the number of removed elements.
   * @param k Caller-supplied argument used by this operation.
   * @return Result described by the function brief.
   */
  size_type erase(const K& k) { return impl_.erase(k); }
  /**
   * @brief Erase the key named by `pos` and return the logical successor.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  iterator erase(iterator pos) {
    if (pos.it_ == impl_.end()) {
      handle_error("set::erase iterator");
      return end();
    }
    return iterator(impl_.erase(pos.it_));
  }
  /**
   * @brief Erase the half-open range `[first,last)`.
   * @param first Start of the half-open range.
   * @param last One-past-end of the range.
   * @return Iterator naming `last` after erasure.
   */
  iterator erase(iterator first, iterator last) {
    return iterator(impl_.erase(first.it_, last.it_));
  }
  /**
   * @brief Return an iterator to the first ordered key.
   * @return An iterator to the first ordered key.
   */
  iterator begin() { return iterator(impl_.begin()); }
  /**
   * @brief Return a const iterator to the first ordered key.
   * @return A const iterator to the first ordered key.
   */
  const_iterator begin() const { return const_iterator(impl_.begin()); }
  /**
   * @brief Return a const iterator to the first ordered key.
   * @return A const iterator to the first ordered key.
   */
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
  /**
   * @brief Return a const iterator one past the final ordered key.
   * @return A const iterator one past the final ordered key.
   */
  const_iterator cend() const { return end(); }
  /**
   * @brief Return a mutable reverse iterator to the final ordered key.
   * @return A mutable reverse iterator to the final ordered key.
   */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final ordered key.
   * @return A const reverse iterator to the final ordered key.
   */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final ordered key.
   * @return A const reverse iterator to the final ordered key.
   */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a mutable reverse iterator one before the first ordered key.
   * @return A mutable reverse iterator one before the first ordered key.
   */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first ordered key.
   * @return A const reverse iterator one before the first ordered key.
   */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first ordered key.
   * @return A const reverse iterator one before the first ordered key.
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return the number of stored keys.
   * @return The number of stored keys.
   */
  size_type size() const { return impl_.size(); }
  /**
   * @brief Return the fixed compile-time capacity without requiring an object.
   * @return The fixed compile-time capacity without requiring an object.
   */
  static size_type capacity() { return N; }
  /**
   * @brief Return the largest number of keys this fixed-capacity set can hold.
   * @return The largest number of keys this fixed-capacity set can hold.
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
  /**
   * @brief Return the key comparison predicate.
   * @return The key comparison predicate.
   */
  key_compare key_comp() const { return impl_.key_comp(); }
  /**
   * @brief Return the value comparison predicate.
   * @return The value comparison predicate.
   */
  value_compare value_comp() const { return impl_.key_comp(); }
  /**
   * @brief Exchange contents with another same-capacity set without external allocation.
   * @param other Other object participating in the operation.
   */
  void swap(set& other) { impl_.swap(other.impl_); }
private:
  /** @brief Underlying fixed-capacity ordered map storage. */
  impl_type impl_;
};

/**
 * @brief Exchange two same-capacity ordered sets through the member swap operation.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class K, size_t N, class Compare>
inline void swap(set<K, N, Compare>& lhs, set<K, N, Compare>& rhs) {
  lhs.swap(rhs);
}

/** @brief Compare two ordered sets for element-wise equality across capacities. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator==(const set<K, N, Compare>& lhs, const set<K, M, Compare>& rhs) {
  if (lhs.size() != rhs.size()) return false;
  typename set<K, N, Compare>::const_iterator a = lhs.begin();
  typename set<K, M, Compare>::const_iterator b = rhs.begin();
  for (; a != lhs.end(); ++a, ++b) {
    if (!(*a == *b)) return false;
  }
  return true;
}

/** @brief Compare two ordered sets for inequality. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator!=(const set<K, N, Compare>& lhs, const set<K, M, Compare>& rhs) { return !(lhs == rhs); }

/** @brief Lexicographically compare two ordered sets. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator<(const set<K, N, Compare>& lhs, const set<K, M, Compare>& rhs) {
  typename set<K, N, Compare>::const_iterator a = lhs.begin();
  typename set<K, M, Compare>::const_iterator b = rhs.begin();
  for (; a != lhs.end() && b != rhs.end(); ++a, ++b) {
    if (*a < *b) return true;
    if (*b < *a) return false;
  }
  return a == lhs.end() && b != rhs.end();
}

/** @brief Return true when `lhs` is not lexicographically greater than `rhs`. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator<=(const set<K, N, Compare>& lhs, const set<K, M, Compare>& rhs) { return !(rhs < lhs); }

/** @brief Return true when `lhs` is lexicographically greater than `rhs`. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator>(const set<K, N, Compare>& lhs, const set<K, M, Compare>& rhs) { return rhs < lhs; }

/** @brief Return true when `lhs` is not lexicographically less than `rhs`. */
template <class K, size_t N, size_t M, class Compare>
inline bool operator>=(const set<K, N, Compare>& lhs, const set<K, M, Compare>& rhs) { return !(lhs < rhs); }

} // namespace sstl

#endif

