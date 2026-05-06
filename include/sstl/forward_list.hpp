/**
 * @file forward_list.hpp
 * @brief Fixed-capacity singly linked list with a static node pool.
 *
 * The container provides a real `before_begin()` sentinel instead of pointer
 * arithmetic outside an array. This keeps insert-after and splice-after
 * semantics explicit while preserving the no-heap storage model.
 */
#ifndef SSTL_FORWARD_LIST_HPP
/** @def SSTL_FORWARD_LIST_HPP
 * @brief Include guard for forward_list.hpp.
 */
#define SSTL_FORWARD_LIST_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "type_traits.hpp"

namespace sstl {

/** @brief Fixed-capacity singly linked list backed by an inline node pool. */
template <class T, size_t N>
class forward_list {
  /** @brief Allow forward_list specializations to access node and iterator internals. */
  template <class, size_t> friend class forward_list;

  /** @brief Pool node containing one value plus a forward link. */
  struct node {
    /** @brief Raw storage for the list element while `used` is true. */
    raw_storage<T, 1> value;
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
  /** @brief Forward iterator over live nodes in list order. */
  class iterator {
  public:
    /** @brief Value type exposed by this iterator. */
    typedef T value_type;
    /** @brief Mutable reference returned by dereference. */
    typedef T& reference;
    /** @brief Mutable pointer returned by arrow access. */
    typedef T* pointer;
    /** @brief Signed distance type used by iterator traits. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;

/**
 * @brief Construct an iterator object while initializing its owner and position state.
 * @param n Requested count or size.
 * @param before Caller-supplied argument used by this operation.
 */
    iterator(node* n = 0, bool before = false) : n_(n), before_(before) {}
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
    iterator& operator++() { if (n_) n_ = n_->next; before_ = false; return *this; }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    iterator operator++(int) { iterator old(*this); ++*this; return old; }
/**
 * @brief Compare two iterators or values for equality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator==(const iterator& other) const { return n_ == other.n_ && before_ == other.before_; }
/**
 * @brief Compare two iterators or values for inequality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator!=(const iterator& other) const { return !(*this == other); }

  private:
    /** @brief Allow forward_list specializations to construct and inspect mutable iterators. */
    template <class, size_t> friend class forward_list;
    /** @brief Current node, or null for `end()` and `before_begin()`. */
    node* n_;
    /** @brief True only for the synthetic position before the first element. */
    bool before_;
  };

  /** @brief Const forward iterator over live nodes in list order. */
  class const_iterator {
  public:
    /** @brief Value type exposed by this const iterator. */
    typedef T value_type;
    /** @brief Const reference returned by dereference. */
    typedef const T& reference;
    /** @brief Const pointer returned by arrow access. */
    typedef const T* pointer;
    /** @brief Signed distance type used by iterator traits. */
    typedef ptrdiff_t difference_type;
    /** @brief Iterator category advertised to generic algorithms. */
    typedef forward_iterator_tag iterator_category;

/**
 * @brief Construct a const_iterator object while initializing its owner and position state.
 * @param n Requested count or size.
 * @param before Caller-supplied argument used by this operation.
 */
    const_iterator(const node* n = 0, bool before = false) : n_(n), before_(before) {}
/**
 * @brief Construct a const_iterator object while initializing its owner and position state.
 * @param it Caller-supplied argument used by this operation.
 */
    const_iterator(const iterator& it) : n_(it.n_), before_(it.before_) {}
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
    const_iterator& operator++() { if (n_) n_ = n_->next; before_ = false; return *this; }
/**
 * @brief Advance this iterator to the next element.
 * @return Result described by the function brief.
 */
    const_iterator operator++(int) { const_iterator old(*this); ++*this; return old; }
/**
 * @brief Compare two iterators or values for equality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator==(const const_iterator& other) const { return n_ == other.n_ && before_ == other.before_; }
/**
 * @brief Compare two iterators or values for inequality.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
    bool operator!=(const const_iterator& other) const { return !(*this == other); }

  private:
    /** @brief Allow forward_list specializations to construct and inspect const iterators. */
    template <class, size_t> friend class forward_list;
    /** @brief Current node, or null for `end()` and `before_begin()`. */
    const node* n_;
    /** @brief True only for the synthetic position before the first element. */
    bool before_;
  };

  /** @brief Element type stored by the fixed-capacity forward list. */
  typedef T value_type;
  /** @brief Unsigned size and capacity type used by the forward list. */
  typedef size_t size_type;

/** @brief Construct a forward_list object while initializing its fixed inline storage state. */
  forward_list() : head_(0), free_head_(0), size_(0u) { reset_free_list(); }

/**
 * @brief Construct a forward_list object while initializing its fixed inline storage state.
 * @param other Other object participating in the operation.
 */
  forward_list(const forward_list& other) : head_(0), free_head_(0), size_(0u) {
    reset_free_list();
    iterator tail = before_begin();
    for (const_iterator it = other.begin(); it != other.end(); ++it) tail = insert_after(tail, *it);
  }

/** @brief Destroy forward_list and release any live inline-owned values. */
  ~forward_list() { clear(); }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  forward_list& operator=(const forward_list& other) {
    if (this != &other) {
      clear();
      iterator tail = before_begin();
      for (const_iterator it = other.begin(); it != other.end(); ++it) tail = insert_after(tail, *it);
    }
    return *this;
  }

/**
 * @brief Return the sentinel iterator that precedes the first forward-list element.
 * @return The sentinel iterator that precedes the first forward-list element.
 */
  iterator before_begin() { return iterator(0, true); }
/**
 * @brief Return the sentinel iterator that precedes the first forward-list element.
 * @return The sentinel iterator that precedes the first forward-list element.
 */
  const_iterator before_begin() const { return const_iterator(0, true); }
/**
 * @brief Return the const sentinel iterator that precedes the first element.
 * @return The const sentinel iterator that precedes the first element.
 */
  const_iterator cbefore_begin() const { return before_begin(); }
/**
 * @brief Return an iterator to the first element.
 * @return An iterator to the first element.
 */
  iterator begin() { return iterator(head_, false); }
/**
 * @brief Return an iterator to the first element.
 * @return An iterator to the first element.
 */
  const_iterator begin() const { return const_iterator(head_, false); }
/**
 * @brief Return a const iterator to the first element.
 * @return A const iterator to the first element.
 */
  const_iterator cbegin() const { return begin(); }
/**
 * @brief Return an iterator one past the final element.
 * @return An iterator one past the final element.
 */
  iterator end() { return iterator(0, false); }
/**
 * @brief Return an iterator one past the final element.
 * @return An iterator one past the final element.
 */
  const_iterator end() const { return const_iterator(0, false); }
/**
 * @brief Return a const iterator one past the final element.
 * @return A const iterator one past the final element.
 */
  const_iterator cend() const { return end(); }
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
 * @brief Access the first element.
 * @return Result described by the function brief.
 */
  T& front() { return head_ ? head_->get() : fail_reference<T>("forward_list::front empty"); } // LCOV_EXCL_BR_LINE
/**
 * @brief Access the first element.
 * @return Result described by the function brief.
 */
  const T& front() const { return head_ ? head_->get() : fail_reference<const T>("forward_list::front empty"); } // LCOV_EXCL_BR_LINE
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
 * @brief Insert a value at the front of the forward list.
 * @param x Element value supplied by the caller.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool push_front(const T& x) {
    return insert_after(before_begin(), x) != end();
  }

/**
 * @brief Try to insert a value at the front without invoking the active error policy.
 * @param x Element value supplied by the caller.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_push_front(const T& x) {
    if (full()) return false;
    return push_front(x);
  }

/** @brief Remove the first element, routing empty-list errors through the active policy. */
  void pop_front() {
    if (!try_pop_front(0)) handle_error("forward_list::pop_front empty");
  }

/**
 * @brief Remove the first element and optionally copy it to `out` under RETURN-style handling.
 * @param out Caller-provided destination for produced values.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_pop_front(T* out) {
    if (!head_) return false;
    return erase_after_with_value(before_begin(), out) != end() || !head_;
  }

/**
 * @brief Validate that a mutable iterator names before_begin, a linked node, or end.
 * @param it Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool is_valid_iterator(iterator it) const {
    return it.before_ ? it.n_ == 0 : (it.n_ == 0 || owns_node(it.n_));
  }

/**
 * @brief Validate that a const iterator names before_begin, a linked node, or end.
 * @param it Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool is_valid_iterator(const_iterator it) const {
    return it.before_ ? it.n_ == 0 : (it.n_ == 0 || owns_node(it.n_));
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
    size_ = 0u;
  }

/**
 * @brief Insert a value after a forward-list position.
 * @param pos Zero-based logical position.
 * @param x Element value supplied by the caller.
 * @return Result described by the function brief.
 */
  iterator insert_after(iterator pos, const T& x) {
    if (!is_insert_after_position(pos)) {
      handle_error("forward_list::insert_after iterator");
      return end();
    }
    node* n = allocate(x);
    if (!n) return end();
    if (pos.before_) {
      n->next = head_;
      head_ = n;
    } else if (pos.n_) {
      n->next = pos.n_->next;
      pos.n_->next = n;
    }
    ++size_;
    return iterator(n, false);
  }

/**
 * @brief Insert `count` copies of `x` after `pos`.
 * @param pos Position after which values are inserted.
 * @param count Requested element count.
 * @param x Element value supplied by the caller.
 * @return Iterator to the last inserted element, or `end()` on failure.
 */
  iterator insert_after(iterator pos, size_type count, const T& x) {
    if (!is_insert_after_position(pos) || count > N - size_) {
      handle_error("forward_list::insert_after count");
      return end();
    }
    iterator current = pos;
    for (size_type i = 0u; i != count; ++i) {
      current = insert_after(current, x);
      if (current == end()) return end(); // LCOV_EXCL_BR_LINE
    }
    return current;
  }

/**
 * @brief Insert a range of values after `pos`.
 * @param pos Position after which values are inserted.
 * @param first Start of the half-open input range.
 * @param last One-past-end of the input range.
 * @return Iterator to the last inserted element, or `end()` on failure.
 */
  template <class InputIt>
  iterator insert_after(iterator pos, InputIt first, InputIt last) {
    return insert_after_dispatch(pos, first, last, bool_constant<is_integral<InputIt>::value>());
  }

/**
 * @brief Try to insert a value after `pos` without invoking the active error policy.
 * @param pos Zero-based logical position.
 * @param x Element value supplied by the caller.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  iterator try_insert_after(iterator pos, const T& x) {
    if (full() || !is_insert_after_position(pos)) return end();
    return insert_after(pos, x);
  }

/**
 * @brief Erase the element after a forward-list position.
 * @param pos Zero-based logical position.
 * @return Result described by the function brief.
 */
  iterator erase_after(iterator pos) {
    node* target = 0;
    if (!is_insert_after_position(pos)) {
      handle_error("forward_list::erase_after iterator");
      return end();
    }
    if (pos.before_) target = head_;
    else if (pos.n_) target = pos.n_->next;
    if (!target) {
      handle_error("forward_list::erase_after iterator");
      return end();
    }
    node* after = target->next;
    if (pos.before_) head_ = after;
    else pos.n_->next = after;
    SSTL_DESTROY_AT(target->value.ptr(0));
    release_destroyed_node(target);
    --size_;
    return iterator(after, false);
  }

/**
 * @brief Erase the half-open range after `first` and before `last`.
 * @param first Position before the first erased element.
 * @param last One-past-end position that stops erasure.
 * @return Iterator naming `last` after erasure.
 */
  iterator erase_after(iterator first, iterator last) {
    if (!is_valid_after_range(first, last)) {
      handle_error("forward_list::erase_after range");
      return end();
    }
    while (next_after(first) && iterator(next_after(first), false) != last) {
      (void)erase_after(first);
    }
    return iterator(last.n_, false);
  }

/**
 * @brief Transfer elements after a forward-list position.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool splice_after(iterator pos, forward_list& other) {
    if (!is_insert_after_position(pos)) {
      handle_error("forward_list::splice_after iterator");
      return false;
    }
    if (this == &other || other.empty()) return true;
    if (N - size_ < other.size_) {
      handle_error("forward_list::splice_after full");
      return false;
    }
    iterator insert_pos = pos;
    for (iterator it = other.begin(); it != other.end(); ++it) insert_pos = insert_after(insert_pos, *it);
    other.clear();
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring all elements after a position.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_splice_after(iterator pos, forward_list& other) {
    if (!is_insert_after_position(pos)) return false;
    if (this == &other || other.empty()) return true;
    if (N - size_ < other.size_) return false;
    iterator insert_pos = pos;
    for (iterator it = other.begin(); it != other.end(); ++it) insert_pos = insert_after(insert_pos, *it);
    other.clear();
    return true;
  }

/**
 * @brief Transfer elements after a forward-list position.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M>
  bool splice_after(iterator pos, forward_list<T, M>& other) {
    if (!is_insert_after_position(pos)) {
      handle_error("forward_list::splice_after iterator");
      return false;
    }
    if (N - size_ < other.size_) {
      handle_error("forward_list::splice_after full");
      return false;
    }
    iterator insert_pos = pos;
    for (typename forward_list<T, M>::iterator it = other.begin(); it != other.end(); ++it) {
      insert_pos = insert_after(insert_pos, *it);
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
  bool try_splice_after(iterator pos, forward_list<T, M>& other) {
    if (!is_insert_after_position(pos)) return false;
    if (other.empty()) return true;
    if (N - size_ < other.size_) return false;
    iterator insert_pos = pos;
    for (typename forward_list<T, M>::iterator it = other.begin(); it != other.end(); ++it) {
      insert_pos = insert_after(insert_pos, *it);
    }
    other.clear();
    return true;
  }

/**
 * @brief Transfer elements after a forward-list position.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param before Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool splice_after(iterator pos, forward_list& other, iterator before) {
    if (!is_insert_after_position(pos) || !other.is_insert_after_position(before)) {
      handle_error("forward_list::splice_after iterator");
      return false;
    }
    node* moving = before.before_ ? other.head_ : (before.n_ ? before.n_->next : 0);
    if (!moving) {
      handle_error("forward_list::splice_after iterator");
      return false;
    }
    if (this == &other) {
      if (pos.n_ == moving || pos.n_ == before.n_) return true;
      other.unlink_after(before);
      link_after(pos, moving);
      return true;
    }
    if (full()) {
      handle_error("forward_list::splice_after full");
      return false;
    }
    T value = moving->get();
    if (insert_after(pos, value) == end()) return false; // LCOV_EXCL_BR_LINE
    other.unlink_after_and_destroy(before);
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring one element after a position.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param before Caller-supplied argument used by this operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_splice_after(iterator pos, forward_list& other, iterator before) {
    if (!is_insert_after_position(pos) || !other.is_insert_after_position(before)) return false;
    if (!other.next_after(before)) return false;
    node* moving = before.before_ ? other.head_ : (before.n_ ? before.n_->next : 0);
    if (this == &other) {
      if (pos.n_ == moving || pos.n_ == before.n_) return true;
      other.unlink_after(before);
      link_after(pos, moving);
      return true;
    }
    if (full()) return false;
    T value = moving->get();
    if (insert_after(pos, value) == end()) return false; // LCOV_EXCL_BR_LINE
    other.unlink_after_and_destroy(before);
    return true;
  }

/**
 * @brief Transfer a half-open range after a forward-list position.
 * @param pos Insertion position in this list.
 * @param other Source list.
 * @param before_first Position before the first transferred element.
 * @param before_last Position that stops transfer.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool splice_after(iterator pos, forward_list& other, iterator before_first, iterator before_last) {
    if (!is_insert_after_position(pos) || !other.is_valid_after_range(before_first, before_last)) {
      handle_error("forward_list::splice_after iterator");
      return false;
    }
    node* first = other.next_after(before_first);
    node* stop = before_last.n_;
    if (first == stop) return true;

    if (this == &other) {
      if (same_position(pos, before_first) || same_position(pos, before_last)) return true;
      for (node* scan = first; scan != stop; scan = scan->next) {
        if (!pos.before_ && scan == pos.n_) return true;
      }
      node* range_last = first;
      while (range_last->next != stop) range_last = range_last->next;
      if (before_first.before_) head_ = stop;
      else before_first.n_->next = stop;
      link_range_after(pos, first, range_last);
      return true;
    }

    const size_type count = other.after_range_count(before_first, before_last);
    if (N - size_ < count) {
      handle_error("forward_list::splice_after full");
      return false;
    }
    iterator insert_pos = pos;
    for (node* scan = first; scan != stop; scan = scan->next) {
      insert_pos = insert_after(insert_pos, scan->get());
      if (insert_pos == end()) return false; // LCOV_EXCL_BR_LINE
    }
    while (other.next_after(before_first) != stop) other.unlink_after_and_destroy(before_first);
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring a same-capacity range.
 * @param pos Insertion position in this list.
 * @param other Source list.
 * @param before_first Position before the first transferred element.
 * @param before_last Position that stops transfer.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_splice_after(iterator pos, forward_list& other, iterator before_first, iterator before_last) {
    if (!is_insert_after_position(pos) || !other.is_valid_after_range(before_first, before_last)) return false;
    if (this != &other) {
      const size_type count = other.after_range_count(before_first, before_last);
      if (N - size_ < count) return false;
    }
    return splice_after(pos, other, before_first, before_last);
  }

  /**
   * @brief Remove every element equal to `value`.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   */
  void remove(const T& value) {
    iterator prev = before_begin();
    while (next_after(prev)) {
      if (next_after(prev)->get() == value) erase_after(prev);
      else advance_to_next(prev);
    }
  }

  /**
   * @brief Remove every element for which `pred(element)` is true.
   * @param pred Predicate used to test elements.
   */
  template <class Pred>
  void remove_if(Pred pred) {
    iterator prev = before_begin();
    while (next_after(prev)) {
      if (pred(next_after(prev)->get())) erase_after(prev);
      else advance_to_next(prev);
    }
  }

  /** @brief Collapse consecutive equal elements. */
  void unique() { unique(equal_to<T>()); }

  /**
   * @brief Collapse consecutive elements equivalent under `pred(a, b)`.
   * @param pred Predicate used to test elements.
   */
  template <class BinaryPred>
  void unique(BinaryPred pred) {
    node* current = head_;
    while (current && current->next) {
      if (pred(current->get(), current->next->get())) {
        node* duplicate = current->next;
        current->next = duplicate->next;
        SSTL_DESTROY_AT(duplicate->value.ptr(0));
        release_destroyed_node(duplicate);
        --size_;
      } else {
        current = current->next;
      }
    }
  }

  /** @brief Reverse element order in place. */
  void reverse() {
    node* previous = 0;
    node* current = head_;
    while (current) {
      node* next = current->next;
      current->next = previous;
      previous = current;
      current = next;
    }
    head_ = previous;
  }

  /**
   * @brief Sort the list in-place with an allocation-free linked merge sort.
   * @param comp Strict weak ordering used for comparisons.
   */
  template <class Compare>
  void sort(Compare comp) {
    if (size_ < 2u) return;
    head_ = merge_sort_nodes(head_, comp);
  }

/** @brief Sort the container contents in place without heap allocation. */
  void sort() { sort(less<T>()); }

  /**
   * @brief Merge another sorted list into this one using fixed destination storage.
   * @param other Other object participating in the operation.
   * @param comp Strict weak ordering used for comparisons.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  template <class Compare>
  bool merge(forward_list& other, Compare comp) {
    if (this == &other) return true;
    return merge_destination_owned(other, comp);
  }

/**
 * @brief Merge sorted input from another fixed-capacity list.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  bool merge(forward_list& other) { return merge(other, less<T>()); }

/**
 * @brief Always-RETURN alias for merging sorted input from another forward list.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <class Compare>
  bool try_merge(forward_list& other, Compare comp) {
    if (this == &other) return true;
    return try_merge_destination_owned(other, comp);
  }

/**
 * @brief Always-RETURN alias for merging sorted input from another forward list.
 * @param other Other object participating in the operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  bool try_merge(forward_list& other) { return try_merge(other, less<T>()); }

/**
 * @brief Merge sorted input from another fixed-capacity list.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M, class Compare>
  bool merge(forward_list<T, M>& other, Compare comp) {
    return merge_destination_owned(other, comp);
  }

/**
 * @brief Merge sorted input from another fixed-capacity list.
 * @param other Other object participating in the operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M>
  bool merge(forward_list<T, M>& other) { return merge(other, less<T>()); }

/**
 * @brief Always-RETURN alias for merging sorted input from another capacity list.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M, class Compare>
  bool try_merge(forward_list<T, M>& other, Compare comp) { return try_merge_destination_owned(other, comp); }

/**
 * @brief Always-RETURN alias for merging sorted input from another capacity list.
 * @param other Other object participating in the operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M>
  bool try_merge(forward_list<T, M>& other) { return try_merge(other, less<T>()); }

/**
 * @brief Transfer elements after a forward-list position.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param before Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M>
  bool splice_after(iterator pos, forward_list<T, M>& other, typename forward_list<T, M>::iterator before) {
    if (!is_insert_after_position(pos) || !other.is_insert_after_position(before)) {
      handle_error("forward_list::splice_after iterator");
      return false;
    }
    typename forward_list<T, M>::node* moving = before.before_ ? other.head_ : (before.n_ ? before.n_->next : 0);
    if (!moving) {
      handle_error("forward_list::splice_after iterator");
      return false;
    }
    if (full()) {
      handle_error("forward_list::splice_after full");
      return false;
    }
    T value = moving->get();
    if (insert_after(pos, value) == end()) return false; // LCOV_EXCL_BR_LINE
    other.unlink_after_and_destroy(before);
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring one element from another capacity list.
 * @param pos Zero-based logical position.
 * @param other Other object participating in the operation.
 * @param before Caller-supplied argument used by this operation.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M>
  bool try_splice_after(iterator pos, forward_list<T, M>& other, typename forward_list<T, M>::iterator before) {
    if (!is_insert_after_position(pos) || !other.is_insert_after_position(before)) return false;
    if (!other.next_after(before)) return false;
    if (full()) return false;
    typename forward_list<T, M>::node* moving = before.before_ ? other.head_ : (before.n_ ? before.n_->next : 0);
    T value = moving->get();
    if (insert_after(pos, value) == end()) return false; // LCOV_EXCL_BR_LINE
    other.unlink_after_and_destroy(before);
    return true;
  }

/**
 * @brief Transfer a half-open range from another-capacity forward list.
 * @param pos Insertion position in this list.
 * @param other Source list.
 * @param before_first Position before the first transferred element.
 * @param before_last Position that stops transfer.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M>
  bool splice_after(iterator pos, forward_list<T, M>& other, typename forward_list<T, M>::iterator before_first,
                    typename forward_list<T, M>::iterator before_last) {
    if (!is_insert_after_position(pos) || !other.is_valid_after_range(before_first, before_last)) {
      handle_error("forward_list::splice_after iterator");
      return false;
    }
    typename forward_list<T, M>::node* first = other.next_after(before_first);
    typename forward_list<T, M>::node* stop = before_last.n_;
    if (first == stop) return true;
    const size_type count = other.after_range_count(before_first, before_last);
    if (N - size_ < count) {
      handle_error("forward_list::splice_after full");
      return false;
    }
    iterator insert_pos = pos;
    for (typename forward_list<T, M>::node* scan = first; scan != stop; scan = scan->next) {
      insert_pos = insert_after(insert_pos, scan->get());
      if (insert_pos == end()) return false; // LCOV_EXCL_BR_LINE
    }
    while (other.next_after(before_first) != stop) other.unlink_after_and_destroy(before_first);
    return true;
  }

/**
 * @brief Always-RETURN alias for transferring an another-capacity range.
 * @param pos Insertion position in this list.
 * @param other Source list.
 * @param before_first Position before the first transferred element.
 * @param before_last Position that stops transfer.
 * @return `true` on success; otherwise `false` without invoking the panic policy.
 */
  template <size_t M>
  bool try_splice_after(iterator pos, forward_list<T, M>& other, typename forward_list<T, M>::iterator before_first,
                        typename forward_list<T, M>::iterator before_last) {
    if (!is_insert_after_position(pos) || !other.is_valid_after_range(before_first, before_last)) return false;
    typename forward_list<T, M>::node* first = other.next_after(before_first);
    typename forward_list<T, M>::node* stop = before_last.n_;
    if (first == stop) return true;
    const size_type count = other.after_range_count(before_first, before_last);
    if (N - size_ < count) return false;
    iterator insert_pos = pos;
    for (typename forward_list<T, M>::node* scan = first; scan != stop; scan = scan->next) {
      insert_pos = insert_after(insert_pos, scan->get());
      if (insert_pos == end()) return false; // LCOV_EXCL_BR_LINE
    }
    while (other.next_after(before_first) != stop) other.unlink_after_and_destroy(before_first);
    return true;
  }

/**
 * @brief Exchange contents with another same-capacity forward list without external allocation.
 * @param other Other object participating in the operation.
 */
  void swap(forward_list& other) {
    if (this == &other) return;
    forward_list tmp(*this);
    *this = other;
    other = tmp;
  }

private:
  /** @brief Physical node-pool size, with one dummy slot for `N == 0`. */
  enum { slot_count = N == 0 ? 1 : N };
  /** @brief Inline node pool containing every possible forward-list element. */
  node nodes_[slot_count];
  /** @brief First live node, or null when the list is empty. */
  node* head_;
  /** @brief Head of the intrusive free-list used for O(1) node allocation. */
  node* free_head_;
  /** @brief Number of live nodes linked into the list. */
  unsigned size_;

/** @brief Reset every pool slot into the intrusive free-list. */
  void reset_free_list() {
    free_head_ = slot_count == 0 ? 0 : &nodes_[0];
    for (unsigned i = 0u; i != slot_count; ++i) {
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
 * @brief Validate an iterator as a position that may have an element after it.
 * @param pos Zero-based logical position.
 * @return True for `before_begin()` or a live node owned by this list.
 */
  bool is_insert_after_position(iterator pos) const {
    return pos.before_ ? pos.n_ == 0 : (pos.n_ != 0 && owns_node(pos.n_));
  }

/**
 * @brief Return true when two forward-list positions name the same sentinel or node.
 * @param lhs First position.
 * @param rhs Second position.
 * @return True when both positions are equal.
 */
  bool same_position(iterator lhs, iterator rhs) const {
    return lhs.n_ == rhs.n_ && lhs.before_ == rhs.before_;
  }

/**
 * @brief Validate that `before_last` is reachable after `before_first`.
 * @param before_first Position before the first ranged element.
 * @param before_last Position that stops the range.
 * @return True when `(before_first,before_last)` is a valid forward range.
 */
  bool is_valid_after_range(iterator before_first, iterator before_last) const {
    if (!is_insert_after_position(before_first) || !is_valid_iterator(before_last) || before_last.before_) return false;
    for (node* scan = next_after(before_first); scan; scan = scan->next) {
      if (scan == before_last.n_) return true;
    }
    return before_last.n_ == 0;
  }

/**
 * @brief Count nodes in the half-open range after `before_first` and before `before_last`.
 * @param before_first Position before the first ranged element.
 * @param before_last Position that stops the range.
 * @return Number of elements in the validated after-range.
 */
  size_type after_range_count(iterator before_first, iterator before_last) const {
    size_type count = 0u;
    for (node* scan = next_after(before_first); scan != before_last.n_; scan = scan->next) ++count;
    return count;
  }

/**
 * @brief Return the node immediately after a forward-list iterator position.
 * @param pos Zero-based logical position.
 * @return The node immediately after a forward-list iterator position.
 */
  node* next_after(iterator pos) const {
    return pos.before_ ? head_ : (pos.n_ ? pos.n_->next : 0); // LCOV_EXCL_BR_LINE
  }

/**
 * @brief Advance a forward-list iterator to the next node.
 * @param pos Zero-based logical position.
 */
  void advance_to_next(iterator& pos) const {
    pos = iterator(next_after(pos), false);
  }

/**
 * @brief Reserve a free inline node slot and construct a value inside it.
 * @param x Element value supplied by the caller.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
  node* allocate(const T& x) {
    if (full()) {
      handle_error("forward_list::allocate full");
      return 0;
    }
    node* n = free_head_;
    if (!n) return 0; // LCOV_EXCL_LINE
    free_head_ = n->next;
    SSTL_CONSTRUCT_AT(n->value.ptr(0), x);
    n->next = 0;
    n->used = true;
    return n;
  }

/**
 * @brief Return a destroyed pool node to the intrusive free-list.
 * @param n Requested count or size.
 */
  void release_destroyed_node(node* n) {
    n->next = free_head_;
    n->used = false;
    free_head_ = n;
  }

/**
 * @brief Link a node after the requested position.
 * @param pos Zero-based logical position.
 * @param n Requested count or size.
 */
  void link_after(iterator pos, node* n) {
    if (pos.before_) {
      n->next = head_;
      head_ = n;
    } else {
      n->next = pos.n_ ? pos.n_->next : 0; // LCOV_EXCL_BR_LINE
      if (pos.n_) pos.n_->next = n; // LCOV_EXCL_BR_LINE
    }
  }

/**
 * @brief Link a detached non-empty node range after the requested position.
 * @param pos Position to link after.
 * @param first First node in the detached range.
 * @param last Last node in the detached range.
 */
  void link_range_after(iterator pos, node* first, node* last) {
    if (pos.before_) {
      last->next = head_;
      head_ = first;
    } else {
      last->next = pos.n_ ? pos.n_->next : 0; // LCOV_EXCL_BR_LINE
      if (pos.n_) pos.n_->next = first; // LCOV_EXCL_BR_LINE
    }
  }

/**
 * @brief Detach the node after a forward-list position.
 * @param pos Zero-based logical position.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
  node* unlink_after(iterator pos) {
    node* target = pos.before_ ? head_ : (pos.n_ ? pos.n_->next : 0); // LCOV_EXCL_BR_LINE
    if (!target) return 0; // LCOV_EXCL_BR_LINE
    if (pos.before_) head_ = target->next;
    else pos.n_->next = target->next;
    target->next = 0;
    return target;
  }

/**
 * @brief Detach and destroy the node after a forward-list position.
 * @param pos Zero-based logical position.
 */
  void unlink_after_and_destroy(iterator pos) {
    node* target = unlink_after(pos);
    if (!target) return; // LCOV_EXCL_BR_LINE
    SSTL_DESTROY_AT(target->value.ptr(0));
    release_destroyed_node(target);
    --size_;
  }

/**
 * @brief Erase after a position while copying the removed value when requested.
 * @param pos Zero-based logical position.
 * @param out Caller-provided destination for produced values.
 * @return Result described by the function brief.
 */
  iterator erase_after_with_value(iterator pos, T* out) {
    node* target = 0;
    if (pos.before_) target = head_;
    else if (pos.n_) target = pos.n_->next; // LCOV_EXCL_LINE
    if (!target) return end();
    node* after = target->next;
    if (out) *out = target->get();
    if (pos.before_) head_ = after;
    else pos.n_->next = after; // LCOV_EXCL_LINE
    SSTL_DESTROY_AT(target->value.ptr(0));
    release_destroyed_node(target);
    --size_;
    return iterator(after, false);
  }

/**
 * @brief Insert a value at its sorted position.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <class Compare>
  bool insert_sorted_value(const T& value, Compare comp) {
    iterator prev = before_begin();
    while (next_after(prev) && !comp(value, next_after(prev)->get())) advance_to_next(prev);
    return insert_after(prev, value) != end();
  }

/**
 * @brief Append an existing or newly allocated node to a rebuilt forward chain.
 * @param tail Caller-supplied argument used by this operation.
 * @param n Requested count or size.
 */
  static void append_relinked_node(node**& tail, node* n) {
    *tail = n;
    tail = &n->next;
    n->next = 0;
  }

/**
 * @brief Merge sorted source values into this list in one pass using destination-owned nodes.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
  template <size_t M, class Compare>
  bool merge_destination_owned(forward_list<T, M>& other, Compare comp) {
    if (N - size_ < other.size_) {
      handle_error("forward_list::merge full");
      return false;
    }
    return merge_destination_owned_unchecked(other, comp);
  }

/**
 * @brief Try to merge sorted source values without invoking the active error policy.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when merged; otherwise `false` when destination capacity is insufficient.
 */
  template <size_t M, class Compare>
  bool try_merge_destination_owned(forward_list<T, M>& other, Compare comp) {
    if (N - size_ < other.size_) return false;
    return merge_destination_owned_unchecked(other, comp);
  }

/**
 * @brief Merge sorted source values after callers have checked destination capacity.
 * @param other Other object participating in the operation.
 * @param comp Strict weak ordering used for comparisons.
 * @return `true` when the merge completed.
 */
  template <size_t M, class Compare>
  bool merge_destination_owned_unchecked(forward_list<T, M>& other, Compare comp) {
    node* a = head_;
    typename forward_list<T, M>::node* b = other.head_;
    node* merged_head = 0;
    node** tail = &merged_head;
    while (a || b) {
      if (!b || (a && !comp(b->get(), a->get()))) {
        node* next = a->next;
        append_relinked_node(tail, a);
        a = next;
      } else {
        typename forward_list<T, M>::node* next = b->next;
        node* copied = allocate(b->get());
        if (!copied) return false; // LCOV_EXCL_BR_LINE
        ++size_;
        append_relinked_node(tail, copied);
        b = next;
      }
    }
    head_ = merged_head;
    other.clear();
    return true;
  }

/**
 * @brief Split a linked run after its middle node for merge sort.
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
    node* second = slow ? slow->next : 0; // LCOV_EXCL_BR_LINE
    if (slow) slow->next = 0; // LCOV_EXCL_BR_LINE
    return second;
  }

/**
 * @brief Merge two sorted node chains without allocating.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @param comp Strict weak ordering used for comparisons.
 * @return Pointer described by the function brief, or null for probe-style failure cases.
 */
  template <class Compare>
  static node* merge_sorted_nodes(node* a, node* b, Compare comp) {
    node* head = 0;
    node** tail = &head;
    while (a && b) {
      if (comp(b->get(), a->get())) {
        *tail = b;
        b = b->next;
      } else {
        *tail = a;
        a = a->next;
      }
      tail = &((*tail)->next);
    }
    *tail = a ? a : b;
    return head;
  }

/**
 * @brief Sort a node chain with allocation-free linked merge sort.
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

/**
 * @brief Count a multipass input range without mutating the list.
 * @param first Start of the half-open range.
 * @param last One-past-end of the half-open range.
 * @return Number of elements in the range.
 */
  template <class InputIt>
  size_type range_count(InputIt first, InputIt last) const {
    size_type count = 0u;
    for (; first != last; ++first) ++count;
    return count;
  }

/**
 * @brief Dispatch integral insert-after calls to counted insertion.
 * @param pos Position after which values are inserted.
 * @param count Requested element count.
 * @param value Element value supplied by the caller.
 * @return Iterator to the last inserted element, or `end()` on failure.
 */
  template <class Count, class Value>
  iterator insert_after_dispatch(iterator pos, Count count, Value value, bool_constant<true>) {
    return insert_after(pos, static_cast<size_type>(count), static_cast<T>(value));
  }

/**
 * @brief Dispatch iterator insert-after calls to range insertion.
 * @param pos Position after which values are inserted.
 * @param first Start of the half-open input range.
 * @param last One-past-end of the input range.
 * @return Iterator to the last inserted element, or `end()` on failure.
 */
  template <class InputIt>
  iterator insert_after_dispatch(iterator pos, InputIt first, InputIt last, bool_constant<false>) {
    const size_type count = range_count(first, last);
    if (!is_insert_after_position(pos) || count > N - size_) {
      handle_error("forward_list::insert_after range");
      return end();
    }
    iterator current = pos;
    for (; first != last; ++first) {
      current = insert_after(current, *first);
      if (current == end()) return end(); // LCOV_EXCL_BR_LINE
    }
    return current;
  }
};

/**
 * @brief Exchange two same-capacity forward lists through the member swap operation.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <class T, size_t N>
inline void swap(forward_list<T, N>& lhs, forward_list<T, N>& rhs) {
  lhs.swap(rhs);
}

/** @brief Compare two forward lists for element-wise equality across capacities. */
template <class T, size_t N, size_t M>
inline bool operator==(const forward_list<T, N>& lhs, const forward_list<T, M>& rhs) {
  typename forward_list<T, N>::const_iterator a = lhs.begin();
  typename forward_list<T, M>::const_iterator b = rhs.begin();
  for (; a != lhs.end() && b != rhs.end(); ++a, ++b) {
    if (!(*a == *b)) return false;
  }
  return a == lhs.end() && b == rhs.end();
}

/** @brief Compare two forward lists for inequality. */
template <class T, size_t N, size_t M>
inline bool operator!=(const forward_list<T, N>& lhs, const forward_list<T, M>& rhs) { return !(lhs == rhs); }

/** @brief Lexicographically compare two forward-list sequences. */
template <class T, size_t N, size_t M>
inline bool operator<(const forward_list<T, N>& lhs, const forward_list<T, M>& rhs) {
  typename forward_list<T, N>::const_iterator a = lhs.begin();
  typename forward_list<T, M>::const_iterator b = rhs.begin();
  for (; a != lhs.end() && b != rhs.end(); ++a, ++b) {
    if (*a < *b) return true;
    if (*b < *a) return false;
  }
  return a == lhs.end() && b != rhs.end();
}

/** @brief Return true when `lhs` is not lexicographically greater than `rhs`. */
template <class T, size_t N, size_t M>
inline bool operator<=(const forward_list<T, N>& lhs, const forward_list<T, M>& rhs) { return !(rhs < lhs); }

/** @brief Return true when `lhs` is lexicographically greater than `rhs`. */
template <class T, size_t N, size_t M>
inline bool operator>(const forward_list<T, N>& lhs, const forward_list<T, M>& rhs) { return rhs < lhs; }

/** @brief Return true when `lhs` is not lexicographically less than `rhs`. */
template <class T, size_t N, size_t M>
inline bool operator>=(const forward_list<T, N>& lhs, const forward_list<T, M>& rhs) { return !(lhs < rhs); }

} // namespace sstl

#endif

