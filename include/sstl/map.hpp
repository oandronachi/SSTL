/**
 * @file map.hpp
 * @brief Fixed-capacity ordered map backed by an inline red/black tree.
 *
 * Nodes are drawn from a static pool owned by the map object. Insert and find
 * follow tree links rather than reshuffling a contiguous array, so iterators to
 * existing nodes remain stable across successful insertions.
 */
#ifndef SSTL_MAP_HPP
/** @def SSTL_MAP_HPP
 * @brief Include guard for map.hpp.
 */
#define SSTL_MAP_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "utility.hpp"

namespace sstl {

/** @brief Fixed-capacity ordered map implemented as an inline red/black tree. */
template <class K, class V, size_t N, class Compare = less<K> >
class map {
  /** @brief Pool node containing one constructed key/value pair plus red/black links. */
  struct node {
    /** @brief Raw storage for the key/value pair while `used` is true. */
    raw_storage<pair<K, V>, 1> value;
    /** @brief Parent node index, or -1 when this node is the root or unused. */
    int parent;
    /** @brief Left-child node index, or -1 when no left child exists. */
    int left;
    /** @brief Right-child node index, or -1 when no right child exists. */
    int right;
    /** @brief Next node index in the unused-slot free-list, or -1 when unlinked. */
    int next_free;
    /** @brief Red/black color bit; true means red, false means black. */
    bool red;
    /** @brief Indicates whether this pool slot currently owns a live value. */
    bool used;
    /** @brief Monotonic slot generation used to reject stale iterators after reuse. */
    unsigned generation;
    /**
     * @brief Return the mutable key/value pair constructed in this node.
     * @return The mutable key/value pair constructed in this node.
     */
    pair<K, V>& get() { return *value.ptr(0); }
    /**
     * @brief Return the const key/value pair constructed in this node.
     * @return The const key/value pair constructed in this node.
     */
    const pair<K, V>& get() const { return *value.ptr(0); }
  };

public:
  /** @brief Key/value pair type stored in the ordered tree. */
  typedef pair<K, V> value_type;
  /** @brief Unsigned size and index type used by the map. */
  typedef size_t size_type;

  /** @brief Bidirectional iterator over ordered key/value pairs. */
  class iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef pair<K, V> value_type;
    /** @brief Mutable reference returned by dereference. */
    typedef value_type& reference;
    /** @brief Mutable pointer returned by arrow access. */
    typedef value_type* pointer;
    /** @brief Signed distance type used by iterator traits. */
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef bidirectional_iterator_tag iterator_category;

/**
 * @brief Construct an iterator object while initializing its owner and position state.
 * @param owner Caller-supplied argument used by this operation.
 * @param index Caller-supplied argument used by this operation.
 */
    iterator(map* owner = 0, int index = -1)
        : owner_(owner), index_(index), generation_(owner ? owner->iterator_generation(index) : 0u) {}
/**
 * @brief Dereference this iterator or wrapper to access the current value.
 * @return Result described by the function brief.
 */
    value_type& operator*() const { return owner_->nodes_[static_cast<unsigned>(index_)].get(); }
/**
 * @brief Return a pointer to the current value exposed by this iterator or wrapper.
 * @return A pointer to the current value exposed by this iterator or wrapper.
 */
    value_type* operator->() const { return &owner_->nodes_[static_cast<unsigned>(index_)].get(); }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    iterator& operator++() {
      index_ = owner_ ? owner_->successor(index_) : -1;
      generation_ = owner_ ? owner_->iterator_generation(index_) : 0u;
      return *this;
    }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
/**
 * @brief Move this iterator to the previous element.
 * @return Result described by the function brief.
 */
    iterator& operator--() {
      index_ = owner_ ? owner_->predecessor(index_) : -1;
      generation_ = owner_ ? owner_->iterator_generation(index_) : 0u;
      return *this;
    }
/**
 * @brief Post-decrement and return the previous iterator value.
 * @return Result described by the function brief.
 */
    iterator operator--(int) { iterator old(*this); --*this; return old; }
/**
 * @brief Compare two iterators or values for equality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator==(const iterator& other) const {
      return owner_ == other.owner_ && index_ == other.index_ && generation_ == other.generation_;
    }
/**
 * @brief Compare two iterators or values for inequality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator!=(const iterator& other) const { return !(*this == other); }

  private:
    template <class, class, size_t, class> friend class map;
    friend class const_iterator;
    /** @brief Owning map used to resolve node indices during iterator operations. */
    map* owner_;
    /** @brief Current node index, or -1 for the end iterator. */
    int index_;
    /** @brief Slot generation captured when this iterator was created or advanced. */
    unsigned generation_;
  };

  /** @brief Const bidirectional iterator over ordered key/value pairs. */
  class const_iterator {
  public:
    /** @brief Value type exposed by this const iterator. */
    typedef pair<K, V> value_type;
    /** @brief Const reference returned by dereference. */
    typedef const value_type& reference;
    /** @brief Const pointer returned by arrow access. */
    typedef const value_type* pointer;
    /** @brief Signed distance type used by iterator traits. */
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef bidirectional_iterator_tag iterator_category;

/**
 * @brief Construct a const_iterator object while initializing its owner and position state.
 * @param owner Caller-supplied argument used by this operation.
 * @param index Caller-supplied argument used by this operation.
 */
    const_iterator(const map* owner = 0, int index = -1)
        : owner_(owner), index_(index), generation_(owner ? owner->iterator_generation(index) : 0u) {}
/**
 * @brief Construct a const_iterator object while initializing its owner and position state.
 * @param it Caller-supplied argument used by this operation.
 */
    const_iterator(const iterator& it) : owner_(it.owner_), index_(it.index_), generation_(it.generation_) {}
/**
 * @brief Dereference this iterator or wrapper to access the current value.
 * @return Result described by the function brief.
 */
    const value_type& operator*() const { return owner_->nodes_[static_cast<unsigned>(index_)].get(); }
/**
 * @brief Return a pointer to the current value exposed by this iterator or wrapper.
 * @return A pointer to the current value exposed by this iterator or wrapper.
 */
    const value_type* operator->() const { return &owner_->nodes_[static_cast<unsigned>(index_)].get(); }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    const_iterator& operator++() {
      index_ = owner_ ? owner_->successor(index_) : -1;
      generation_ = owner_ ? owner_->iterator_generation(index_) : 0u;
      return *this;
    }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
/**
 * @brief Move this iterator to the previous element.
 * @return Result described by the function brief.
 */
    const_iterator& operator--() {
      index_ = owner_ ? owner_->predecessor(index_) : -1;
      generation_ = owner_ ? owner_->iterator_generation(index_) : 0u;
      return *this;
    }
/**
 * @brief Post-decrement and return the previous iterator value.
 * @return Result described by the function brief.
 */
    const_iterator operator--(int) { const_iterator old(*this); --*this; return old; }
/**
 * @brief Compare two iterators or values for equality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator==(const const_iterator& other) const {
      return owner_ == other.owner_ && index_ == other.index_ && generation_ == other.generation_;
    }
/**
 * @brief Compare two iterators or values for inequality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator!=(const const_iterator& other) const { return !(*this == other); }

  private:
    template <class, class, size_t, class> friend class map;
    /** @brief Owning map used to resolve node indices during const iterator operations. */
    const map* owner_;
    /** @brief Current node index, or -1 for the end iterator. */
    int index_;
    /** @brief Slot generation captured when this iterator was created or advanced. */
    unsigned generation_;
  };

/** @brief Construct a map object while initializing its fixed inline storage state. */
  map() : root_(-1), free_head_(-1), size_(0u) {
    for (unsigned i = 0u; i != node_slots; ++i) {
      nodes_[i].generation = 0u;
      reset_node_metadata(i);
    }
    reset_free_list();
  }

/**
 * @brief Construct a map object while initializing its fixed inline storage state.
 * @param other Other object participating in the operation.
 */
  map(const map& other) : root_(-1), free_head_(-1), size_(0u) {
    for (unsigned i = 0u; i != node_slots; ++i) {
      nodes_[i].generation = 0u;
      reset_node_metadata(i);
    }
    reset_free_list();
    for (const_iterator it = other.begin(); it != other.end(); ++it) insert(*it);
  }

/** @brief Destroy map and release any live inline-owned values. */
  ~map() { clear(); }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  map& operator=(const map& other) {
    if (this != &other) {
      clear();
      for (const_iterator it = other.begin(); it != other.end(); ++it) insert(*it);
    }
    return *this;
  }

/**
 * @brief Return an iterator to the first element.
 * @return An iterator to the first element.
 */
  iterator begin() { return iterator(this, minimum(root_)); }
/**
 * @brief Return an iterator one past the final element.
 * @return An iterator one past the final element.
 */
  iterator end() { return iterator(this, -1); }
/**
 * @brief Return an iterator to the first element.
 * @return An iterator to the first element.
 */
  const_iterator begin() const { return const_iterator(this, minimum(root_)); }
/**
 * @brief Return an iterator one past the final element.
 * @return An iterator one past the final element.
 */
  const_iterator end() const { return const_iterator(this, -1); }
/**
 * @brief Return the number of live elements.
 * @return The number of live elements.
 */
  size_type size() const { return size_; }
/**
 * @brief Return the fixed compile-time capacity without requiring an object.
 * @return The fixed compile-time capacity without requiring an object.
 */
  static size_type capacity() { return N; }
/**
 * @brief Return the largest number of key/value pairs this fixed-capacity map can hold.
 * @return The largest number of key/value pairs this fixed-capacity map can hold.
 */
  size_type max_size() const { return N; }
/**
 * @brief Report whether no elements are live.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool empty() const { return size_ == 0u; }
/**
 * @brief Report whether the fixed capacity is exhausted.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool full() const { return size_ == N; }

/**
 * @brief Find an element by key and return the container sentinel when absent.
 * @param key Lookup or insertion key.
 * @return Result described by the function brief.
 */
  iterator find(const K& key) {
    int current = root_;
    while (current >= 0) {
      const K& node_key = nodes_[static_cast<unsigned>(current)].get().first;
      if (comp_(key, node_key)) current = nodes_[static_cast<unsigned>(current)].left;
      else if (comp_(node_key, key)) current = nodes_[static_cast<unsigned>(current)].right;
      else return iterator(this, current);
    }
    return end();
  }

/**
 * @brief Find an element by key and return the container sentinel when absent.
 * @param key Lookup or insertion key.
 * @return Result described by the function brief.
 */
  const_iterator find(const K& key) const {
    int current = root_;
    while (current >= 0) {
      const K& node_key = nodes_[static_cast<unsigned>(current)].get().first;
      if (comp_(key, node_key)) current = nodes_[static_cast<unsigned>(current)].left;
      else if (comp_(node_key, key)) current = nodes_[static_cast<unsigned>(current)].right;
      else return const_iterator(this, current);
    }
    return end();
  }

/**
 * @brief Return one when `key` is present and zero otherwise.
 * @param key Lookup or insertion key.
 * @return One when `key` is present and zero otherwise.
 */
  size_type count(const K& key) const { return find(key) == end() ? 0u : 1u; }

/**
 * @brief Return the first iterator whose key is not less than `key`.
 * @param key Lookup or insertion key.
 * @return The first iterator whose key is not less than `key`.
 */
  iterator lower_bound(const K& key) {
    int current = root_;
    int result = -1;
    while (current >= 0) {
      const K& node_key = nodes_[static_cast<unsigned>(current)].get().first;
      if (!comp_(node_key, key)) {
        result = current;
        current = nodes_[static_cast<unsigned>(current)].left;
      } else {
        current = nodes_[static_cast<unsigned>(current)].right;
      }
    }
    return iterator(this, result);
  }

/**
 * @brief Return the first const iterator whose key is not less than `key`.
 * @param key Lookup or insertion key.
 * @return The first const iterator whose key is not less than `key`.
 */
  const_iterator lower_bound(const K& key) const {
    int current = root_;
    int result = -1;
    while (current >= 0) {
      const K& node_key = nodes_[static_cast<unsigned>(current)].get().first;
      if (!comp_(node_key, key)) {
        result = current;
        current = nodes_[static_cast<unsigned>(current)].left;
      } else {
        current = nodes_[static_cast<unsigned>(current)].right;
      }
    }
    return const_iterator(this, result);
  }

/**
 * @brief Return the first iterator whose key is greater than `key`.
 * @param key Lookup or insertion key.
 * @return The first iterator whose key is greater than `key`.
 */
  iterator upper_bound(const K& key) {
    int current = root_;
    int result = -1;
    while (current >= 0) {
      const K& node_key = nodes_[static_cast<unsigned>(current)].get().first;
      if (comp_(key, node_key)) {
        result = current;
        current = nodes_[static_cast<unsigned>(current)].left;
      } else {
        current = nodes_[static_cast<unsigned>(current)].right;
      }
    }
    return iterator(this, result);
  }

/**
 * @brief Return the first const iterator whose key is greater than `key`.
 * @param key Lookup or insertion key.
 * @return The first const iterator whose key is greater than `key`.
 */
  const_iterator upper_bound(const K& key) const {
    int current = root_;
    int result = -1;
    while (current >= 0) {
      const K& node_key = nodes_[static_cast<unsigned>(current)].get().first;
      if (comp_(key, node_key)) {
        result = current;
        current = nodes_[static_cast<unsigned>(current)].left;
      } else {
        current = nodes_[static_cast<unsigned>(current)].right;
      }
    }
    return const_iterator(this, result);
  }

/**
 * @brief Return the half-open iterator range containing keys equal to `key`.
 * @param key Lookup or insertion key.
 * @return The half-open iterator range containing keys equal to `key`.
 */
  pair<iterator, iterator> equal_range(const K& key) { return make_pair(lower_bound(key), upper_bound(key)); }

/**
 * @brief Return the const half-open iterator range containing keys equal to `key`.
 * @param key Lookup or insertion key.
 * @return The const half-open iterator range containing keys equal to `key`.
 */
  pair<const_iterator, const_iterator> equal_range(const K& key) const {
    return make_pair(lower_bound(key), upper_bound(key));
  }

/**
 * @brief Insert a key/value pair when the key is absent and report iterator plus success flag.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  pair<iterator, bool> insert(const value_type& value) {
    int parent = -1;
    int current = root_;
    bool went_left = false;
    while (current >= 0) {
      parent = current;
      const K& node_key = nodes_[static_cast<unsigned>(current)].get().first;
      if (comp_(value.first, node_key)) {
        went_left = true;
        current = nodes_[static_cast<unsigned>(current)].left;
      } else if (comp_(node_key, value.first)) {
        went_left = false;
        current = nodes_[static_cast<unsigned>(current)].right;
      } else {
        return make_pair(iterator(this, current), false);
      }
    }

    if (full()) {
      handle_error("map::insert full");
      return make_pair(end(), false);
    }

    const int inserted = allocate_node(value);
    nodes_[static_cast<unsigned>(inserted)].parent = parent;
    if (parent < 0) root_ = inserted;
    else if (went_left) nodes_[static_cast<unsigned>(parent)].left = inserted;
    else nodes_[static_cast<unsigned>(parent)].right = inserted;
    fix_after_insert(inserted);
    ++size_;
    return make_pair(iterator(this, inserted), true);
  }

  /**
   * @brief Erase one element by iterator.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  iterator erase(iterator pos) {
    if (pos.index_ < 0 || !is_valid_iterator(pos)) {
      handle_error("map::erase iterator");
      return end();
    }
    return iterator(this, erase_index(pos.index_));
  }

  /**
   * @brief Erase one element by key and report whether anything was removed.
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
 * @brief Return the mapped value for the key, inserting a default value when necessary.
 * @param key Lookup or insertion key.
 * @return The mapped value for the key, inserting a default value when necessary.
 */
  V& operator[](const K& key) {
    iterator it = find(key);
    if (it != end()) return it->second;
    pair<iterator, bool> inserted = insert(make_pair(key, V()));
    if (inserted.second) return inserted.first->second;
    handle_error("map::operator[] full");
    return overflow_value();
  }

/**
 * @brief Return whether the iterator belongs to this container and currently names a valid slot.
 * @param it Caller-supplied argument used by this operation.
 * @return Whether the iterator belongs to this container and currently names a valid slot.
 */
  bool is_valid_iterator(iterator it) const {
    return it.owner_ == this &&
           ((it.index_ == -1 && it.generation_ == 0u) ||
            (it.index_ >= 0 && static_cast<unsigned>(it.index_) < node_slots &&
             nodes_[static_cast<unsigned>(it.index_)].used &&
             nodes_[static_cast<unsigned>(it.index_)].generation == it.generation_));
  }

/**
 * @brief Return whether the const iterator belongs to this container and currently names a valid slot.
 * @param it Caller-supplied argument used by this operation.
 * @return Whether the const iterator belongs to this container and currently names a valid slot.
 */
  bool is_valid_iterator(const_iterator it) const {
    return it.owner_ == this &&
           ((it.index_ == -1 && it.generation_ == 0u) ||
            (it.index_ >= 0 && static_cast<unsigned>(it.index_) < node_slots &&
             nodes_[static_cast<unsigned>(it.index_)].used &&
             nodes_[static_cast<unsigned>(it.index_)].generation == it.generation_));
  }

/** @brief Destroy or remove all live elements while preserving inline capacity. */
  void clear() {
    for (unsigned i = 0u; i != node_slots; ++i) {
      if (nodes_[i].used) SSTL_DESTROY_AT(nodes_[i].value.ptr(0));
      reset_node_metadata(i);
    }
    root_ = -1;
    size_ = 0u;
    reset_free_list();
  }

/**
 * @brief Exchange contents with another same-capacity map without external allocation.
 * @param other Other object participating in the operation.
 */
  void swap(map& other) {
    if (this == &other) return;
    map tmp(*this);
    *this = other;
    other = tmp;
  }

private:
  /** @brief Physical node-pool size, with one dummy slot for `N == 0`. */
  enum { node_slots = N == 0 ? 1 : N };
  /** @brief Inline node pool containing all possible tree nodes. */
  node nodes_[node_slots];
  /** @brief Root node index, or -1 when the tree is empty. */
  int root_;
  /** @brief First unused node index in the O(1) allocation free-list. */
  int free_head_;
  /** @brief Number of live key/value pairs in the tree. */
  size_type size_;
  /** @brief Key ordering predicate used for search, insertion, and balancing decisions. */
  Compare comp_;

/**
 * @brief Reset tree-node bookkeeping without linking the slot into the free-list.
 * @param i Zero-based logical index.
 */
  void reset_node_metadata(unsigned i) {
    bump_generation(i);
    nodes_[i].parent = -1;
    nodes_[i].left = -1;
    nodes_[i].right = -1;
    nodes_[i].next_free = -1;
    nodes_[i].red = false;
    nodes_[i].used = false;
  }

/** @brief Rebuild the unused-slot free-list over all fixed-capacity public slots. */
  void reset_free_list() {
    free_head_ = N == 0u ? -1 : 0;
    for (unsigned i = 0u; i != node_slots; ++i) {
      nodes_[i].next_free = (i + 1u < N) ? static_cast<int>(i + 1u) : -1;
    }
  }

/**
 * @brief Advance a slot generation, avoiding zero so end iterators remain distinct.
 * @param i Zero-based logical index.
 */
  void bump_generation(unsigned i) {
    ++nodes_[i].generation;
    if (nodes_[i].generation == 0u) ++nodes_[i].generation; // LCOV_EXCL_BR_LINE
  }

/**
 * @brief Return the generation value captured by an iterator for a node index.
 * @param index Caller-supplied argument used by this operation.
 * @return The generation value captured by an iterator for a node index.
 */
  unsigned iterator_generation(int index) const {
    return (index >= 0 && static_cast<unsigned>(index) < node_slots && nodes_[static_cast<unsigned>(index)].used)
               ? nodes_[static_cast<unsigned>(index)].generation
               : 0u;
  }

/**
 * @brief Reserve a free inline tree node and construct a key/value pair inside it.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return Result described by the function brief.
 */
  int allocate_node(const value_type& value) {
    if (free_head_ < 0) return -1; // LCOV_EXCL_LINE
    const unsigned i = static_cast<unsigned>(free_head_);
    free_head_ = nodes_[i].next_free;
    SSTL_CONSTRUCT_AT(nodes_[i].value.ptr(0), value);
    nodes_[i].parent = -1;
    nodes_[i].left = -1;
    nodes_[i].right = -1;
    nodes_[i].next_free = -1;
    nodes_[i].red = true;
    nodes_[i].used = true;
    return static_cast<int>(i);
  }

/**
 * @brief Return an erased inline tree node to the O(1) free-list.
 * @param i Zero-based logical index.
 */
  void release_node(unsigned i) {
    reset_node_metadata(i);
    nodes_[i].next_free = free_head_;
    free_head_ = static_cast<int>(i);
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
 * @brief Return whether the tree node at an index is red.
 * @param index Caller-supplied argument used by this operation.
 * @return Whether the tree node at an index is red.
 */
  bool is_red(int index) const {
    return index >= 0 && nodes_[static_cast<unsigned>(index)].red;
  }

/**
 * @brief Set the red/black color of a tree node when it exists.
 * @param index Caller-supplied argument used by this operation.
 * @param red Caller-supplied argument used by this operation.
 */
  void set_red(int index, bool red) {
    if (index >= 0) nodes_[static_cast<unsigned>(index)].red = red;
  }

/**
 * @brief Return the parent node index or -1 for null.
 * @param index Caller-supplied argument used by this operation.
 * @return The parent node index or -1 for null.
 */
  int parent_of(int index) const {
    return index >= 0 ? nodes_[static_cast<unsigned>(index)].parent : -1;
  }

/**
 * @brief Return the left child node index or -1 for null.
 * @param index Caller-supplied argument used by this operation.
 * @return The left child node index or -1 for null.
 */
  int left_of(int index) const {
    return index >= 0 ? nodes_[static_cast<unsigned>(index)].left : -1;
  }

/**
 * @brief Return the right child node index or -1 for null.
 * @param index Caller-supplied argument used by this operation.
 * @return The right child node index or -1 for null.
 */
  int right_of(int index) const {
    return index >= 0 ? nodes_[static_cast<unsigned>(index)].right : -1;
  }

/**
 * @brief Return the smallest node index in a subtree.
 * @param index Caller-supplied argument used by this operation.
 * @return The smallest node index in a subtree.
 */
  int minimum(int index) const {
    if (index < 0) return -1;
    while (nodes_[static_cast<unsigned>(index)].left >= 0) index = nodes_[static_cast<unsigned>(index)].left;
    return index;
  }

/**
 * @brief Return the largest node index in a subtree.
 * @param index Caller-supplied argument used by this operation.
 * @return The largest node index in a subtree.
 */
  int maximum(int index) const {
    if (index < 0) return -1;
    while (nodes_[static_cast<unsigned>(index)].right >= 0) index = nodes_[static_cast<unsigned>(index)].right;
    return index;
  }

/**
 * @brief Return the in-order successor node index.
 * @param index Caller-supplied argument used by this operation.
 * @return The in-order successor node index.
 */
  int successor(int index) const {
    if (index < 0) return -1;
    if (nodes_[static_cast<unsigned>(index)].right >= 0) return minimum(nodes_[static_cast<unsigned>(index)].right);
    int parent = nodes_[static_cast<unsigned>(index)].parent;
    while (parent >= 0 && index == nodes_[static_cast<unsigned>(parent)].right) {
      index = parent;
      parent = nodes_[static_cast<unsigned>(parent)].parent;
    }
    return parent;
  }

/**
 * @brief Return the in-order predecessor node index.
 * @param index Caller-supplied argument used by this operation.
 * @return The in-order predecessor node index.
 */
  int predecessor(int index) const {
    if (index < 0) return maximum(root_);
    if (nodes_[static_cast<unsigned>(index)].left >= 0) return maximum(nodes_[static_cast<unsigned>(index)].left);
    int parent = nodes_[static_cast<unsigned>(index)].parent;
    while (parent >= 0 && index == nodes_[static_cast<unsigned>(parent)].left) {
      index = parent;
      parent = nodes_[static_cast<unsigned>(parent)].parent;
    }
    return parent;
  }

/**
 * @brief Replace one tree subtree root with another during erase.
 * @param old_root Caller-supplied argument used by this operation.
 * @param new_root Caller-supplied argument used by this operation.
 */
  void transplant(int old_root, int new_root) {
    const int parent = nodes_[static_cast<unsigned>(old_root)].parent;
    if (parent < 0) root_ = new_root;
    else if (old_root == nodes_[static_cast<unsigned>(parent)].left) nodes_[static_cast<unsigned>(parent)].left = new_root;
    else nodes_[static_cast<unsigned>(parent)].right = new_root;
    if (new_root >= 0) nodes_[static_cast<unsigned>(new_root)].parent = parent;
  }

/**
 * @brief Erase a tree node by pool index and return the successor index.
 * @param z Caller-supplied argument used by this operation.
 * @return Result described by the function brief.
 */
  int erase_index(int z) {
    int result = successor(z);
    int y = z;
    if (nodes_[static_cast<unsigned>(z)].left >= 0 && nodes_[static_cast<unsigned>(z)].right >= 0) {
      y = result;
      nodes_[static_cast<unsigned>(z)].get() = nodes_[static_cast<unsigned>(y)].get();
      bump_generation(static_cast<unsigned>(z));
      result = z;
    }

    const int x = nodes_[static_cast<unsigned>(y)].left >= 0
                ? nodes_[static_cast<unsigned>(y)].left
                : nodes_[static_cast<unsigned>(y)].right;
    const int parent = nodes_[static_cast<unsigned>(y)].parent;
    const bool y_was_red = nodes_[static_cast<unsigned>(y)].red;
    transplant(y, x);
    if (!y_was_red) fix_after_erase(x, parent);
    SSTL_DESTROY_AT(nodes_[static_cast<unsigned>(y)].value.ptr(0));
    release_node(static_cast<unsigned>(y));
    --size_;
    if (size_ == 0u) root_ = -1;
    return result;
  }

/**
 * @brief Restore red/black invariants after node removal.
 * @param x Element value supplied by the caller.
 * @param parent Caller-supplied argument used by this operation.
 */
  void fix_after_erase(int x, int parent) {
    while (x != root_ && !is_red(x)) {
      if (parent < 0) break; // LCOV_EXCL_BR_LINE
      if (x == nodes_[static_cast<unsigned>(parent)].left) {
        int sibling = nodes_[static_cast<unsigned>(parent)].right;
        if (is_red(sibling)) {
          set_red(sibling, false);
          set_red(parent, true);
          rotate_left(parent);
          sibling = nodes_[static_cast<unsigned>(parent)].right;
        }
        if (!is_red(left_of(sibling)) && !is_red(right_of(sibling))) {
          set_red(sibling, true);
          x = parent;
          parent = parent_of(x);
        } else {
          if (!is_red(right_of(sibling))) {
            set_red(left_of(sibling), false);
            set_red(sibling, true);
            rotate_right(sibling);
            sibling = nodes_[static_cast<unsigned>(parent)].right;
          }
          set_red(sibling, is_red(parent));
          set_red(parent, false);
          set_red(right_of(sibling), false);
          rotate_left(parent);
          x = root_;
          parent = -1;
        }
      } else {
        int sibling = nodes_[static_cast<unsigned>(parent)].left;
        if (is_red(sibling)) {
          set_red(sibling, false);
          set_red(parent, true);
          rotate_right(parent);
          sibling = nodes_[static_cast<unsigned>(parent)].left;
        }
        if (!is_red(right_of(sibling)) && !is_red(left_of(sibling))) {
          set_red(sibling, true);
          x = parent;
          parent = parent_of(x);
        } else {
          if (!is_red(left_of(sibling))) {
            set_red(right_of(sibling), false);
            set_red(sibling, true);
            rotate_left(sibling);
            sibling = nodes_[static_cast<unsigned>(parent)].left;
          }
          set_red(sibling, is_red(parent));
          set_red(parent, false);
          set_red(left_of(sibling), false);
          rotate_right(parent);
          x = root_;
          parent = -1;
        }
      }
    }
    set_red(x, false);
  }

/**
 * @brief Rotate a tree subtree left around the supplied node.
 * @param x Element value supplied by the caller.
 */
  void rotate_left(int x) {
    const int y = nodes_[static_cast<unsigned>(x)].right;
    nodes_[static_cast<unsigned>(x)].right = nodes_[static_cast<unsigned>(y)].left;
    if (nodes_[static_cast<unsigned>(y)].left >= 0) {
      nodes_[static_cast<unsigned>(nodes_[static_cast<unsigned>(y)].left)].parent = x;
    }
    nodes_[static_cast<unsigned>(y)].parent = nodes_[static_cast<unsigned>(x)].parent;
    if (nodes_[static_cast<unsigned>(x)].parent < 0) root_ = y;
    else if (x == nodes_[static_cast<unsigned>(nodes_[static_cast<unsigned>(x)].parent)].left) {
      nodes_[static_cast<unsigned>(nodes_[static_cast<unsigned>(x)].parent)].left = y;
    } else {
      nodes_[static_cast<unsigned>(nodes_[static_cast<unsigned>(x)].parent)].right = y;
    }
    nodes_[static_cast<unsigned>(y)].left = x;
    nodes_[static_cast<unsigned>(x)].parent = y;
  }

/**
 * @brief Rotate a tree subtree right around the supplied node.
 * @param x Element value supplied by the caller.
 */
  void rotate_right(int x) {
    const int y = nodes_[static_cast<unsigned>(x)].left;
    nodes_[static_cast<unsigned>(x)].left = nodes_[static_cast<unsigned>(y)].right;
    if (nodes_[static_cast<unsigned>(y)].right >= 0) {
      nodes_[static_cast<unsigned>(nodes_[static_cast<unsigned>(y)].right)].parent = x;
    }
    nodes_[static_cast<unsigned>(y)].parent = nodes_[static_cast<unsigned>(x)].parent;
    if (nodes_[static_cast<unsigned>(x)].parent < 0) root_ = y;
    else if (x == nodes_[static_cast<unsigned>(nodes_[static_cast<unsigned>(x)].parent)].right) {
      nodes_[static_cast<unsigned>(nodes_[static_cast<unsigned>(x)].parent)].right = y;
    } else {
      nodes_[static_cast<unsigned>(nodes_[static_cast<unsigned>(x)].parent)].left = y;
    }
    nodes_[static_cast<unsigned>(y)].right = x;
    nodes_[static_cast<unsigned>(x)].parent = y;
  }

/**
 * @brief Restore red/black invariants after insertion.
 * @param z Caller-supplied argument used by this operation.
 */
  void fix_after_insert(int z) {
    while (z != root_ && is_red(nodes_[static_cast<unsigned>(z)].parent)) {
      const int parent = nodes_[static_cast<unsigned>(z)].parent;
      const int grand = nodes_[static_cast<unsigned>(parent)].parent;
      if (parent == nodes_[static_cast<unsigned>(grand)].left) {
        const int uncle = nodes_[static_cast<unsigned>(grand)].right;
        if (is_red(uncle)) {
          nodes_[static_cast<unsigned>(parent)].red = false;
          nodes_[static_cast<unsigned>(uncle)].red = false;
          nodes_[static_cast<unsigned>(grand)].red = true;
          z = grand;
        } else {
          if (z == nodes_[static_cast<unsigned>(parent)].right) {
            z = parent;
            rotate_left(z);
          }
          const int new_parent = nodes_[static_cast<unsigned>(z)].parent;
          const int new_grand = nodes_[static_cast<unsigned>(new_parent)].parent;
          nodes_[static_cast<unsigned>(new_parent)].red = false;
          nodes_[static_cast<unsigned>(new_grand)].red = true;
          rotate_right(new_grand);
        }
      } else {
        const int uncle = nodes_[static_cast<unsigned>(grand)].left;
        if (is_red(uncle)) {
          nodes_[static_cast<unsigned>(parent)].red = false;
          nodes_[static_cast<unsigned>(uncle)].red = false;
          nodes_[static_cast<unsigned>(grand)].red = true;
          z = grand;
        } else {
          if (z == nodes_[static_cast<unsigned>(parent)].left) {
            z = parent;
            rotate_right(z);
          }
          const int new_parent = nodes_[static_cast<unsigned>(z)].parent;
          const int new_grand = nodes_[static_cast<unsigned>(new_parent)].parent;
          nodes_[static_cast<unsigned>(new_parent)].red = false;
          nodes_[static_cast<unsigned>(new_grand)].red = true;
          rotate_left(new_grand);
        }
      }
    }
    nodes_[static_cast<unsigned>(root_)].red = false;
  }
};

/**
 * @brief Exchange two same-capacity ordered maps through the member swap operation.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class K, class V, size_t N, class Compare>
inline void swap(map<K, V, N, Compare>& lhs, map<K, V, N, Compare>& rhs) {
  lhs.swap(rhs);
}

} // namespace sstl

#endif

