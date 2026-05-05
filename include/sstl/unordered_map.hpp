/**
 * @file unordered_map.hpp
 * @brief Fixed-capacity hash table map with static bucket and node arrays.
 *
 * Buckets store indices into an inline node pool. Collisions are handled by
 * singly linked chains, so the `B` bucket-count parameter affects distribution
 * and is observable through `bucket_count()`.
 */
#ifndef SSTL_UNORDERED_MAP_HPP
/** @def SSTL_UNORDERED_MAP_HPP
 * @brief Include guard for unordered_map.hpp.
 */
#define SSTL_UNORDERED_MAP_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "utility.hpp"

namespace sstl {

/** @brief Fixed-capacity chained hash map with inline node and bucket arrays. */
template <class K, class V, size_t N, size_t B = next_prime_ge<N>::value, class H = hash<K>, class Eq = equal_to<K> >
class unordered_map {
  /** @brief Node stored in the inline pool for one key/value pair and chain link. */
  struct node {
    /** @brief Raw storage containing the live key/value pair while `used` is true. */
    raw_storage<pair<K, V>, 1> value;
    /** @brief Index of the next node in the bucket chain, or -1 for end of chain. */
    int next;
    /** @brief Indicates whether this node slot currently owns a live value. */
    bool used;
    /** @brief Monotonic slot generation used to reject stale iterators after reuse. */
    unsigned generation;
    /**
     * @brief Return a mutable reference to the constructed key/value pair.
     * @return A mutable reference to the constructed key/value pair.
     */
    pair<K, V>& get() { return *value.ptr(0); }
    /**
     * @brief Return a const reference to the constructed key/value pair.
     * @return A const reference to the constructed key/value pair.
     */
    const pair<K, V>& get() const { return *value.ptr(0); }
  };

public:
  /** @brief Stored key/value pair type. */
  typedef pair<K, V> value_type;
  /** @brief Unsigned size and index type used by the hash map. */
  typedef size_t size_type;

  class const_iterator;

  /** @brief Forward iterator over occupied hash-map node slots. */
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

    /**
     * @brief Construct an iterator bound to `owner` and a physical node slot.
     * @param owner Caller-supplied argument used by this operation.
     * @param index Caller-supplied argument used by this operation.
     */
    iterator(unordered_map* owner = 0, size_type index = 0u)
        : owner_(owner), index_(index), generation_(owner ? owner->iterator_generation(index) : 0u) {}
    /**
     * @brief Dereference the current occupied slot.
     * @return Result described by the function brief.
     */
    value_type& operator*() const { return owner_->nodes_[index_].get(); }
    /**
     * @brief Return a pointer to the current occupied slot.
     * @return A pointer to the current occupied slot.
     */
    value_type* operator->() const { return &owner_->nodes_[index_].get(); }
    /**
     * @brief Advance to the next occupied slot or `end()`.
     * @return Result described by the function brief.
     */
    iterator& operator++() {
      if (owner_) index_ = owner_->next_used(index_ + 1u);
      generation_ = owner_ ? owner_->iterator_generation(index_) : 0u;
      return *this;
    }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
    /**
     * @brief Compare owner and slot index for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const iterator& other) const {
      return owner_ == other.owner_ && index_ == other.index_ && generation_ == other.generation_;
    }
    /**
     * @brief Compare owner and slot index for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const iterator& other) const { return !(*this == other); }

  private:
    template <class, class, size_t, size_t, class, class> friend class unordered_map;
    friend class const_iterator;
    /** @brief Owning unordered_map used to dereference and advance. */
    unordered_map* owner_;
    /** @brief Physical node slot, or `node_slots` for `end()`. */
    size_type index_;
    /** @brief Slot generation captured when this iterator was created or advanced. */
    unsigned generation_;
  };

  /** @brief Forward const iterator over occupied hash-map node slots. */
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

    /**
     * @brief Construct a const iterator bound to `owner` and a physical node slot.
     * @param owner Caller-supplied argument used by this operation.
     * @param index Caller-supplied argument used by this operation.
     */
    const_iterator(const unordered_map* owner = 0, size_type index = 0u)
        : owner_(owner), index_(index), generation_(owner ? owner->iterator_generation(index) : 0u) {}
    /**
     * @brief Convert a mutable iterator to a const iterator.
     * @param it Caller-supplied argument used by this operation.
     */
    const_iterator(const iterator& it) : owner_(it.owner_), index_(it.index_), generation_(it.generation_) {}
    /**
     * @brief Dereference the current occupied slot.
     * @return Result described by the function brief.
     */
    const value_type& operator*() const { return owner_->nodes_[index_].get(); }
    /**
     * @brief Return a pointer to the current occupied slot.
     * @return A pointer to the current occupied slot.
     */
    const value_type* operator->() const { return &owner_->nodes_[index_].get(); }
    /**
     * @brief Advance to the next occupied slot or `end()`.
     * @return Result described by the function brief.
     */
    const_iterator& operator++() {
      if (owner_) index_ = owner_->next_used(index_ + 1u);
      generation_ = owner_ ? owner_->iterator_generation(index_) : 0u;
      return *this;
    }
    /**
     * @brief Post-increment and return the previous iterator value.
     * @return Result described by the function brief.
     */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
    /**
     * @brief Compare owner and slot index for equality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator==(const const_iterator& other) const {
      return owner_ == other.owner_ && index_ == other.index_ && generation_ == other.generation_;
    }
    /**
     * @brief Compare owner and slot index for inequality.
     * @param other Other object participating in the operation.
     * @return `true` when the documented condition holds; otherwise `false`.
     */
    bool operator!=(const const_iterator& other) const { return !(*this == other); }

  private:
    template <class, class, size_t, size_t, class, class> friend class unordered_map;
    /** @brief Owning unordered_map used to dereference and advance. */
    const unordered_map* owner_;
    /** @brief Physical node slot, or `node_slots` for `end()`. */
    size_type index_;
    /** @brief Slot generation captured when this iterator was created or advanced. */
    unsigned generation_;
  };

  /** @brief Construct an empty hash map with all buckets and free slots reset. */
  unordered_map() : free_head_(-1), size_(0u) { reset_slots(); }

  /**
   * @brief Copy-construct by inserting each live pair from `other`.
   * @param other Other object participating in the operation.
   */
  unordered_map(const unordered_map& other) : free_head_(-1), size_(0u), hash_(other.hash_), eq_(other.eq_) {
    reset_slots();
    copy_from(other);
  }

  /** @brief Destroy all live key/value pairs. */
  ~unordered_map() { clear(); }

  /**
   * @brief Replace this map with a deep copy of `other`.
   * @param other Other object participating in the operation.
   * @return Result described by the function brief.
   */
  unordered_map& operator=(const unordered_map& other) {
    if (this != &other) {
      clear();
      hash_ = other.hash_;
      eq_ = other.eq_;
      copy_from(other);
    }
    return *this;
  }

  /**
   * @brief Return the number of stored pairs.
   * @return The number of stored pairs.
   */
  size_type size() const { return size_; }
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
   * @brief Report whether no pairs are stored.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return size_ == 0u; }
  /**
   * @brief Report whether insertion capacity is exhausted.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool full() const { return size_ == N; }
  /**
   * @brief Return the compile-time bucket count.
   * @return The compile-time bucket count.
   */
  size_type bucket_count() const { return B; }
  /**
   * @brief Return the current load factor as `size() / bucket_count()`.
   * @return The current load factor as `size() / bucket_count()`.
   */
  float load_factor() const { return B == 0u ? 0.0f : static_cast<float>(size_) / static_cast<float>(B); }

  /**
   * @brief Return an iterator to the first occupied node, or `end()`.
   * @return An iterator to the first occupied node, or `end()`.
   */
  iterator begin() { return iterator(this, next_used(0u)); }
  /**
   * @brief Return a sentinel iterator one past all occupied node slots.
   * @return A sentinel iterator one past all occupied node slots.
   */
  iterator end() { return iterator(this, node_slots); }
  /**
   * @brief Return a const iterator to the first occupied node, or `end()`.
   * @return A const iterator to the first occupied node, or `end()`.
   */
  const_iterator begin() const { return const_iterator(this, next_used(0u)); }
  /**
   * @brief Return a const sentinel iterator one past all occupied node slots.
   * @return A const sentinel iterator one past all occupied node slots.
   */
  const_iterator end() const { return const_iterator(this, node_slots); }

  /**
   * @brief Validate that a mutable iterator belongs to this map and names a live slot or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const {
    return it.owner_ == this &&
           it.index_ <= node_slots &&
           ((it.index_ == node_slots && it.generation_ == 0u) ||
            (it.index_ < node_slots && nodes_[it.index_].used && nodes_[it.index_].generation == it.generation_));
  }

  /**
   * @brief Validate that a const iterator belongs to this map and names a live slot or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(const_iterator it) const {
    return it.owner_ == this &&
           it.index_ <= node_slots &&
           ((it.index_ == node_slots && it.generation_ == 0u) ||
            (it.index_ < node_slots && nodes_[it.index_].used && nodes_[it.index_].generation == it.generation_));
  }

  /**
   * @brief Find a key and return an iterator to its pair, or `end()` when absent.
   * @param key Lookup or insertion key.
   * @return Result described by the function brief.
   */
  iterator find(const K& key) {
    if (B == 0u) return end();
    int idx = buckets_[bucket_for(key)];
    while (idx >= 0) {
      if (eq_(nodes_[static_cast<unsigned>(idx)].get().first, key)) return iterator(this, static_cast<unsigned>(idx));
      idx = nodes_[static_cast<unsigned>(idx)].next;
    }
    return end();
  }

  /**
   * @brief Find a key through a const map, returning `end()` when absent.
   * @param key Lookup or insertion key.
   * @return Result described by the function brief.
   */
  const_iterator find(const K& key) const {
    if (B == 0u) return end();
    int idx = buckets_[bucket_for(key)];
    while (idx >= 0) {
      if (eq_(nodes_[static_cast<unsigned>(idx)].get().first, key)) return const_iterator(this, static_cast<unsigned>(idx));
      idx = nodes_[static_cast<unsigned>(idx)].next;
    }
    return end();
  }

  /**
   * @brief Insert a pair into the appropriate hash bucket when key is absent.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  pair<iterator, bool> insert(const value_type& value) {
    iterator existing = find(value.first);
    if (existing != end()) return make_pair(existing, false);
    if (full() || B == 0u) {
      handle_error("unordered_map::insert full");
      return make_pair(end(), false);
    }
    const int slot = allocate_slot(value);
    if (slot < 0) return make_pair(end(), false); // LCOV_EXCL_BR_LINE
    const unsigned bucket = bucket_for(value.first);
    nodes_[static_cast<unsigned>(slot)].next = buckets_[bucket];
    buckets_[bucket] = slot;
    ++size_;
    return make_pair(iterator(this, static_cast<unsigned>(slot)), true);
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
    handle_error("unordered_map::operator[] full");
    return overflow_value();
  }

  /**
   * @brief Erase one element by key and return the number of removed pairs.
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
    if (pos.index_ >= node_slots || !is_valid_iterator(pos)) {
      handle_error("unordered_map::erase iterator");
      return end();
    }
    const size_type next = next_used(pos.index_ + 1u);
    unlink_bucket_node(pos.index_);
    SSTL_DESTROY_AT(nodes_[pos.index_].value.ptr(0));
    release_slot(pos.index_);
    --size_;
    return iterator(this, next);
  }

  /** @brief Destroy all stored pairs and reset buckets and node links. */
  void clear() {
    for (unsigned i = 0u; i != node_slots; ++i) {
      if (nodes_[i].used) {
        SSTL_DESTROY_AT(nodes_[i].value.ptr(0));
        nodes_[i].used = false;
        bump_generation(i);
      }
    }
    for (unsigned b = 0u; b != bucket_slots; ++b) buckets_[b] = -1;
    reset_free_list();
    size_ = 0u;
  }

  /**
   * @brief Exchange all fixed-capacity contents with another unordered map of the same type.
   * @param other Other object participating in the operation.
   */
  void swap(unordered_map& other) {
    if (this == &other) return;
    unordered_map tmp(*this);
    *this = other;
    other = tmp;
  }

private:
  /** @brief Physical pool sizes with one dummy slot for zero-capacity configurations. */
  enum { node_slots = N == 0 ? 1 : N, bucket_slots = B == 0 ? 1 : B };
  /** @brief Inline node pool containing all possible key/value pairs. */
  node nodes_[node_slots];
  /** @brief Bucket heads, each storing a node index or -1 for empty. */
  int buckets_[bucket_slots];
  /** @brief Head of the intrusive free-list threaded through unused node slots. */
  int free_head_;
  /** @brief Number of currently live key/value pairs. */
  size_type size_;
  /** @brief Hash functor used to select buckets. */
  H hash_;
  /** @brief Equality predicate used inside bucket chains. */
  Eq eq_;

  /** @brief Reset buckets and node metadata when no values are live. */
  void reset_slots() {
    for (unsigned i = 0u; i != bucket_slots; ++i) buckets_[i] = -1;
    for (unsigned i = 0u; i != node_slots; ++i) {
      nodes_[i].next = -1;
      nodes_[i].used = false;
      nodes_[i].generation = 1u;
    }
    reset_free_list();
  }

  /**
   * @brief Advance a slot generation, avoiding zero so end iterators remain distinct.
   * @param index Caller-supplied argument used by this operation.
   */
  void bump_generation(size_type index) {
    ++nodes_[index].generation;
    if (nodes_[index].generation == 0u) ++nodes_[index].generation; // LCOV_EXCL_BR_LINE
  }

  /**
   * @brief Return the generation value captured by an iterator for a node index.
   * @param index Caller-supplied argument used by this operation.
   * @return The generation value captured by an iterator for a node index.
   */
  unsigned iterator_generation(size_type index) const {
    return (index < node_slots && nodes_[index].used) ? nodes_[index].generation : 0u;
  }

  /**
   * @brief Deep-copy every live pair from `other` using normal insertion.
   * @param other Other object participating in the operation.
   */
  void copy_from(const unordered_map& other) {
    for (const_iterator it = other.begin(); it != other.end(); ++it) {
      (void)insert(*it);
    }
  }

  /**
   * @brief Return a stable fallback value for failed `operator[]` insertions.
   * @return A stable fallback value for failed `operator[]` insertions.
   */
  static V& overflow_value() {
    static V value = V();
    return value;
  }

  /**
   * @brief Return the first used physical slot at or after `start`, or `node_slots`.
   * @param start Caller-supplied argument used by this operation.
   * @return The first used physical slot at or after `start`, or `node_slots`.
   */
  size_type next_used(size_type start) const {
    for (size_type i = start; i != node_slots; ++i) {
      if (nodes_[i].used) return i;
    }
    return node_slots;
  }

  /**
   * @brief Convert a key to a bucket index, guarding zero-bucket configurations.
   * @param key Lookup or insertion key.
   * @return Result described by the function brief.
   */
  unsigned bucket_for(const K& key) const {
    return B == 0u ? 0u : static_cast<unsigned>(hash_(key) % B);
  }

  /** @brief Rebuild the free-list over all public node slots when no values are live. */
  void reset_free_list() {
    free_head_ = N == 0u ? -1 : 0;
    for (unsigned i = 0u; i != node_slots; ++i) {
      nodes_[i].next = (i + 1u < N) ? static_cast<int>(i + 1u) : -1;
    }
  }

  /**
   * @brief Return an erased node slot to the front of the intrusive free-list.
   * @param index Caller-supplied argument used by this operation.
   */
  void release_slot(size_type index) {
    nodes_[index].used = false;
    bump_generation(index);
    nodes_[index].next = free_head_;
    free_head_ = static_cast<int>(index);
  }

  /**
   * @brief Allocate a free node slot in O(1) and construct `value` inside it.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   * @return Result described by the function brief.
   */
  int allocate_slot(const value_type& value) {
    if (free_head_ < 0) return -1; // LCOV_EXCL_LINE
    const unsigned slot = static_cast<unsigned>(free_head_);
    free_head_ = nodes_[slot].next;
    SSTL_CONSTRUCT_AT(nodes_[slot].value.ptr(0), value);
    nodes_[slot].used = true;
    nodes_[slot].next = -1;
    return static_cast<int>(slot);
  }

  /**
   * @brief Remove a live node slot from its current bucket chain.
   * @param index Caller-supplied argument used by this operation.
   */
  void unlink_bucket_node(size_type index) {
    const unsigned bucket = bucket_for(nodes_[index].get().first);
    int current = buckets_[bucket];
    int previous = -1;
    while (current >= 0) {
      if (static_cast<size_type>(current) == index) {
        if (previous >= 0) nodes_[static_cast<size_type>(previous)].next = nodes_[index].next;
        else buckets_[bucket] = nodes_[index].next;
        return;
      }
      previous = current;
      current = nodes_[static_cast<size_type>(current)].next;
    }
  }
};

/**
 * @brief Exchange two same-capacity unordered maps through the member swap operation.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class K, class V, size_t N, size_t B, class H, class Eq>
inline void swap(unordered_map<K, V, N, B, H, Eq>& lhs,
                 unordered_map<K, V, N, B, H, Eq>& rhs) {
  lhs.swap(rhs);
}

} // namespace sstl

#endif

