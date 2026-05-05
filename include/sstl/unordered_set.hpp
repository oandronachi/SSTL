/**
 * @file unordered_set.hpp
 * @brief Fixed-capacity hash table set.
 *
 * The set reuses the same static bucket/node strategy as `unordered_map`,
 * storing only keys and exposing key pointers as iterators.
 */
#ifndef SSTL_UNORDERED_SET_HPP
/** @def SSTL_UNORDERED_SET_HPP
 * @brief Include guard for unordered_set.hpp.
 */
#define SSTL_UNORDERED_SET_HPP

#include "unordered_map.hpp"

namespace sstl {

/** @brief Fixed-capacity hash set implemented through `unordered_map<K, char>`. */
template <class K, size_t N, size_t B = next_prime_ge<N>::value, class H = hash<K>, class Eq = equal_to<K> >
class unordered_set {
  /** @brief Underlying hash map type that stores each key once with a dummy value. */
  typedef unordered_map<K, char, N, B, H, Eq> impl_type;

public:
  /** @brief Key type stored by the fixed-capacity unordered set. */
  typedef K value_type;
  /** @brief Unsigned size and bucket index type used by the unordered set. */
  typedef size_t size_type;

  class const_iterator;

  /** @brief Forward iterator over occupied set keys. */
  class iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef K value_type;
    /** @brief Mutable reference type exposed by this iterator. */
    typedef K& reference;
    /** @brief Mutable pointer type exposed by this iterator. */
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
    /**
     * @brief Advance to the next occupied key or `end()`.
     * @return Result described by the function brief.
     */
    iterator& operator++() { ++it_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
    /**
     * @brief Compare underlying hash-map iterators for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const iterator& other) const { return it_ == other.it_; }
    /**
     * @brief Compare underlying hash-map iterators for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const iterator& other) const { return !(*this == other); }

  private:
    friend class unordered_set;
    friend class const_iterator;
    /**
     * @brief Construct from the underlying map iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    explicit iterator(typename impl_type::iterator it) : it_(it) {}
    /** @brief Underlying map iterator naming the key/dummy-value pair. */
    typename impl_type::iterator it_;
  };

  /** @brief Forward const iterator over occupied set keys. */
  class const_iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef K value_type;
    /** @brief Const reference type exposed by this iterator. */
    typedef const K& reference;
    /** @brief Const pointer type exposed by this iterator. */
    typedef const K* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;

    /** @brief Construct a null const iterator. */
    const_iterator() : it_() {}
    /**
     * @brief Convert a mutable set iterator into a const iterator.
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
    /**
     * @brief Advance to the next occupied key or `end()`.
     * @return Result described by the function brief.
     */
    const_iterator& operator++() { ++it_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
    /**
     * @brief Compare underlying hash-map iterators for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const const_iterator& other) const { return it_ == other.it_; }
    /**
     * @brief Compare underlying hash-map iterators for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const const_iterator& other) const { return !(*this == other); }

  private:
    friend class unordered_set;
    /**
     * @brief Construct from the underlying const map iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    explicit const_iterator(typename impl_type::const_iterator it) : it_(it) {}
    /** @brief Underlying const map iterator naming the key/dummy-value pair. */
    typename impl_type::const_iterator it_;
  };

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
   * @brief Find `key` through a const set, returning `end()` when absent.
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
    if (!impl_.is_valid_iterator(pos.it_) || pos.it_ == impl_.end()) {
      handle_error("unordered_set::erase iterator");
      return end();
    }
    return iterator(impl_.erase(pos.it_));
  }

  /**
   * @brief Return an iterator to the first stored key.
   * @return An iterator to the first stored key.
   */
  iterator begin() { return iterator(impl_.begin()); }

  /**
   * @brief Return a const iterator to the first stored key.
   * @return A const iterator to the first stored key.
   */
  const_iterator begin() const { return const_iterator(impl_.begin()); }

  /**
   * @brief Return the end iterator sentinel.
   * @return The end iterator sentinel.
   */
  iterator end() { return iterator(impl_.end()); }

  /**
   * @brief Return the const end iterator sentinel.
   * @return The const end iterator sentinel.
   */
  const_iterator end() const { return const_iterator(impl_.end()); }

  /** @brief Destroy all stored keys. */
  void clear() { impl_.clear(); }
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
   * @brief Return the fixed bucket count configured for the underlying hash map.
   * @return The fixed bucket count configured for the underlying hash map.
   */
  size_type bucket_count() const { return impl_.bucket_count(); }
  /**
   * @brief Return the underlying hash table load factor.
   * @return The underlying hash table load factor.
   */
  float load_factor() const { return impl_.load_factor(); }

  /**
   * @brief Validate that a mutable iterator belongs to this set's underlying map.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const { return impl_.is_valid_iterator(it.it_); }

  /**
   * @brief Validate that a const iterator belongs to this set's underlying map.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(const_iterator it) const { return impl_.is_valid_iterator(it.it_); }

  /**
   * @brief Exchange contents with another same-capacity unordered set without external allocation.
   * @param other Other object participating in the operation.
   */
  void swap(unordered_set& other) { impl_.swap(other.impl_); }

private:
  /** @brief Underlying fixed-capacity hash map storage. */
  impl_type impl_;
};

/**
 * @brief Exchange two same-capacity unordered sets through the member swap operation.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class K, size_t N, size_t B, class H, class Eq>
inline void swap(unordered_set<K, N, B, H, Eq>& lhs, unordered_set<K, N, B, H, Eq>& rhs) {
  lhs.swap(rhs);
}

} // namespace sstl

#endif

