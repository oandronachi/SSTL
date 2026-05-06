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
  /** @brief Key type stored by the flat map. */
  typedef K key_type;
  /** @brief Mapped value type stored by the flat map. */
  typedef V mapped_type;
  /** @brief Key comparison predicate type. */
  typedef Compare key_compare;
  /** @brief Key/value pair type stored in sorted contiguous order. */
  typedef pair<K, V> value_type;
  /** @brief Unsigned size and index type used by the flat map. */
  typedef size_t size_type;

  /** @brief Value comparison predicate that orders pairs by key. */
  class value_compare {
  public:
    /** @brief Compare two values by their keys. */
    bool operator()(const value_type& a, const value_type& b) const { return comp_(a.first, b.first); }
  private:
    /** @brief Allow flat_map to construct value_compare. */
    friend class flat_map;
    /** @brief Construct a value comparator from the key comparator. */
    explicit value_compare(key_compare comp) : comp_(comp) {}
    /** @brief Stored key comparator. */
    key_compare comp_;
  };

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
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef random_access_iterator_tag iterator_category;

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
    /** @brief Move to the previous entry. */
    iterator& operator--() { --index_; return *this; }
    /** @brief Post-decrement and return the previous iterator value. */
    iterator operator--(int) { iterator old(*this); --*this; return old; }
    /** @brief Advance by `n` entries. */
    iterator& operator+=(difference_type n) { index_ = static_cast<size_type>(static_cast<difference_type>(index_) + n); return *this; }
    /** @brief Retreat by `n` entries. */
    iterator& operator-=(difference_type n) { return *this += -n; }
    /** @brief Return an iterator advanced by `n` entries. */
    iterator operator+(difference_type n) const { iterator out(*this); out += n; return out; }
    /** @brief Return an iterator retreated by `n` entries. */
    iterator operator-(difference_type n) const { iterator out(*this); out -= n; return out; }
    /** @brief Return the logical distance from `other`. */
    difference_type operator-(const iterator& other) const { return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_); }
    /** @brief Access an entry relative to this iterator. */
    value_type& operator[](difference_type n) const { return *(*this + n); }
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
    /** @brief Compare logical positions within the same flat map. */
    bool operator<(const iterator& other) const { return owner_ == other.owner_ && index_ < other.index_; }
    /** @brief Compare logical positions within the same flat map. */
    bool operator<=(const iterator& other) const { return *this == other || *this < other; }
    /** @brief Compare logical positions within the same flat map. */
    bool operator>(const iterator& other) const { return other < *this; }
    /** @brief Compare logical positions within the same flat map. */
    bool operator>=(const iterator& other) const { return other <= *this; }

  private:
    /** @brief Allow the owning flat_map to construct and inspect mutable iterators. */
    friend class flat_map;
    /** @brief Allow const iterators to copy mutable iterator internals. */
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
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef random_access_iterator_tag iterator_category;

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
    /** @brief Move to the previous entry. */
    const_iterator& operator--() { --index_; return *this; }
    /** @brief Post-decrement and return the previous iterator value. */
    const_iterator operator--(int) { const_iterator old(*this); --*this; return old; }
    /** @brief Advance by `n` entries. */
    const_iterator& operator+=(difference_type n) { index_ = static_cast<size_type>(static_cast<difference_type>(index_) + n); return *this; }
    /** @brief Retreat by `n` entries. */
    const_iterator& operator-=(difference_type n) { return *this += -n; }
    /** @brief Return an iterator advanced by `n` entries. */
    const_iterator operator+(difference_type n) const { const_iterator out(*this); out += n; return out; }
    /** @brief Return an iterator retreated by `n` entries. */
    const_iterator operator-(difference_type n) const { const_iterator out(*this); out -= n; return out; }
    /** @brief Return the logical distance from `other`. */
    difference_type operator-(const const_iterator& other) const { return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_); }
    /** @brief Access an entry relative to this iterator. */
    const value_type& operator[](difference_type n) const { return *(*this + n); }
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
    /** @brief Compare logical positions within the same flat map. */
    bool operator<(const const_iterator& other) const { return owner_ == other.owner_ && index_ < other.index_; }
    /** @brief Compare logical positions within the same flat map. */
    bool operator<=(const const_iterator& other) const { return *this == other || *this < other; }
    /** @brief Compare logical positions within the same flat map. */
    bool operator>(const const_iterator& other) const { return other < *this; }
    /** @brief Compare logical positions within the same flat map. */
    bool operator>=(const const_iterator& other) const { return other <= *this; }

  private:
    /** @brief Allow the owning flat_map to construct and inspect const iterators. */
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

  /** @brief Mutable reverse iterator over sorted flat-map entries. */
  typedef reverse_iterator_adaptor<iterator> reverse_iterator;
  /** @brief Const reverse iterator over sorted flat-map entries. */
  typedef reverse_iterator_adaptor<const_iterator> const_reverse_iterator;

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
  /** @brief Return a const iterator to the first sorted pair. */
  const_iterator cbegin() const { return begin(); }
  /**
   * @brief Return a const iterator one past the last sorted pair.
   * @return A const iterator one past the last sorted pair.
   */
  const_iterator end() const { return const_iterator(this, data_.size(), generation_); }
  /** @brief Return a const iterator one past the last sorted pair. */
  const_iterator cend() const { return end(); }
  /** @brief Return a mutable reverse iterator to the final sorted pair. */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /** @brief Return a const reverse iterator to the final sorted pair. */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /** @brief Return a const reverse iterator to the final sorted pair. */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /** @brief Return a mutable reverse iterator one before the first sorted pair. */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /** @brief Return a const reverse iterator one before the first sorted pair. */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /** @brief Return a const reverse iterator one before the first sorted pair. */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }
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

  /** @brief Return the key comparison predicate. */
  key_compare key_comp() const { return comp_; }
  /** @brief Return the value comparison predicate. */
  value_compare value_comp() const { return value_compare(comp_); }
  /** @brief Return one when `key` is present and zero otherwise. */
  size_type count(const K& key) const { return find(key) == end() ? 0u : 1u; }

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

  /** @brief Insert a value using `hint` as a non-binding placement hint. */
  iterator insert(iterator hint, const value_type& value) {
    (void)hint;
    return insert(value).first;
  }

  /** @brief Insert every value in the half-open input range. */
  template <class InputIt>
  void insert(InputIt first, InputIt last) {
    for (; first != last; ++first) (void)insert(*first);
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
   * @brief Return the mapped value for an existing key without inserting.
   * @param key Lookup key.
   * @return Reference to the mapped value.
  */
  V& at(const K& key) {
    V* value = try_at(key);
    if (!value) return fail_reference<V>("flat_map::at"); // LCOV_EXCL_BR_LINE
    return *value;
  }

  /**
   * @brief Return the const mapped value for an existing key without inserting.
   * @param key Lookup key.
   * @return Const reference to the mapped value.
  */
  const V& at(const K& key) const {
    const V* value = try_at(key);
    if (!value) return fail_reference<const V>("flat_map::at"); // LCOV_EXCL_BR_LINE
    return *value;
  }

  /**
   * @brief Return the mapped value for an existing key, or null when absent.
   * @param key Lookup key.
   * @return Pointer to the mapped value, or null when `key` is absent.
   */
  V* try_at(const K& key) {
    iterator it = find(key);
    return it == end() ? 0 : &it->second;
  }

  /**
   * @brief Return the const mapped value for an existing key, or null when absent.
   * @param key Lookup key.
   * @return Const pointer to the mapped value, or null when `key` is absent.
   */
  const V* try_at(const K& key) const {
    const_iterator it = find(key);
    return it == end() ? 0 : &it->second;
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

  /** @brief Erase the half-open iterator range `[first,last)`. */
  iterator erase(iterator first, iterator last) {
    if (!is_valid_iterator(first) || !is_valid_iterator(last) || first.index_ > last.index_) {
      handle_error("flat_map::erase range");
      return end();
    }
    const size_type index = first.index_;
    while (first != last) {
      first = erase(first);
      last = iterator(this, last.index_ - 1u, generation_);
    }
    return iterator(this, index, generation_);
  }

  /** @brief Destroy all stored entries. */
  void clear() {
    data_.clear();
    ++generation_;
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
    if (inserted.second) return inserted.first->second; // LCOV_EXCL_BR_LINE
    return fail_reference<V>("flat_map::operator[] full"); // LCOV_EXCL_LINE
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

/** @brief Compare two flat maps for element-wise equality across capacities. */
template <class K, class V, size_t N, size_t M, class Compare>
inline bool operator==(const flat_map<K, V, N, Compare>& lhs, const flat_map<K, V, M, Compare>& rhs) {
  if (lhs.size() != rhs.size()) return false;
  typename flat_map<K, V, N, Compare>::const_iterator a = lhs.begin();
  typename flat_map<K, V, M, Compare>::const_iterator b = rhs.begin();
  for (; a != lhs.end(); ++a, ++b) {
    if (!(*a == *b)) return false;
  }
  return true;
}

/** @brief Compare two flat maps for inequality. */
template <class K, class V, size_t N, size_t M, class Compare>
inline bool operator!=(const flat_map<K, V, N, Compare>& lhs, const flat_map<K, V, M, Compare>& rhs) { return !(lhs == rhs); }

/** @brief Lexicographically compare two flat maps. */
template <class K, class V, size_t N, size_t M, class Compare>
inline bool operator<(const flat_map<K, V, N, Compare>& lhs, const flat_map<K, V, M, Compare>& rhs) {
  typename flat_map<K, V, N, Compare>::const_iterator a = lhs.begin();
  typename flat_map<K, V, M, Compare>::const_iterator b = rhs.begin();
  for (; a != lhs.end() && b != rhs.end(); ++a, ++b) {
    if (*a < *b) return true;
    if (*b < *a) return false;
  }
  return a == lhs.end() && b != rhs.end();
}

/** @brief Return true when `lhs` is not lexicographically greater than `rhs`. */
template <class K, class V, size_t N, size_t M, class Compare>
inline bool operator<=(const flat_map<K, V, N, Compare>& lhs, const flat_map<K, V, M, Compare>& rhs) { return !(rhs < lhs); }

/** @brief Return true when `lhs` is lexicographically greater than `rhs`. */
template <class K, class V, size_t N, size_t M, class Compare>
inline bool operator>(const flat_map<K, V, N, Compare>& lhs, const flat_map<K, V, M, Compare>& rhs) { return rhs < lhs; }

/** @brief Return true when `lhs` is not lexicographically less than `rhs`. */
template <class K, class V, size_t N, size_t M, class Compare>
inline bool operator>=(const flat_map<K, V, N, Compare>& lhs, const flat_map<K, V, M, Compare>& rhs) { return !(lhs < rhs); }

} // namespace sstl

#endif

