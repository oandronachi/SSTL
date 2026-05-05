/**
 * @file flat_map.hpp
 * @brief Fixed-capacity sorted-vector associative container.
 *
 * `flat_map` intentionally stores pairs contiguously. Insertions can move
 * elements and therefore invalidate iterators, but iteration has compact memory
 * locality and no per-node link overhead.
 */
#ifndef SSTL_FLAT_MAP_HPP
/** @def SSTL_FLAT_MAP_HPP
 * @brief Include guard for flat_map.hpp.
 */
#define SSTL_FLAT_MAP_HPP

#include "vector.hpp"
#include "utility.hpp"

namespace sstl {

/** @brief Sorted contiguous fixed-capacity map with generation-tagged iterators. */
template <class K, class V, size_t N, class Compare = less<K> >
class flat_map {
public:
  /** @brief Key/value pair type stored in sorted contiguous order. */
  typedef pair<K, V> value_type;
  /** @brief Unsigned size and index type used by the flat map. */
  typedef size_t size_type;

  class const_iterator;

  /** @brief Generation-tagged iterator over contiguous sorted map entries. */
  class iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef pair<K, V> value_type;
    /** @brief Mutable reference type exposed by this iterator. */
    typedef value_type& reference;
    /** @brief Mutable pointer type exposed by this iterator. */
    typedef value_type* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;

    /** @brief Construct a null iterator. */
    iterator() : owner_(0), index_(0u), generation_(0u) {}
    /**
     * @brief Dereference the current entry.
     * @return Result described by the function brief.
     */
    value_type& operator*() const { return owner_->data_[index_]; }
    /**
     * @brief Return a pointer to the current entry.
     * @return A pointer to the current entry.
     */
    value_type* operator->() const { return &owner_->data_[index_]; }
    /**
     * @brief Advance to the next entry.
     * @return Result described by the function brief.
     */
    iterator& operator++() { ++index_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
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

  private:
    friend class flat_map;
    friend class const_iterator;
    /**
     * @brief Construct an iterator at an index and validity generation.
     * @param owner Caller-supplied argument used by this operation.
     * @param index Caller-supplied argument used by this operation.
     * @param generation Caller-supplied argument used by this operation.
     */
    iterator(flat_map* owner, size_type index, unsigned generation)
      : owner_(owner), index_(index), generation_(generation) {}
    /** @brief Owning flat_map used for dereference and validation. */
    flat_map* owner_;
    /** @brief Logical index in the sorted contiguous storage. */
    size_type index_;
    /** @brief Mutation generation captured when the iterator was produced. */
    unsigned generation_;
  };

  /** @brief Generation-tagged const iterator over contiguous sorted map entries. */
  class const_iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef pair<K, V> value_type;
    /** @brief Const reference type exposed by this iterator. */
    typedef const value_type& reference;
    /** @brief Const pointer type exposed by this iterator. */
    typedef const value_type* pointer;
    /** @brief Signed difference type used by generic iterator traits. */
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;

    /** @brief Construct a null const iterator. */
    const_iterator() : owner_(0), index_(0u), generation_(0u) {}
    /**
     * @brief Convert a mutable iterator to a const iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    const_iterator(const iterator& it) : owner_(it.owner_), index_(it.index_), generation_(it.generation_) {}
    /**
     * @brief Dereference the current entry.
     * @return Result described by the function brief.
     */
    const value_type& operator*() const { return owner_->data_[index_]; }
    /**
     * @brief Return a pointer to the current entry.
     * @return A pointer to the current entry.
     */
    const value_type* operator->() const { return &owner_->data_[index_]; }
    /**
     * @brief Advance to the next entry.
     * @return Result described by the function brief.
     */
    const_iterator& operator++() { ++index_; return *this; }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
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

  private:
    friend class flat_map;
    /**
     * @brief Construct a const iterator at an index and validity generation.
     * @param owner Caller-supplied argument used by this operation.
     * @param index Caller-supplied argument used by this operation.
     * @param generation Caller-supplied argument used by this operation.
     */
    const_iterator(const flat_map* owner, size_type index, unsigned generation)
      : owner_(owner), index_(index), generation_(generation) {}
    /** @brief Owning flat_map used for dereference and validation. */
    const flat_map* owner_;
    /** @brief Logical index in the sorted contiguous storage. */
    size_type index_;
    /** @brief Mutation generation captured when the iterator was produced. */
    unsigned generation_;
  };

  /** @brief Construct an empty map in iterator generation zero. */
  flat_map() : generation_(0u) {}

  /**
   * @brief Return a mutable iterator to the first sorted pair.
   * @return A mutable iterator to the first sorted pair.
   */
  iterator begin() { return iterator(this, 0u, generation_); }
  /**
   * @brief Return a mutable iterator one past the last sorted pair.
   * @return A mutable iterator one past the last sorted pair.
   */
  iterator end() { return iterator(this, data_.size(), generation_); }
  /**
   * @brief Return a const iterator to the first sorted pair.
   * @return A const iterator to the first sorted pair.
   */
  const_iterator begin() const { return const_iterator(this, 0u, generation_); }
  /**
   * @brief Return a const iterator one past the last sorted pair.
   * @return A const iterator one past the last sorted pair.
   */
  const_iterator end() const { return const_iterator(this, data_.size(), generation_); }
  /**
   * @brief Return the number of stored key/value pairs.
   * @return The number of stored key/value pairs.
   */
  size_type size() const { return data_.size(); }
  /**
   * @brief Return the fixed compile-time pair capacity without requiring an object.
   * @return The fixed compile-time pair capacity without requiring an object.
   */
  static size_type capacity() { return N; }
  /**
   * @brief Return the fixed compile-time pair capacity.
   * @return The fixed compile-time pair capacity.
   */
  size_type max_size() const { return N; }
  /**
   * @brief Report whether the map contains no pairs.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return data_.empty(); }
  /**
   * @brief Report whether insertion capacity is exhausted.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool full() const { return data_.full(); }

  /**
   * @brief Find a mutable pair by key, or return `end()` when absent.
   * @param key Lookup or insertion key.
   * @return Result described by the function brief.
   */
  iterator find(const K& key) {
    const size_type pos = lower_bound_index(key);
    return (pos != data_.size() && lower_bound_key_matches(data_[pos].first, key)) ? iterator(this, pos, generation_) : end();
  }

  /**
   * @brief Find a const pair by key, or return `end()` when absent.
   * @param key Lookup or insertion key.
   * @return Result described by the function brief.
   */
  const_iterator find(const K& key) const {
    const size_type pos = lower_bound_index(key);
    return (pos != data_.size() && lower_bound_key_matches(data_[pos].first, key)) ? const_iterator(this, pos, generation_) : end();
  }

  /**
   * @brief Return the first entry whose key is not ordered before `key`.
   * @param key Lookup or insertion key.
   * @return The first entry whose key is not ordered before `key`.
   */
  iterator lower_bound(const K& key) { return iterator(this, lower_bound_index(key), generation_); }

  /**
   * @brief Return the first const entry whose key is not ordered before `key`.
   * @param key Lookup or insertion key.
   * @return The first const entry whose key is not ordered before `key`.
   */
  const_iterator lower_bound(const K& key) const { return const_iterator(this, lower_bound_index(key), generation_); }

  /**
   * @brief Return the first entry whose key is ordered after `key`.
   * @param key Lookup or insertion key.
   * @return The first entry whose key is ordered after `key`.
   */
  iterator upper_bound(const K& key) { return iterator(this, upper_bound_index(key), generation_); }

  /**
   * @brief Return the first const entry whose key is ordered after `key`.
   * @param key Lookup or insertion key.
   * @return The first const entry whose key is ordered after `key`.
   */
  const_iterator upper_bound(const K& key) const { return const_iterator(this, upper_bound_index(key), generation_); }

  /**
   * @brief Return the half-open subrange containing entries equivalent to `key`.
   * @param key Lookup or insertion key.
   * @return The half-open subrange containing entries equivalent to `key`.
   */
  pair<iterator, iterator> equal_range(const K& key) { return make_pair(lower_bound(key), upper_bound(key)); }

  /**
   * @brief Return the const half-open subrange containing entries equivalent to `key`.
   * @param key Lookup or insertion key.
   * @return The const half-open subrange containing entries equivalent to `key`.
   */
  pair<const_iterator, const_iterator> equal_range(const K& key) const {
    return make_pair(lower_bound(key), upper_bound(key));
  }

  /**
   * @brief Insert a key/value pair while preserving sorted key order.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  pair<iterator, bool> insert(const value_type& value) {
    const size_type pos = lower_bound_index(value.first);
    if (pos != data_.size() && lower_bound_key_matches(data_[pos].first, value.first)) {
      return make_pair(iterator(this, pos, generation_), false);
    }
    if (full()) {
      handle_error("flat_map::insert full");
      return make_pair(end(), false);
    }
    (void)data_.insert(data_.begin() + pos, value);
    ++generation_;
    return make_pair(iterator(this, pos, generation_), true);
  }

  /**
   * @brief Erase one element by key and return the number of removed entries.
   * @param key Lookup or insertion key.
   * @return Result described by the function brief.
   */
  size_type erase(const K& key) {
    iterator it = find(key);
    if (it == end()) return 0u;
    erase(it);
    return 1u;
  }

  /**
   * @brief Erase the element named by `pos` and return its logical successor.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  iterator erase(iterator pos) {
    if (!is_valid_iterator(pos) || pos.index_ >= data_.size()) {
      handle_error("flat_map::erase iterator");
      return end();
    }
    const size_type index = pos.index_;
    data_.erase(data_.begin() + index);
    ++generation_;
    return iterator(this, index, generation_);
  }

  /**
   * @brief Return the mapped value for `key`, inserting a default value when absent.
   * @param key Lookup or insertion key.
   * @return The mapped value for `key`, inserting a default value when absent.
   */
  V& operator[](const K& key) {
    iterator it = find(key);
    if (it != end()) return it->second;
    pair<iterator, bool> inserted = insert(make_pair(key, V()));
    if (inserted.second) return inserted.first->second;
    handle_error("flat_map::operator[] full");
    return overflow_value();
  }

  /**
   * @brief Exchange all fixed-capacity contents with another flat map of the same type.
   * @param other Other object participating in the operation.
   */
  void swap(flat_map& other) {
    if (this == &other) return;
    flat_map tmp(*this);
    *this = other;
    other = tmp;
    ++generation_;
    ++other.generation_;
  }

  /**
   * @brief Test whether an iterator belongs to this map's current validity epoch.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const {
    return it.owner_ == this && it.generation_ == generation_ && it.index_ <= data_.size();
  }

  /**
   * @brief Test whether a const iterator belongs to this map's current validity epoch.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(const_iterator it) const {
    return it.owner_ == this && it.generation_ == generation_ && it.index_ <= data_.size();
  }

  /**
   * @brief Backward-compatible no-op test hook for older iterator diagnostics.
   *
   * Iterators now capture the current generation when they are created, so a
   * freshly captured iterator validates without any manual reset.
   */
  void revalidate_iterators() const {}

private:
  /** @brief Contiguous sorted storage for key/value pairs. */
  vector<value_type, N> data_;
  /** @brief Ordering predicate used for key search and insertion placement. */
  Compare comp_;
  /** @brief Mutation generation captured by iterators for validity checks. */
  unsigned generation_;

  /**
   * @brief Return a stable fallback value for failed `operator[]` insertions.
   * @return A stable fallback value for failed `operator[]` insertions.
   */
  static V& overflow_value() {
    static V value = V();
    return value;
  }

  /**
   * @brief Test whether a lower-bound candidate is equivalent to the queried key.
   * @param candidate Caller-supplied argument used by this operation.
   * @param query Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool lower_bound_key_matches(const K& candidate, const K& query) const {
    return !comp_(query, candidate);
  }

  /**
   * @brief Return first sorted index whose key is not ordered before `key`.
   * @param key Lookup or insertion key.
   * @return First sorted index whose key is not ordered before `key`.
   */
  size_type lower_bound_index(const K& key) const {
    size_type first = 0u;
    size_type last = data_.size();
    while (first != last) {
      const size_type mid = first + ((last - first) / 2u);
      if (comp_(data_[mid].first, key)) first = mid + 1u;
      else last = mid;
    }
    return first;
  }

  /**
   * @brief Return first sorted index whose key is ordered after `key`.
   * @param key Lookup or insertion key.
   * @return First sorted index whose key is ordered after `key`.
   */
  size_type upper_bound_index(const K& key) const {
    size_type first = 0u;
    size_type last = data_.size();
    while (first != last) {
      const size_type mid = first + ((last - first) / 2u);
      if (!comp_(key, data_[mid].first)) first = mid + 1u;
      else last = mid;
    }
    return first;
  }
};

/**
 * @brief Exchange two same-capacity flat maps through the member swap operation.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class K, class V, size_t N, class Compare>
inline void swap(flat_map<K, V, N, Compare>& lhs, flat_map<K, V, N, Compare>& rhs) {
  lhs.swap(rhs);
}

} // namespace sstl

#endif

