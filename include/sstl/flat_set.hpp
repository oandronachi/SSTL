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
    /** @brief Mutable key reference exposed by this iterator. */
    typedef K& reference;
    /** @brief Mutable key pointer exposed by this iterator. */
    typedef K* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;

    /** @brief Construct a null iterator. */
    iterator() : it_() {}
    /**
     * @brief Dereference the current key.
     * @return Result described by the function brief.
     */
    K& operator*() const { return it_->first; }
    /**
     * @brief Return a pointer to the current key.
     * @return A pointer to the current key.
     */
    K* operator->() const { return &it_->first; }
    /** @brief Convert to a key pointer for legacy pointer-style flat-set code. */
    operator K*() const { return &it_->first; }
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
    friend class flat_set;
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
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;

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

  /**
   * @brief Insert `key` if absent and return iterator plus success flag.
   * @param key Lookup or insertion key.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  pair<iterator, bool> insert(const K& key) {
    pair<typename impl_type::iterator, bool> result = impl_.insert(make_pair(key, char()));
    return make_pair(iterator(result.first), result.second);
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

} // namespace sstl

#endif

