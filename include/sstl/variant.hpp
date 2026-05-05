/**
 * @file variant.hpp
 * @brief Fixed-alternative discriminated unions for C++03 SSTL code.
 *
 * The implementation stores exactly one active alternative in inline raw
 * storage. No heap allocation, RTTI, exceptions, or hosted standard-library
 * helpers are required. The arity-specific classes are intentionally explicit:
 * C++03 lacks variadic templates, so spelling out variant2/3/4 keeps the public
 * surface understandable to future maintainers.
 */
#ifndef SSTL_VARIANT_HPP
/** @def SSTL_VARIANT_HPP
 * @brief Include guard for variant.hpp.
 */
#define SSTL_VARIANT_HPP

#include "config.hpp"
#include "type_traits.hpp"

namespace sstl {

/** @brief Placeholder type used for absent alternatives in fixed-arity helper templates. */
struct variant_none {};

/** @brief Compile-time mapping from a requested type to its variant alternative index. */
template <class T, class A, class B, class C = variant_none, class D = variant_none>
struct variant_index_of {
  enum {
    /** @brief Zero-based alternative index for `T`, or -1 when `T` is not present. */
    value =
      is_same<T, A>::value ? 0 :
      is_same<T, B>::value ? 1 :
      is_same<T, C>::value ? 2 :
      is_same<T, D>::value ? 3 : -1
  };
};

/**
 * @brief Two-alternative fixed-storage variant.
 * @tparam A First alternative and default-constructed active type.
 * @tparam B Second alternative.
 */
template <class A, class B>
class variant2 {
public:
/** @brief Construct a variant2 object while initializing its fixed inline storage state. */
  variant2() : idx_(0) { SSTL_CONSTRUCT_AT(storage_.a.ptr(0), A()); }
/**
 * @brief Construct a variant2 object while initializing its fixed inline storage state.
 * @param a First operand or first range start.
 */
  variant2(const A& a) : idx_(0) { SSTL_CONSTRUCT_AT(storage_.a.ptr(0), a); }
/**
 * @brief Construct a variant2 object while initializing its fixed inline storage state.
 * @param b Second operand or second range start.
 */
  variant2(const B& b) : idx_(1) { SSTL_CONSTRUCT_AT(storage_.b.ptr(0), b); }
/**
 * @brief Construct a variant2 object while initializing its fixed inline storage state.
 * @param other Other object participating in the operation.
 */
  variant2(const variant2& other) : idx_(0) { copy_from(other); }
/** @brief Destroy variant2 and release any live inline-owned values. */
  ~variant2() { destroy(); }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  variant2& operator=(const variant2& other) {
    if (this != &other) {
      destroy();
      copy_from(other);
    }
    return *this;
  }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param a First operand or first range start.
 * @return Result described by the function brief.
 */
  variant2& operator=(const A& a) { destroy(); idx_ = 0; SSTL_CONSTRUCT_AT(storage_.a.ptr(0), a); return *this; }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param b Second operand or second range start.
 * @return Result described by the function brief.
 */
  variant2& operator=(const B& b) { destroy(); idx_ = 1; SSTL_CONSTRUCT_AT(storage_.b.ptr(0), b); return *this; }

/**
 * @brief Return the active alternative index.
 * @return The active alternative index.
 */
  unsigned index() const { return idx_; }

/**
 * @brief Access alternative 0 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  A& get0() { return *storage_.a.ptr(0); }
/**
 * @brief Access alternative 0 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const A& get0() const { return *storage_.a.ptr(0); }
/**
 * @brief Access alternative 1 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  B& get1() { return *storage_.b.ptr(0); }
/**
 * @brief Access alternative 1 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const B& get1() const { return *storage_.b.ptr(0); }

/**
 * @brief Dispatch the active variant alternative to a visitor.
 * @param visitor Caller-supplied argument used by this operation.
 */
  template <class Visitor>
  void visit(Visitor visitor) {
    if (idx_ == 0) visitor(get0());
    else visitor(get1());
  }

/**
 * @brief Dispatch the active variant alternative to a visitor.
 * @param visitor Caller-supplied argument used by this operation.
 */
  template <class Visitor>
  void visit(Visitor visitor) const {
    if (idx_ == 0) visitor(get0());
    else visitor(get1());
  }

private:
  /** @brief Raw-storage union that can hold either variant2 alternative. */
  union storage_type {
    /** @brief Raw storage for alternative 0. */
    raw_storage<A, 1> a;
    /** @brief Raw storage for alternative 1. */
    raw_storage<B, 1> b;
  };
  /** @brief Union instance holding raw storage for all alternatives. */
  storage_type storage_;
  /** @brief Index of the active alternative currently constructed in `storage_`. */
  unsigned idx_;

/** @brief Destroy the currently active variant alternative. */
  void destroy() {
    if (idx_ == 0) SSTL_DESTROY_AT(storage_.a.ptr(0));
    else SSTL_DESTROY_AT(storage_.b.ptr(0));
  }

/**
 * @brief Copy the active variant alternative from another variant.
 * @param other Other object participating in the operation.
 */
  void copy_from(const variant2& other) {
    idx_ = other.idx_;
    if (idx_ == 0) SSTL_CONSTRUCT_AT(storage_.a.ptr(0), other.get0());
    else SSTL_CONSTRUCT_AT(storage_.b.ptr(0), other.get1());
  }
};

/**
 * @brief Three-alternative fixed-storage variant.
 */
template <class A, class B, class C>
class variant3 {
public:
/** @brief Construct a variant3 object while initializing its fixed inline storage state. */
  variant3() : idx_(0) { SSTL_CONSTRUCT_AT(storage_.a.ptr(0), A()); }
/**
 * @brief Construct a variant3 object while initializing its fixed inline storage state.
 * @param a First operand or first range start.
 */
  variant3(const A& a) : idx_(0) { SSTL_CONSTRUCT_AT(storage_.a.ptr(0), a); }
/**
 * @brief Construct a variant3 object while initializing its fixed inline storage state.
 * @param b Second operand or second range start.
 */
  variant3(const B& b) : idx_(1) { SSTL_CONSTRUCT_AT(storage_.b.ptr(0), b); }
/**
 * @brief Construct a variant3 object while initializing its fixed inline storage state.
 * @param c Character value.
 */
  variant3(const C& c) : idx_(2) { SSTL_CONSTRUCT_AT(storage_.c.ptr(0), c); }
/**
 * @brief Construct a variant3 object while initializing its fixed inline storage state.
 * @param other Other object participating in the operation.
 */
  variant3(const variant3& other) : idx_(0) { copy_from(other); }
/** @brief Destroy variant3 and release any live inline-owned values. */
  ~variant3() { destroy(); }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  variant3& operator=(const variant3& other) {
    if (this != &other) {
      destroy();
      copy_from(other);
    }
    return *this;
  }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param a First operand or first range start.
 * @return Result described by the function brief.
 */
  variant3& operator=(const A& a) { destroy(); idx_ = 0; SSTL_CONSTRUCT_AT(storage_.a.ptr(0), a); return *this; }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param b Second operand or second range start.
 * @return Result described by the function brief.
 */
  variant3& operator=(const B& b) { destroy(); idx_ = 1; SSTL_CONSTRUCT_AT(storage_.b.ptr(0), b); return *this; }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param c Character value.
 * @return Result described by the function brief.
 */
  variant3& operator=(const C& c) { destroy(); idx_ = 2; SSTL_CONSTRUCT_AT(storage_.c.ptr(0), c); return *this; }

/**
 * @brief Return the active alternative index.
 * @return The active alternative index.
 */
  unsigned index() const { return idx_; }

/**
 * @brief Access alternative 0 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  A& get0() { return *storage_.a.ptr(0); }
/**
 * @brief Access alternative 0 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const A& get0() const { return *storage_.a.ptr(0); }
/**
 * @brief Access alternative 1 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  B& get1() { return *storage_.b.ptr(0); }
/**
 * @brief Access alternative 1 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const B& get1() const { return *storage_.b.ptr(0); }
/**
 * @brief Access alternative 2 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  C& get2() { return *storage_.c.ptr(0); }
/**
 * @brief Access alternative 2 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const C& get2() const { return *storage_.c.ptr(0); }

/**
 * @brief Dispatch the active variant alternative to a visitor.
 * @param visitor Caller-supplied argument used by this operation.
 */
  template <class Visitor>
  void visit(Visitor visitor) {
    if (idx_ == 0) visitor(get0());
    else if (idx_ == 1) visitor(get1());
    else visitor(get2());
  }

/**
 * @brief Dispatch the active variant alternative to a visitor.
 * @param visitor Caller-supplied argument used by this operation.
 */
  template <class Visitor>
  void visit(Visitor visitor) const {
    if (idx_ == 0) visitor(get0());
    else if (idx_ == 1) visitor(get1());
    else visitor(get2());
  }

private:
  /** @brief Raw-storage union that can hold any variant3 alternative. */
  union storage_type {
    /** @brief Raw storage for alternative 0. */
    raw_storage<A, 1> a;
    /** @brief Raw storage for alternative 1. */
    raw_storage<B, 1> b;
    /** @brief Raw storage for alternative 2. */
    raw_storage<C, 1> c;
  };
  /** @brief Union instance holding raw storage for all alternatives. */
  storage_type storage_;
  /** @brief Index of the active alternative currently constructed in `storage_`. */
  unsigned idx_;

/** @brief Destroy the currently active variant alternative. */
  void destroy() {
    if (idx_ == 0) SSTL_DESTROY_AT(storage_.a.ptr(0));
    else if (idx_ == 1) SSTL_DESTROY_AT(storage_.b.ptr(0));
    else SSTL_DESTROY_AT(storage_.c.ptr(0));
  }

/**
 * @brief Copy the active variant alternative from another variant.
 * @param other Other object participating in the operation.
 */
  void copy_from(const variant3& other) {
    idx_ = other.idx_;
    if (idx_ == 0) SSTL_CONSTRUCT_AT(storage_.a.ptr(0), other.get0());
    else if (idx_ == 1) SSTL_CONSTRUCT_AT(storage_.b.ptr(0), other.get1());
    else SSTL_CONSTRUCT_AT(storage_.c.ptr(0), other.get2());
  }
};

/**
 * @brief Four-alternative fixed-storage variant.
 */
template <class A, class B, class C, class D>
class variant4 {
public:
/** @brief Construct a variant4 object while initializing its fixed inline storage state. */
  variant4() : idx_(0) { SSTL_CONSTRUCT_AT(storage_.a.ptr(0), A()); }
/**
 * @brief Construct a variant4 object while initializing its fixed inline storage state.
 * @param a First operand or first range start.
 */
  variant4(const A& a) : idx_(0) { SSTL_CONSTRUCT_AT(storage_.a.ptr(0), a); }
/**
 * @brief Construct a variant4 object while initializing its fixed inline storage state.
 * @param b Second operand or second range start.
 */
  variant4(const B& b) : idx_(1) { SSTL_CONSTRUCT_AT(storage_.b.ptr(0), b); }
/**
 * @brief Construct a variant4 object while initializing its fixed inline storage state.
 * @param c Character value.
 */
  variant4(const C& c) : idx_(2) { SSTL_CONSTRUCT_AT(storage_.c.ptr(0), c); }
/**
 * @brief Construct a variant4 object while initializing its fixed inline storage state.
 * @param d Deque instance.
 */
  variant4(const D& d) : idx_(3) { SSTL_CONSTRUCT_AT(storage_.d.ptr(0), d); }
/**
 * @brief Construct a variant4 object while initializing its fixed inline storage state.
 * @param other Other object participating in the operation.
 */
  variant4(const variant4& other) : idx_(0) { copy_from(other); }
/** @brief Destroy variant4 and release any live inline-owned values. */
  ~variant4() { destroy(); }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  variant4& operator=(const variant4& other) {
    if (this != &other) {
      destroy();
      copy_from(other);
    }
    return *this;
  }

/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param a First operand or first range start.
 * @return Result described by the function brief.
 */
  variant4& operator=(const A& a) { destroy(); idx_ = 0; SSTL_CONSTRUCT_AT(storage_.a.ptr(0), a); return *this; }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param b Second operand or second range start.
 * @return Result described by the function brief.
 */
  variant4& operator=(const B& b) { destroy(); idx_ = 1; SSTL_CONSTRUCT_AT(storage_.b.ptr(0), b); return *this; }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param c Character value.
 * @return Result described by the function brief.
 */
  variant4& operator=(const C& c) { destroy(); idx_ = 2; SSTL_CONSTRUCT_AT(storage_.c.ptr(0), c); return *this; }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param d Deque instance.
 * @return Result described by the function brief.
 */
  variant4& operator=(const D& d) { destroy(); idx_ = 3; SSTL_CONSTRUCT_AT(storage_.d.ptr(0), d); return *this; }

/**
 * @brief Return the active alternative index.
 * @return The active alternative index.
 */
  unsigned index() const { return idx_; }

/**
 * @brief Access alternative 0 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  A& get0() { return *storage_.a.ptr(0); }
/**
 * @brief Access alternative 0 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const A& get0() const { return *storage_.a.ptr(0); }
/**
 * @brief Access alternative 1 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  B& get1() { return *storage_.b.ptr(0); }
/**
 * @brief Access alternative 1 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const B& get1() const { return *storage_.b.ptr(0); }
/**
 * @brief Access alternative 2 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  C& get2() { return *storage_.c.ptr(0); }
/**
 * @brief Access alternative 2 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const C& get2() const { return *storage_.c.ptr(0); }
/**
 * @brief Access alternative 3 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  D& get3() { return *storage_.d.ptr(0); }
/**
 * @brief Access alternative 3 in this fixed-arity variant.
 * @return Result described by the function brief.
 */
  const D& get3() const { return *storage_.d.ptr(0); }

/**
 * @brief Dispatch the active variant alternative to a visitor.
 * @param visitor Caller-supplied argument used by this operation.
 */
  template <class Visitor>
  void visit(Visitor visitor) {
    if (idx_ == 0) visitor(get0());
    else if (idx_ == 1) visitor(get1());
    else if (idx_ == 2) visitor(get2());
    else visitor(get3());
  }

/**
 * @brief Dispatch the active variant alternative to a visitor.
 * @param visitor Caller-supplied argument used by this operation.
 */
  template <class Visitor>
  void visit(Visitor visitor) const {
    if (idx_ == 0) visitor(get0());
    else if (idx_ == 1) visitor(get1());
    else if (idx_ == 2) visitor(get2());
    else visitor(get3());
  }

private:
  /** @brief Raw-storage union that can hold any variant4 alternative. */
  union storage_type {
    /** @brief Raw storage for alternative 0. */
    raw_storage<A, 1> a;
    /** @brief Raw storage for alternative 1. */
    raw_storage<B, 1> b;
    /** @brief Raw storage for alternative 2. */
    raw_storage<C, 1> c;
    /** @brief Raw storage for alternative 3. */
    raw_storage<D, 1> d;
  };
  /** @brief Union instance holding raw storage for all alternatives. */
  storage_type storage_;
  /** @brief Index of the active alternative currently constructed in `storage_`. */
  unsigned idx_;

/** @brief Destroy the currently active variant alternative. */
  void destroy() {
    if (idx_ == 0) SSTL_DESTROY_AT(storage_.a.ptr(0));
    else if (idx_ == 1) SSTL_DESTROY_AT(storage_.b.ptr(0));
    else if (idx_ == 2) SSTL_DESTROY_AT(storage_.c.ptr(0));
    else SSTL_DESTROY_AT(storage_.d.ptr(0));
  }

/**
 * @brief Copy the active variant alternative from another variant.
 * @param other Other object participating in the operation.
 */
  void copy_from(const variant4& other) {
    idx_ = other.idx_;
    if (idx_ == 0) SSTL_CONSTRUCT_AT(storage_.a.ptr(0), other.get0());
    else if (idx_ == 1) SSTL_CONSTRUCT_AT(storage_.b.ptr(0), other.get1());
    else if (idx_ == 2) SSTL_CONSTRUCT_AT(storage_.c.ptr(0), other.get2());
    else SSTL_CONSTRUCT_AT(storage_.d.ptr(0), other.get3());
  }
};

/** @brief Primary indexed getter declaration for `variant2`; specialized for valid indices. */
template <int I, class A, class B>
struct variant2_getter;

/** @brief Getter specialization for alternative 0 of `variant2`. */
template <class A, class B>
struct variant2_getter<0, A, B> {
  /** @brief Result type for alternative 0. */
  typedef A result;
  /**
   * @brief Return mutable alternative 0 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 0 from `v`.
   */
  static A& get(variant2<A, B>& v) { return v.get0(); }
  /**
   * @brief Return const alternative 0 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 0 from `v`.
   */
  static const A& get(const variant2<A, B>& v) { return v.get0(); }
};

/** @brief Getter specialization for alternative 1 of `variant2`. */
template <class A, class B>
struct variant2_getter<1, A, B> {
  /** @brief Result type for alternative 1. */
  typedef B result;
  /**
   * @brief Return mutable alternative 1 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 1 from `v`.
   */
  static B& get(variant2<A, B>& v) { return v.get1(); }
  /**
   * @brief Return const alternative 1 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 1 from `v`.
   */
  static const B& get(const variant2<A, B>& v) { return v.get1(); }
};

/** @brief Primary indexed getter declaration for `variant3`; specialized for valid indices. */
template <int I, class A, class B, class C>
struct variant3_getter;

/** @brief Getter specialization for alternative 0 of `variant3`. */
template <class A, class B, class C>
struct variant3_getter<0, A, B, C> {
  /** @brief Result type for alternative 0. */
  typedef A result;
  /**
   * @brief Return mutable alternative 0 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 0 from `v`.
   */
  static A& get(variant3<A, B, C>& v) { return v.get0(); }
  /**
   * @brief Return const alternative 0 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 0 from `v`.
   */
  static const A& get(const variant3<A, B, C>& v) { return v.get0(); }
};

/** @brief Getter specialization for alternative 1 of `variant3`. */
template <class A, class B, class C>
struct variant3_getter<1, A, B, C> {
  /** @brief Result type for alternative 1. */
  typedef B result;
  /**
   * @brief Return mutable alternative 1 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 1 from `v`.
   */
  static B& get(variant3<A, B, C>& v) { return v.get1(); }
  /**
   * @brief Return const alternative 1 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 1 from `v`.
   */
  static const B& get(const variant3<A, B, C>& v) { return v.get1(); }
};

/** @brief Getter specialization for alternative 2 of `variant3`. */
template <class A, class B, class C>
struct variant3_getter<2, A, B, C> {
  /** @brief Result type for alternative 2. */
  typedef C result;
  /**
   * @brief Return mutable alternative 2 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 2 from `v`.
   */
  static C& get(variant3<A, B, C>& v) { return v.get2(); }
  /**
   * @brief Return const alternative 2 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 2 from `v`.
   */
  static const C& get(const variant3<A, B, C>& v) { return v.get2(); }
};

/** @brief Primary indexed getter declaration for `variant4`; specialized for valid indices. */
template <int I, class A, class B, class C, class D>
struct variant4_getter;

/** @brief Getter specialization for alternative 0 of `variant4`. */
template <class A, class B, class C, class D>
struct variant4_getter<0, A, B, C, D> {
  /** @brief Result type for alternative 0. */
  typedef A result;
  /**
   * @brief Return mutable alternative 0 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 0 from `v`.
   */
  static A& get(variant4<A, B, C, D>& v) { return v.get0(); }
  /**
   * @brief Return const alternative 0 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 0 from `v`.
   */
  static const A& get(const variant4<A, B, C, D>& v) { return v.get0(); }
};

/** @brief Getter specialization for alternative 1 of `variant4`. */
template <class A, class B, class C, class D>
struct variant4_getter<1, A, B, C, D> {
  /** @brief Result type for alternative 1. */
  typedef B result;
  /**
   * @brief Return mutable alternative 1 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 1 from `v`.
   */
  static B& get(variant4<A, B, C, D>& v) { return v.get1(); }
  /**
   * @brief Return const alternative 1 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 1 from `v`.
   */
  static const B& get(const variant4<A, B, C, D>& v) { return v.get1(); }
};

/** @brief Getter specialization for alternative 2 of `variant4`. */
template <class A, class B, class C, class D>
struct variant4_getter<2, A, B, C, D> {
  /** @brief Result type for alternative 2. */
  typedef C result;
  /**
   * @brief Return mutable alternative 2 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 2 from `v`.
   */
  static C& get(variant4<A, B, C, D>& v) { return v.get2(); }
  /**
   * @brief Return const alternative 2 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 2 from `v`.
   */
  static const C& get(const variant4<A, B, C, D>& v) { return v.get2(); }
};

/** @brief Getter specialization for alternative 3 of `variant4`. */
template <class A, class B, class C, class D>
struct variant4_getter<3, A, B, C, D> {
  /** @brief Result type for alternative 3. */
  typedef D result;
  /**
   * @brief Return mutable alternative 3 from `v`.
   * @param v Generated object or variant instance.
   * @return Mutable alternative 3 from `v`.
   */
  static D& get(variant4<A, B, C, D>& v) { return v.get3(); }
  /**
   * @brief Return const alternative 3 from `v`.
   * @param v Generated object or variant instance.
   * @return Const alternative 3 from `v`.
   */
  static const D& get(const variant4<A, B, C, D>& v) { return v.get3(); }
};

/**
 * @brief Return mutable alternative `I` from a `variant2`.
 * @param v Generated object or variant instance.
 * @return Mutable alternative `i` from a `variant2`.
 */
template <int I, class A, class B>
typename variant2_getter<I, A, B>::result& get(variant2<A, B>& v) { return variant2_getter<I, A, B>::get(v); }

/**
 * @brief Return const alternative `I` from a `variant2`.
 * @param v Generated object or variant instance.
 * @return Const alternative `i` from a `variant2`.
 */
template <int I, class A, class B>
const typename variant2_getter<I, A, B>::result& get(const variant2<A, B>& v) { return variant2_getter<I, A, B>::get(v); }

/**
 * @brief Return mutable alternative `I` from a `variant3`.
 * @param v Generated object or variant instance.
 * @return Mutable alternative `i` from a `variant3`.
 */
template <int I, class A, class B, class C>
typename variant3_getter<I, A, B, C>::result& get(variant3<A, B, C>& v) { return variant3_getter<I, A, B, C>::get(v); }

/**
 * @brief Return const alternative `I` from a `variant3`.
 * @param v Generated object or variant instance.
 * @return Const alternative `i` from a `variant3`.
 */
template <int I, class A, class B, class C>
const typename variant3_getter<I, A, B, C>::result& get(const variant3<A, B, C>& v) { return variant3_getter<I, A, B, C>::get(v); }

/**
 * @brief Return mutable alternative `I` from a `variant4`.
 * @param v Generated object or variant instance.
 * @return Mutable alternative `i` from a `variant4`.
 */
template <int I, class A, class B, class C, class D>
typename variant4_getter<I, A, B, C, D>::result& get(variant4<A, B, C, D>& v) { return variant4_getter<I, A, B, C, D>::get(v); }

/**
 * @brief Return const alternative `I` from a `variant4`.
 * @param v Generated object or variant instance.
 * @return Const alternative `i` from a `variant4`.
 */
template <int I, class A, class B, class C, class D>
const typename variant4_getter<I, A, B, C, D>::result& get(const variant4<A, B, C, D>& v) { return variant4_getter<I, A, B, C, D>::get(v); }

/**
 * @brief Return mutable pointer to alternative `I` in a `variant2`, or null when inactive.
 * @param v Generated object or variant instance.
 * @return Mutable pointer to alternative `i` in a `variant2`, or null when inactive.
 */
template <int I, class A, class B>
typename variant2_getter<I, A, B>::result* get_if(variant2<A, B>* v) { return v && v->index() == static_cast<unsigned>(I) ? &get<I>(*v) : 0; }

/**
 * @brief Return const pointer to alternative `I` in a `variant2`, or null when inactive.
 * @param v Generated object or variant instance.
 * @return Const pointer to alternative `i` in a `variant2`, or null when inactive.
 */
template <int I, class A, class B>
const typename variant2_getter<I, A, B>::result* get_if(const variant2<A, B>* v) { return v && v->index() == static_cast<unsigned>(I) ? &get<I>(*v) : 0; }

/**
 * @brief Return mutable pointer to alternative `I` in a `variant3`, or null when inactive.
 * @param v Generated object or variant instance.
 * @return Mutable pointer to alternative `i` in a `variant3`, or null when inactive.
 */
template <int I, class A, class B, class C>
typename variant3_getter<I, A, B, C>::result* get_if(variant3<A, B, C>* v) { return v && v->index() == static_cast<unsigned>(I) ? &get<I>(*v) : 0; }

/**
 * @brief Return const pointer to alternative `I` in a `variant3`, or null when inactive.
 * @param v Generated object or variant instance.
 * @return Const pointer to alternative `i` in a `variant3`, or null when inactive.
 */
template <int I, class A, class B, class C>
const typename variant3_getter<I, A, B, C>::result* get_if(const variant3<A, B, C>* v) { return v && v->index() == static_cast<unsigned>(I) ? &get<I>(*v) : 0; }

/**
 * @brief Return mutable pointer to alternative `I` in a `variant4`, or null when inactive.
 * @param v Generated object or variant instance.
 * @return Mutable pointer to alternative `i` in a `variant4`, or null when inactive.
 */
template <int I, class A, class B, class C, class D>
typename variant4_getter<I, A, B, C, D>::result* get_if(variant4<A, B, C, D>* v) { return v && v->index() == static_cast<unsigned>(I) ? &get<I>(*v) : 0; }

/**
 * @brief Return const pointer to alternative `I` in a `variant4`, or null when inactive.
 * @param v Generated object or variant instance.
 * @return Const pointer to alternative `i` in a `variant4`, or null when inactive.
 */
template <int I, class A, class B, class C, class D>
const typename variant4_getter<I, A, B, C, D>::result* get_if(const variant4<A, B, C, D>* v) { return v && v->index() == static_cast<unsigned>(I) ? &get<I>(*v) : 0; }

/**
 * @brief Return whether a `variant2` currently holds alternative index `I`.
 * @param v Generated object or variant instance.
 * @return Whether a `variant2` currently holds alternative index `i`.
 */
template <int I, class A, class B>
bool holds_alternative(const variant2<A, B>& v) {
  return v.index() == static_cast<unsigned>(I);
}

/**
 * @brief Return whether a `variant3` currently holds alternative index `I`.
 * @param v Generated object or variant instance.
 * @return Whether a `variant3` currently holds alternative index `i`.
 */
template <int I, class A, class B, class C>
bool holds_alternative(const variant3<A, B, C>& v) {
  return v.index() == static_cast<unsigned>(I);
}

/**
 * @brief Return whether a `variant4` currently holds alternative index `I`.
 * @param v Generated object or variant instance.
 * @return Whether a `variant4` currently holds alternative index `i`.
 */
template <int I, class A, class B, class C, class D>
bool holds_alternative(const variant4<A, B, C, D>& v) {
  return v.index() == static_cast<unsigned>(I);
}

/**
 * @brief Return whether the variant currently holds the requested type.
 * @return Whether the variant currently holds the requested type.
 */
template <class T, class A, class B>
bool holds_alternative(const variant2<A, B>& v) {
  return variant_index_of<T, A, B>::value == static_cast<int>(v.index());
}

/**
 * @brief Return whether the variant currently holds the requested type.
 * @return Whether the variant currently holds the requested type.
 */
template <class T, class A, class B, class C>
bool holds_alternative(const variant3<A, B, C>& v) {
  return variant_index_of<T, A, B, C>::value == static_cast<int>(v.index());
}

/**
 * @brief Return whether the variant currently holds the requested type.
 * @return Whether the variant currently holds the requested type.
 */
template <class T, class A, class B, class C, class D>
bool holds_alternative(const variant4<A, B, C, D>& v) {
  return variant_index_of<T, A, B, C, D>::value == static_cast<int>(v.index());
}

} // namespace sstl

#endif

