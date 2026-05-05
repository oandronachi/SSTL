/**
 * @file list.hpp
 * @brief Fixed-capacity doubly linked list backed by an inline node pool.
 *
 * Each live element occupies one node from the object's static pool. The list
 * never allocates externally, and same-container splice relinks nodes so
 * iterators to moved elements keep naming the same element.
 */
#ifndef SSTL_LIST_HPP
/** @def SSTL_LIST_HPP
 * @brief Include guard for list.hpp.
 */
#define SSTL_LIST_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "utility.hpp"

namespace sstl {

/** @brief Fixed-capacity doubly linked list backed by an inline node pool. */
template <class T, size_t N>
class list {
  template <class, size_t> friend class list;

  /** @brief Pool node containing one value plus previous/next links. */
  struct node {
    /** @brief Raw storage for the list element while `used` is true. */
    raw_storage<T, 1> value;
    /** @brief Previous node in the list, or null for the head. */
    node* prev;
    /** @brief Next node in the list, or null for the tail. */
    node* next;
    /** @brief Indicates whether this pool slot currently owns a live value. */
    bool used;
    /**
     * @brief Return the mutable value constructed in this node.
     * @return The mutable value constructed in this node.
     */
    T& get() { return *value.ptr(0); }
    /**
     * @brief Return the const value constructed in this node.
     * @return The const value constructed in this node.
     */
    const T& get() const { return *value.ptr(0); }
  };

public:
  /** @brief Bidirectional iterator over live nodes in list order. */
  class iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef T value_type;
    /** @brief Mutable reference returned by dereference. */
    typedef T& reference;
    /** @brief Mutable pointer returned by arrow access. */
    typedef T* pointer;
    /** @brief Signed distance type used by iterator traits. */
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef bidirectional_iterator_tag iterator_category;

/**
 * @brief Construct an iterator object while initializing its owner and position state.
 * @param n Requested count or size.
 * @param tail Caller-supplied argument used by this operation.
 */
    iterator(node* n = 0, node* tail = 0) : n_(n), tail_(tail) {}
/**
 * @brief Dereference this iterator or wrapper to access the current value.
 * @return Result described by the function brief.
 */
    T& operator*() const { return n_->get(); }
/**
 * @brief Return a pointer to the current value exposed by this iterator or wrapper.
 * @return A pointer to the current value exposed by this iterator or wrapper.
 */
    T* operator->() const { return &n_->get(); }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    iterator& operator++() { if (n_) n_ = n_->next; return *this; }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
/**
 * @brief Move this iterator to the previous element.
 * @return Result described by the function brief.
 */
    iterator& operator--() { n_ = n_ ? n_->prev : tail_; return *this; }
/**
 * @brief Move this iterator to the previous element.
 * @return Result described by the function brief.
 */
    iterator operator--(int) { iterator old(*this); --*this; return old; }
/**
 * @brief Compare two iterators or values for equality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator==(const iterator& other) const { return n_ == other.n_; }
/**
 * @brief Compare two iterators or values for inequality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator!=(const iterator& other) const { return n_ != other.n_; }

  private:
    template <class, size_t> friend class list;
    /** @brief Current node, or null for `end()`. */
    node* n_;
    /** @brief Cached tail node used to implement decrement from `end()`. */
    node* tail_;
  };

  /** @brief Const bidirectional iterator over live nodes in list order. */
  class const_iterator {
  public:
    /** @brief Value type exposed by this const iterator. */
    typedef T value_type;
    /** @brief Const reference returned by dereference. */
    typedef const T& reference;
    /** @brief Const pointer returned by arrow access. */
    typedef const T* pointer;
    /** @brief Signed distance type used by iterator traits. */
    typedef int difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef bidirectional_iterator_tag iterator_category;

/**
 * @brief Construct a const_iterator object while initializing its owner and position state.
 * @param n Requested count or size.
 * @param tail Caller-supplied argument used by this operation.
 */
    const_iterator(const node* n = 0, const node* tail = 0) : n_(n), tail_(tail) {}
/**
 * @brief Construct a const_iterator object while initializing its owner and position state.
 * @param it Caller-supplied argument used by this operation.
 */
    const_iterator(const iterator& it) : n_(it.n_), tail_(it.tail_) {}
/**
 * @brief Dereference this iterator or wrapper to access the current value.
 * @return Result described by the function brief.
 */
    const T& operator*() const { return n_->get(); }
/**
 * @brief Return a pointer to the current value exposed by this iterator or wrapper.
 * @return A pointer to the current value exposed by this iterator or wrapper.
 */
    const T* operator->() const { return &n_->get(); }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    const_iterator& operator++() { if (n_) n_ = n_->next; return *this; }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
/**
 * @brief Move this iterator to the previous element.
 * @return Result described by the function brief.
 */
    const_iterator& operator--() { n_ = n_ ? n_->prev : tail_; return *this; }
/**
 * @brief Move this iterator to the previous element.
 * @return Result described by the function brief.
 */
    const_iterator operator--(int) { const_iterator old(*this); --*this; return old; }
/**
 * @brief Compare two iterators or values for equality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator==(const const_iterator& other) const { return n_ == other.n_; }
/**
 * @brief Compare two iterators or values for inequality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator!=(const const_iterator& other) const { return n_ != other.n_; }

  private:
    template <class, size_t> friend class list;
    /** @brief Current node, or null for `end()`. */
    const node* n_;
    /** @brief Cached tail node used to implement decrement from `end()`. */
    const node* tail_;
  };

  /** @brief Element type stored by the fixed-capacity list. */
  typedef T value_type;
  /** @brief Unsigned size and index type used by the list. */
  typedef size_t size_type;

/** @brief Construct a list object while initializing its fixed inline storage state. */
  list() : head_(0), tail_(0), free_head_(0), size_(0u) { reset_free_list(); }

/**
 * @brief Construct a list object while initializing its fixed inline storage state.
 * @param other Other object participating in the operation.
 */
  list(const list& other) : head_(0), tail_(0), free_head_(0), size_(0u) {
    reset_free_list();
    for (const_iterator it = other.begin(); it != other.end(); ++it) push_back(*it);
  }

/** @brief Destroy list and release any live inline-owned values. */
  ~list() { clear(); }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  list& operator=(const list& other) {
    if (this != &other) {
      clear();
      for (const_iterator it = other.begin(); it != other.end(); ++it) push_back(*it);
    }
    return *this;
  }

/**
 * @brief Return the number of live elements.
 * @return The number of live elements.
 */
  size_type size() const { return size_; }
/**
 * @brief Return the fixed compile-time element capacity without requiring an object.
 * @return The fixed compile-time element capacity without requiring an object.
 */
  static size_type capacity() { return N; }
/**
 * @brief Return the fixed compile-time element capacity.
 * @return The fixed compile-time element capacity.
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
 * @brief Return an iterator to the first element.
 * @return An iterator to the first element.
 */
  iterator begin() { return iterator(head_, tail_); }
/**
 * @brief Return an iterator one past the final element.
 * @return An iterator one past the final element.
 */
  iterator end() { return iterator(0, tail_); }
/**
 * @brief Return an iterator to the first element.
 * @return An iterator to the first element.
 */
  const_iterator begin() const { return const_iterator(head_, tail_); }
/**
 * @brief Return an iterator one past the final element.
 * @return An iterator one past the final element.
 */
  const_iterator end() const { return const_iterator(0, tail_); }
/**
 * @brief Access the first element.
 * @return Result described by the function brief.
 */
  T& front() { return head_->get(); }
/**
 * @brief Access the first element.
 * @return Result described by the function brief.
 */
  const T& front() const { return head_->get(); }
/**
 * @brief Access the last element.
 * @return Result described by the function brief.
 */
  T& back() { return tail_->get(); }
/**
 * @brief Access the last element.
 * @return Result described by the function brief.
 */
  const T& back() const { return tail_->get(); }
/**
 * @brief Return a pointer to the first element, or null when empty.
 * @return A pointer to the first element, or null when empty.
 */
  T* try_front() { return head_ ? &head_->get() : 0; }
/**
 * @brief Return a const pointer to the first element, or null when empty.
 * @return A const pointer to the first element, or null when empty.
 */
  const T* try_front() const { return head_ ? &head_->get() : 0; }
/**
 * @brief Return a pointer to the last element, or null when empty.
 * @return A pointer to the last element, or null when empty.
 */
  T* try_back() { return tail_ ? &tail_->get() : 0; }
/**
 * @brief Return a const pointer to the last element, or null when empty.
 * @return A const pointer to the last element, or null when empty.
 */
  const T* try_back() const { return tail_ ? &tail_->get() : 0; }

/**
 * @brief Validate that a mutable iterator names a linked node in this list or this list's end.
 * @param it Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool is_valid_iterator(iterator it) const {
    return (!it.n_ && it.tail_ == tail_) || owns_node(it.n_);
  }

/**
 * @brief Validate that a const iterator names a linked node in this list or this list's end.
 * @param it Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool is_valid_iterator(const_iterator it) const {
    return (!it.n_ && it.tail_ == tail_) || owns_node(it.n_);
  }

/**
 * @brief Insert a value at the front of the sequence.
 * @param x Element value supplied by the caller.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool push_front(const T& x) {
    node* n = allocate(x);
    if (!n) return false;
    link_before(head_, n);
    ++size_;
    return true;
  }

/**
 * @brief Try to insert at the front without invoking the active error policy.
 * @param x Element value supplied by the caller.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_push_front(const T& x) {
    if (full()) return false;
    return push_front(x);
  }

/**
 * @brief Insert a value at the back of the sequence.
 * @param x Element value supplied by the caller.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool push_back(const T& x) {
    node* n = allocate(x);
    if (!n) return false;
    link_before(0, n);
    ++size_;
    return true;
  }

/**
 * @brief Try to insert at the back without invoking the active error policy.
 * @param x Element value supplied by the caller.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_push_back(const T& x) {
    if (full()) return false;
    return push_back(x);
  }

  /**
   * @brief Insert `x` before `pos`.
   * @param pos Zero-based logical position.
   * @param x Element value supplied by the caller.
   * @return Result described by the function brief.
   */
  iterator insert(iterator pos, const T& x) {
    node* n = allocate(x);
    if (!n) return end();
    link_before(pos.n_, n);
    ++size_;
    return iterator(n, tail_);
  }

  /**
   * @brief Try to insert before `pos` without invoking the active error policy.
   * @param pos Zero-based logical position.
   * @param x Element value supplied by the caller.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  iterator try_insert(iterator pos, const T& x) {
    if (full()) return end();
    return insert(pos, x);
  }

  /**
   * @brief Erase the element named by `pos`.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  iterator erase(iterator pos) {
    if (!pos.n_) return end();
    node* next = pos.n_->next;
    unlink_and_destroy(pos.n_);
    return iterator(next, tail_);
  }

/** @brief Remove the front element. */
  void pop_front() { (void)try_pop_front(0); }
/** @brief Remove the back element. */
  void pop_back() { (void)try_pop_back(0); }

/**
 * @brief Remove the front element and report success instead of relying on policy handling.
 * @param out Caller-provided destination for produced values.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_pop_front(T* out) {
    if (!head_) return false;
    if (out) *out = head_->get();
    unlink_and_destroy(head_);
    return true;
  }

/**
 * @brief Remove the back element and report success instead of relying on policy handling.
 * @param out Caller-provided destination for produced values.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_pop_back(T* out) {
    if (!tail_) return false;
    if (out) *out = tail_->get();
    unlink_and_destroy(tail_);
    return true;
  }

/** @brief Destroy or remove all live elements while preserving inline capacity. */
  void clear() {
    node* n = head_;
    while (n) {
      node* next = n->next;
      SSTL_DESTROY_AT(n->value.ptr(0));
      release_destroyed_node(n);
      n = next;
    }
    head_ = 0;
    tail_ = 0;
    size_ = 0u;
  }

/**
 * @brief Transfer elements between fixed-capacity lists.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool splice(iterator pos, list& other) {
    if (this == &other || other.empty()) return true;
    if (N - size_ < other.size_) return false;
    for (iterator it = other.begin(); it != other.end(); ++it) {
      if (!insert_value_before(pos.n_, *it)) return false; // LCOV_EXCL_BR_LINE
    }
    other.clear();
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring all elements from another list.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_splice(iterator pos, list& other) { return splice(pos, other); }

/**
 * @brief Transfer elements between fixed-capacity lists.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M>
  bool splice(iterator pos, list<T, M>& other) {
    if (N - size_ < other.size_) return false;
    for (typename list<T, M>::iterator it = other.begin(); it != other.end(); ++it) {
      if (!insert_value_before(pos.n_, *it)) return false; // LCOV_EXCL_BR_LINE
    }
    other.clear();
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring all elements from another capacity list.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M>
  bool try_splice(iterator pos, list<T, M>& other) { return splice(pos, other); }

/**
 * @brief Transfer elements between fixed-capacity lists.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param it Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool splice(iterator pos, list& other, iterator it) {
    if (it == other.end()) return true;
    if (this == &other) {
      if (pos.n_ == it.n_ || (it.n_ && pos.n_ == it.n_->next)) return true;
      node* moving = it.n_;
      other.unlink_only(moving);
      link_before(pos.n_, moving);
      return true;
    }
    if (full()) return false;
    T value = *it;
    if (!insert_value_before(pos.n_, value)) return false; // LCOV_EXCL_BR_LINE
    other.unlink_and_destroy(it.n_);
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring one element from another list.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param it Caller-supplied argument used by this operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_splice(iterator pos, list& other, iterator it) { return splice(pos, other, it); }

/**
 * @brief Transfer elements between fixed-capacity lists.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param it Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M>
  bool splice(iterator pos, list<T, M>& other, typename list<T, M>::iterator it) {
    if (it == other.end() || full()) return it == other.end();
    T value = *it;
    if (!insert_value_before(pos.n_, value)) return false; // LCOV_EXCL_BR_LINE
    other.unlink_and_destroy(it.n_);
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring one element from another capacity list.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param it Caller-supplied argument used by this operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M>
  bool try_splice(iterator pos, list<T, M>& other, typename list<T, M>::iterator it) {
    return splice(pos, other, it);
  }

/**
 * @brief Transfer the half-open range `[first,last)` between fixed-capacity lists.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool splice(iterator pos, list& other, iterator first, iterator last) {
    if (first == last) return true;
    if (this == &other) {
      if (pos.n_ == last.n_) return true;
      for (node* scan = first.n_; scan != last.n_; scan = scan->next) {
        if (scan == pos.n_) return true;
      }
      node* first_node = first.n_;
      node* last_node = last.n_;
      node* range_last = last_node ? last_node->prev : tail_;
      node* before_range = first_node->prev;
      if (before_range) before_range->next = last_node;
      else head_ = last_node;
      if (last_node) last_node->prev = before_range;
      else tail_ = before_range;

      if (!head_) {
        head_ = first_node;
        tail_ = range_last;
        first_node->prev = 0;
        range_last->next = 0;
      } else if (!pos.n_) {
        first_node->prev = tail_;
        tail_->next = first_node;
        range_last->next = 0;
        tail_ = range_last;
      } else {
        node* before_pos = pos.n_->prev;
        first_node->prev = before_pos;
        range_last->next = pos.n_;
        if (before_pos) before_pos->next = first_node;
        else head_ = first_node;
        pos.n_->prev = range_last;
      }
      return true;
    }

    size_type count = 0u;
    for (node* scan = first.n_; scan != last.n_; scan = scan->next) ++count;
    if (N - size_ < count) return false;
    for (node* scan = first.n_; scan != last.n_; scan = scan->next) {
      if (!insert_value_before(pos.n_, scan->get())) return false; // LCOV_EXCL_BR_LINE
    }
    node* current = first.n_;
    while (current != last.n_) {
      node* next = current->next;
      other.unlink_and_destroy(current);
      current = next;
    }
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring a same-capacity range from another list.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_splice(iterator pos, list& other, iterator first, iterator last) {
    return splice(pos, other, first, last);
  }

/**
 * @brief Transfer the half-open range `[first,last)` from another-capacity list.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M>
  bool splice(iterator pos, list<T, M>& other, typename list<T, M>::iterator first,
              typename list<T, M>::iterator last) {
    if (first == last) return true;
    size_type count = 0u;
    for (typename list<T, M>::node* scan = first.n_; scan != last.n_; scan = scan->next) ++count;
    if (N - size_ < count) return false;
    for (typename list<T, M>::node* scan = first.n_; scan != last.n_; scan = scan->next) {
      if (!insert_value_before(pos.n_, scan->get())) return false; // LCOV_EXCL_BR_LINE
    }
    typename list<T, M>::node* current = first.n_;
    while (current != last.n_) {
      typename list<T, M>::node* next = current->next;
      other.unlink_and_destroy(current);
      current = next;
    }
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring an another-capacity range from another list.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M>
  bool try_splice(iterator pos, list<T, M>& other, typename list<T, M>::iterator first,
                  typename list<T, M>::iterator last) {
    return splice(pos, other, first, last);
  }

/** @brief Reverse element order in place. */
  void reverse() {
    node* n = head_;
    while (n) {
      node* next = n->next;
      n->next = n->prev;
      n->prev = next;
      n = next;
    }
    node* old_head = head_;
    head_ = tail_;
    tail_ = old_head;
  }

/**
 * @brief Remove elements equal to the supplied value.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 */
  void remove(const T& value) {
    node* n = head_;
    while (n) {
      node* next = n->next;
      if (n->get() == value) unlink_and_destroy(n);
      n = next;
    }
  }

/**
 * @brief Remove elements satisfying the supplied predicate.
 * @param pred Predicate used to test elements.
 */
  template <class Pred>
  void remove_if(Pred pred) {
    node* n = head_;
    while (n) {
      node* next = n->next;
      if (pred(n->get())) unlink_and_destroy(n);
      n = next;
    }
  }

/** @brief Collapse consecutive duplicate elements. */
  void unique() {
    if (!head_) return;
    node* n = head_->next;
    node* prev = head_;
    while (n) {
      node* next = n->next;
      if (prev->get() == n->get()) unlink_and_destroy(n);
      else prev = n;
      n = next;
    }
  }

/**
 * @brief Sort the container contents with a stable linked merge sort and no heap allocation.
 * @param comp Strict weak ordering used for comparisons.
 */
  template <class Compare>
  void sort(Compare comp) {
    if (size_ < 2u) return;
    head_ = merge_sort_nodes(head_, comp);
    rebuild_tail_links();
  }

/** @brief Sort the container contents with default ascending ordering. */
  void sort() { sort(less<T>()); }

/**
 * @brief Merge sorted input from another fixed-capacity list.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <class Compare>
  bool merge(list& other, Compare comp) {
    if (this == &other) return true;
    return merge_destination_owned(other, comp);
  }

/**
 * @brief Merge sorted input from another fixed-capacity list.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool merge(list& other) { return merge(other, less<T>()); }

/**
 * @brief Always-RETURN alias for merging sorted input from another list.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <class Compare>
  bool try_merge(list& other, Compare comp) { return merge(other, comp); }

/**
 * @brief Always-RETURN alias for merging sorted input from another list.
 * @param other Other object participating in the operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_merge(list& other) { return merge(other); }

/**
 * @brief Merge sorted input from another fixed-capacity list.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M, class Compare>
  bool merge(list<T, M>& other, Compare comp) {
    return merge_destination_owned(other, comp);
  }

/**
 * @brief Merge sorted input from another fixed-capacity list.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M>
  bool merge(list<T, M>& other) { return merge(other, less<T>()); }

/**
 * @brief Always-RETURN alias for merging sorted input from another capacity list.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M, class Compare>
  bool try_merge(list<T, M>& other, Compare comp) { return merge(other, comp); }

/**
 * @brief Always-RETURN alias for merging sorted input from another capacity list.
 * @param other Other object participating in the operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M>
  bool try_merge(list<T, M>& other) { return merge(other); }

/**
 * @brief Exchange contents with another same-capacity list without external allocation.
 * @param other Other object participating in the operation.
 */
  void swap(list& other) {
    if (this == &other) return;
    list tmp(*this);
    *this = other;
    other = tmp;
  }

private:
  /** @brief Physical node-pool size, with one dummy slot for `N == 0`. */
  enum { slot_count = N == 0 ? 1 : N };
  /** @brief Inline node pool containing every possible list element. */
  node nodes_[slot_count];
  /** @brief First live node, or null when the list is empty. */
  node* head_;
  /** @brief Last live node, or null when the list is empty. */
  node* tail_;
  /** @brief Head of the intrusive free-list used for O(1) node allocation. */
  node* free_head_;
  /** @brief Number of live nodes linked into the list. */
  size_type size_;

/** @brief Reset every pool slot into the intrusive free-list. */
  void reset_free_list() {
    free_head_ = slot_count == 0 ? 0 : &nodes_[0];
    for (unsigned i = 0u; i != slot_count; ++i) {
      nodes_[i].prev = 0;
      nodes_[i].next = (i + 1u != slot_count) ? &nodes_[i + 1u] : 0;
      nodes_[i].used = false;
    }
  }

/**
 * @brief Return true when a node pointer is one of this list's currently linked nodes.
 * @param candidate Caller-supplied argument used by this operation.
 * @return True when a node pointer is one of this list's currently linked nodes.
 */
  bool owns_node(const node* candidate) const {
    for (const node* n = head_; n; n = n->next) {
      if (n == candidate) return true;
    }
    return false;
  }

/**
 * @brief Split a singly reachable node chain into two near-even halves.
 * @param first Start of the half-open range.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
  static node* split_after_middle(node* first) {
    node* slow = first;
    node* fast = first ? first->next : 0; // LCOV_EXCL_BR_LINE
    while (fast && fast->next) {
      slow = slow->next;
      fast = fast->next->next;
    }
    if (!slow) return 0; // LCOV_EXCL_BR_LINE
    node* second = slow->next;
    slow->next = 0;
    if (second) second->prev = 0; // LCOV_EXCL_BR_LINE
    return second;
  }

/**
 * @brief Merge two already-sorted node chains while preserving equivalent-element order.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @param comp Strict weak ordering used for comparisons.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
  template <class Compare>
  static node* merge_sorted_nodes(node* a, node* b, Compare comp) {
    node* head = 0;
    node* tail = 0;
    while (a || b) {
      node* take = 0;
      if (!b || (a && !comp(b->get(), a->get()))) {
        take = a;
        a = a->next;
      } else {
        take = b;
        b = b->next;
      }
      take->prev = tail;
      if (tail) tail->next = take;
      else head = take;
      tail = take;
    }
    if (tail) tail->next = 0; // LCOV_EXCL_BR_LINE
    return head;
  }

/**
 * @brief Append an existing or newly allocated node to a rebuilt doubly linked chain.
 * @param chain_head Caller-supplied argument used by this operation.
 * @param chain_tail Caller-supplied argument used by this operation.
 * @param n Requested count or size.
 */
  static void append_relinked_node(node*& chain_head, node*& chain_tail, node* n) {
    n->prev = chain_tail;
    n->next = 0;
    if (chain_tail) chain_tail->next = n;
    else chain_head = n;
    chain_tail = n;
  }

/**
 * @brief Merge sorted source values into this list in one pass using destination-owned nodes.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M, class Compare>
  bool merge_destination_owned(list<T, M>& other, Compare comp) {
    if (N - size_ < other.size_) return false;
    node* a = head_;
    typename list<T, M>::node* b = other.head_;
    node* merged_head = 0;
    node* merged_tail = 0;
    while (a || b) {
      if (!b || (a && !comp(b->get(), a->get()))) {
        node* next = a->next;
        append_relinked_node(merged_head, merged_tail, a);
        a = next;
      } else {
        typename list<T, M>::node* next = b->next;
        node* copied = allocate(b->get());
        if (!copied) return false; // LCOV_EXCL_BR_LINE
        ++size_;
        append_relinked_node(merged_head, merged_tail, copied);
        b = next;
      }
    }
    head_ = merged_head;
    tail_ = merged_tail;
    other.clear();
    return true;
  }

/**
 * @brief Recursively sort a node chain using allocation-free linked merge sort.
 * @param first Start of the half-open range.
 * @param comp Strict weak ordering used for comparisons.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
  template <class Compare>
  static node* merge_sort_nodes(node* first, Compare comp) {
    if (!first || !first->next) return first; // LCOV_EXCL_BR_LINE
    node* second = split_after_middle(first);
    first = merge_sort_nodes(first, comp);
    second = merge_sort_nodes(second, comp);
    return merge_sorted_nodes(first, second, comp);
  }

/** @brief Rebuild backward links and the tail pointer after linked-list reordering. */
  void rebuild_tail_links() {
    node* previous = 0;
    tail_ = 0;
    for (node* n = head_; n; n = n->next) {
      n->prev = previous;
      tail_ = n;
      previous = n;
    }
  }

/**
 * @brief Reserve a free inline node slot and construct a value inside it.
 * @param x Element value supplied by the caller.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
  node* allocate(const T& x) {
    if (full()) {
      handle_error("list::allocate full");
      return 0;
    }
    node* n = free_head_;
    if (!n) return 0; // LCOV_EXCL_LINE
    free_head_ = n->next;
    SSTL_CONSTRUCT_AT(n->value.ptr(0), x);
    n->prev = 0;
    n->next = 0;
    n->used = true;
    return n;
  }

/**
 * @brief Return a destroyed pool node to the intrusive free-list.
 * @param n Requested count or size.
 */
  void release_destroyed_node(node* n) {
    n->prev = 0;
    n->next = free_head_;
    n->used = false;
    free_head_ = n;
  }

/**
 * @brief Link a node before the requested position.
 * @param pos Zero-based logical position.
 * @param n Requested count or size.
 */
  void link_before(node* pos, node* n) {
    if (!head_) {
      head_ = n;
      tail_ = n;
      n->prev = 0;
      n->next = 0;
      return;
    }
    if (!pos) {
      n->prev = tail_;
      n->next = 0;
      tail_->next = n;
      tail_ = n;
      return;
    }
    n->prev = pos->prev;
    n->next = pos;
    if (pos->prev) pos->prev->next = n;
    else head_ = n;
    pos->prev = n;
  }

/**
 * @brief Allocate a node, link it before the requested position, and update size bookkeeping.
 * @param pos Zero-based logical position.
 * @param x Element value supplied by the caller.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool insert_value_before(node* pos, const T& x) {
    node* n = allocate(x);
    if (!n) return false; // LCOV_EXCL_BR_LINE
    link_before(pos, n);
    ++size_;
    return true;
  }

/**
 * @brief Detach a node from its list without destroying its value.
 * @param n Requested count or size.
 */
  void unlink_only(node* n) {
    if (n->prev) n->prev->next = n->next;
    else head_ = n->next;
    if (n->next) n->next->prev = n->prev;
    else tail_ = n->prev;
    n->prev = 0;
    n->next = 0;
  }

/**
 * @brief Detach a node, destroy its value, and return the slot to the pool.
 * @param n Requested count or size.
 */
  void unlink_and_destroy(node* n) {
    unlink_only(n);
    SSTL_DESTROY_AT(n->value.ptr(0));
    release_destroyed_node(n);
    --size_;
  }
};

/**
 * @brief Exchange two same-capacity lists through the member swap operation.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class T, size_t N>
inline void swap(list<T, N>& lhs, list<T, N>& rhs) {
  lhs.swap(rhs);
}

} // namespace sstl

#endif

