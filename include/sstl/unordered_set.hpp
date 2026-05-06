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
  class const_local_iterator;

  /** @brief Forward iterator over occupied set keys. */
  class iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef K value_type;
    /** @brief Const reference type exposed by this iterator. */
    typedef const K& reference;
    /** @brief Const pointer type exposed by this iterator. */
    typedef const K* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;

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
    /** @brief Allow the owning unordered_set to construct and inspect mutable iterators. */
    friend class unordered_set;
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
    typedef ptrdiff_t difference_type;
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
    /** @brief Allow the owning unordered_set to construct and inspect const iterators. */
    friend class unordered_set;
    /**
     * @brief Construct from the underlying const map iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    explicit const_iterator(typename impl_type::const_iterator it) : it_(it) {}
    /** @brief Underlying const map iterator naming the key/dummy-value pair. */
    typename impl_type::const_iterator it_;
  };

  /** @brief Forward iterator over keys in one bucket. */
  class local_iterator {
  public:
    /** @brief Key type exposed by this iterator. */
    typedef K value_type;
    /** @brief Const key reference type. */
    typedef const K& reference;
    /** @brief Const key pointer type. */
    typedef const K* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;
    /** @brief Construct a null local iterator. */
    local_iterator() : it_() {}
    /** @brief Dereference the current bucket key. */
    const K& operator*() const { return it_->first; }
    /** @brief Return a pointer to the current bucket key. */
    const K* operator->() const { return &it_->first; }
    /** @brief Advance to the next key in the bucket. */
    local_iterator& operator++() { ++it_; return *this; }
    /** @brief Post-increment and return the previous local iterator value. */
    local_iterator operator++(int) { local_iterator old(*this); ++*this; return old; }
    /** @brief Compare underlying local iterators for equality. */
    bool operator==(const local_iterator& other) const { return it_ == other.it_; }
    /** @brief Compare underlying local iterators for inequality. */
    bool operator!=(const local_iterator& other) const { return !(*this == other); }
  private:
    /** @brief Allow unordered_set to construct and inspect local iterators. */
    friend class unordered_set;
    /** @brief Allow const local iterators to copy mutable local iterator internals. */
    friend class const_local_iterator;
    /** @brief Construct from the underlying map local iterator. */
    explicit local_iterator(typename impl_type::local_iterator it) : it_(it) {}
    /** @brief Underlying map local iterator naming the key/dummy-value pair. */
    typename impl_type::local_iterator it_;
  };

  /** @brief Const forward iterator over keys in one bucket. */
  class const_local_iterator {
  public:
    /** @brief Key type exposed by this iterator. */
    typedef K value_type;
    /** @brief Const key reference type. */
    typedef const K& reference;
    /** @brief Const key pointer type. */
    typedef const K* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;
    /** @brief Construct a null const local iterator. */
    const_local_iterator() : it_() {}
    /** @brief Convert a mutable local iterator to a const local iterator. */
    const_local_iterator(const local_iterator& it) : it_(it.it_) {}
    /** @brief Dereference the current bucket key. */
    const K& operator*() const { return it_->first; }
    /** @brief Return a pointer to the current bucket key. */
    const K* operator->() const { return &it_->first; }
    /** @brief Advance to the next key in the bucket. */
    const_local_iterator& operator++() { ++it_; return *this; }
    /** @brief Post-increment and return the previous local iterator value. */
    const_local_iterator operator++(int) { const_local_iterator old(*this); ++*this; return old; }
    /** @brief Compare underlying local iterators for equality. */
    bool operator==(const const_local_iterator& other) const { return it_ == other.it_; }
    /** @brief Compare underlying local iterators for inequality. */
    bool operator!=(const const_local_iterator& other) const { return !(*this == other); }
  private:
    /** @brief Allow unordered_set to construct and inspect const local iterators. */
    friend class unordered_set;
    /** @brief Construct from the underlying map const local iterator. */
    explicit const_local_iterator(typename impl_type::const_local_iterator it) : it_(it) {}
    /** @brief Underlying map const local iterator naming the key/dummy-value pair. */
    typename impl_type::const_local_iterator it_;
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
   * @brief Find `key` through a const set, returning `end()` when absent.
   * @param key Lookup or insertion key.
   * @return Const iterator to `key`, or `end()` when absent.
 */
  const_iterator find(const K& key) const { return const_iterator(impl_.find(key)); }

  /** @brief Return one when `key` is present and zero otherwise. */
  size_type count(const K& key) const { return impl_.count(key); }

  /** @brief Return the half-open range containing `key`. */
  pair<iterator, iterator> equal_range(const K& key) {
    pair<typename impl_type::iterator, typename impl_type::iterator> r = impl_.equal_range(key);
    return make_pair(iterator(r.first), iterator(r.second));
  }

  /** @brief Return the const half-open range containing `key`. */
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
    if (!impl_.is_valid_iterator(pos.it_) || pos.it_ == impl_.end()) {
      handle_error("unordered_set::erase iterator");
      return end();
    }
    return iterator(impl_.erase(pos.it_));
  }

  /** @brief Erase the half-open range `[first,last)`. */
  iterator erase(iterator first, iterator last) {
    return iterator(impl_.erase(first.it_, last.it_));
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
  /** @brief Return a const iterator to the first stored key. */
  const_iterator cbegin() const { return begin(); }

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
  /** @brief Return a const iterator one past the final stored key. */
  const_iterator cend() const { return end(); }

  /** @brief Return a local iterator to the first key in bucket `n`. */
  local_iterator begin(size_type n) { return local_iterator(impl_.begin(n)); }
  /** @brief Return a const local iterator to the first key in bucket `n`. */
  const_local_iterator begin(size_type n) const { return const_local_iterator(impl_.begin(n)); }
  /** @brief Return a const local iterator to the first key in bucket `n`. */
  const_local_iterator cbegin(size_type n) const { return begin(n); }
  /** @brief Return a local iterator one past the final key in bucket `n`. */
  local_iterator end(size_type n) { return local_iterator(impl_.end(n)); }
  /** @brief Return a const local iterator one past the final key in bucket `n`. */
  const_local_iterator end(size_type n) const { return const_local_iterator(impl_.end(n)); }
  /** @brief Return a const local iterator one past the final key in bucket `n`. */
  const_local_iterator cend(size_type n) const { return end(n); }

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
  /** @brief Return the maximum fixed bucket count. */
  size_type max_bucket_count() const { return impl_.max_bucket_count(); }
  /** @brief Return the bucket index for `key`. */
  size_type bucket(const K& key) const { return impl_.bucket(key); }
  /** @brief Return the number of stored keys in bucket `n`. */
  size_type bucket_size(size_type n) const { return impl_.bucket_size(n); }
  /**
   * @brief Return the underlying hash table load factor.
   * @return The underlying hash table load factor.
   */
  float load_factor() const { return impl_.load_factor(); }
  /** @brief Return the fixed maximum load factor implied by capacity and bucket count. */
  float max_load_factor() const { return impl_.max_load_factor(); }
  /** @brief Return the hash functor. */
  H hash_function() const { return impl_.hash_function(); }
  /** @brief Return the key equality predicate. */
  Eq key_eq() const { return impl_.key_eq(); }

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

/** @brief Compare two unordered sets for key equality across capacities and bucket counts. */
template <class K, size_t N, size_t M, size_t B1, size_t B2, class H, class Eq>
inline bool operator==(const unordered_set<K, N, B1, H, Eq>& lhs,
                       const unordered_set<K, M, B2, H, Eq>& rhs) {
  if (lhs.size() != rhs.size()) return false;
  typename unordered_set<K, N, B1, H, Eq>::const_iterator it = lhs.begin();
  for (; it != lhs.end(); ++it) {
    if (rhs.find(*it) == rhs.end()) return false;
  }
  return true;
}

/** @brief Compare two unordered sets for inequality. */
template <class K, size_t N, size_t M, size_t B1, size_t B2, class H, class Eq>
inline bool operator!=(const unordered_set<K, N, B1, H, Eq>& lhs,
                       const unordered_set<K, M, B2, H, Eq>& rhs) {
  return !(lhs == rhs);
}

} // namespace sstl

#endif

