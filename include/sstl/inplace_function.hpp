/**
 * @file inplace_function.hpp
 * @brief Fixed-storage callable wrappers for C++03 SSTL.
 *
 * `function0`..`function3` store small callable objects directly inside the
 * wrapper. The implementation avoids heap allocation and standard library type
 * erasure machinery while still supporting stateful functors.
 */
#ifndef SSTL_INPLACE_FUNCTION_HPP
/** @def SSTL_INPLACE_FUNCTION_HPP
 * @brief Include guard for inplace_function.hpp.
 */
#define SSTL_INPLACE_FUNCTION_HPP

#include "config.hpp"

namespace sstl {

/** @brief Aligned inline byte storage used by fixed-capacity callable wrappers. */
template <unsigned Sz>
struct function_storage {
  /** @brief Alignment carrier and byte buffer for one stored callable object. */
  union block {
    /** @brief Pointer-sized alignment member. */
    void* p;
    /** @brief Long alignment member. */
    long l;
    /** @brief Double alignment member. */
    double d;
    /**
     * @brief Function-pointer alignment member.
     */
    void (*fn)();
    /** @brief Inline byte buffer; keeps one dummy byte when `Sz == 0`. */
    unsigned char bytes[Sz == 0 ? 1 : Sz];
  };
  /** @brief Storage block instance that provides both alignment and byte access. */
  block data;
/**
 * @brief Return a pointer into the inline storage buffer.
 * @return Result described by the function brief.
 */
  void* ptr() { return data.bytes; }
/**
 * @brief Return a pointer into the inline storage buffer.
 * @return Result described by the function brief.
 */
  const void* ptr() const { return data.bytes; }
};

/** @brief Fixed-storage callable wrapper for nullary callables. */
template <class R, unsigned Sz = 32>
class function0 {
public:
/** @brief Construct a function0 object while initializing its fixed inline storage state. */
  function0() : invoke_(0), destroy_(0), copy_(0) {}
/**
 * @brief Construct the callable wrapper from a plain function pointer target.
 * @param fn Callable target or function pointer.
 */
  function0(R (*fn)()) : invoke_(0), destroy_(0), copy_(0) { assign(fn); }
/**
 * @brief Construct a function0 object while initializing its fixed inline storage state.
 * @param f Callable object or function wrapper.
 */
  template <class F> function0(const F& f) : invoke_(0), destroy_(0), copy_(0) { assign(f); }
/**
 * @brief Copy-construct by cloning the other wrapper's inline target when present.
 * @param other Other object participating in the operation.
 */
  function0(const function0& other) : invoke_(0), destroy_(0), copy_(0) { other.copy_to(*this); }
/** @brief Destroy function0 and release any live inline-owned values. */
  ~function0() { reset(); }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  function0& operator=(const function0& other) { if (this != &other) { reset(); other.copy_to(*this); } return *this; }
/** @brief Report whether this wrapper currently contains a callable or value. */
  operator bool() const { return invoke_ != 0; }
/**
 * @brief Invoke the stored callable through the fixed-storage wrapper.
 * @return Result described by the function brief.
 */
  R operator()() const { return invoke_(const_cast<void*>(storage_.ptr())); }
/** @brief Reset this wrapper or storage object to its empty state. */
  void reset() { if (destroy_) destroy_(storage_.ptr()); invoke_ = 0; destroy_ = 0; copy_ = 0; }
/**
 * @brief Replace the current target with a callable copied into inline storage.
 * @param f Callable object or function wrapper.
 */
  template <class F> void assign(const F& f) {
    SSTL_STATIC_ASSERT(sizeof(F) <= Sz, function0_target_fits);
    reset();
    SSTL_CONSTRUCT_AT(reinterpret_cast<F*>(storage_.ptr()), f);
    invoke_ = &invoke_impl<F>;
    destroy_ = &destroy_impl<F>;
    copy_ = &copy_impl<F>;
  }
private:
  /** @brief Erased invocation function pointer type. */
  typedef R (*invoke_fn)(void*);
  /** @brief Erased destruction function pointer type. */
  typedef void (*destroy_fn)(void*);
  /** @brief Erased copy function pointer type. */
  typedef void (*copy_fn)(void*, const void*);
  /**
   * @brief Invoke a stored callable of concrete type `F`.
   * @param p Erased storage pointer.
   * @return Result described by the function brief.
   */
  template <class F> static R invoke_impl(void* p) { return (*reinterpret_cast<F*>(p))(); }
  /**
   * @brief Destroy a stored callable of concrete type `F`.
   * @param p Erased storage pointer.
   */
  template <class F> static void destroy_impl(void* p) { reinterpret_cast<F*>(p)->~F(); }
/**
 * @brief Copy the stored callable state into another wrapper.
 * @param dst Caller-provided destination storage.
 * @param src Caller-provided source data.
 */
  template <class F> static void copy_impl(void* dst, const void* src) { SSTL_CONSTRUCT_AT(reinterpret_cast<F*>(dst), *reinterpret_cast<const F*>(src)); }
  /**
   * @brief Copy this wrapper's callable target into `out` when one exists.
   * @param out Caller-provided destination for produced values.
   */
  void copy_to(function0& out) const { if (copy_) { copy_(out.storage_.ptr(), storage_.ptr()); out.invoke_ = invoke_; out.destroy_ = destroy_; out.copy_ = copy_; } }
  /** @brief Inline storage for the erased callable target. */
  function_storage<Sz> storage_;
  /** @brief Erased invocation thunk, or null when empty. */
  invoke_fn invoke_;
  /** @brief Erased destruction thunk, or null when empty. */
  destroy_fn destroy_;
  /** @brief Erased copy thunk, or null when empty. */
  copy_fn copy_;
};

/** @brief Fixed-storage callable wrapper for one-argument callables. */
template <class R, class A0, unsigned Sz = 32>
class function1 {
public:
/** @brief Construct a function1 object while initializing its fixed inline storage state. */
  function1() : invoke_(0), destroy_(0), copy_(0) {}
/**
 * @brief Construct the callable wrapper from a plain function pointer target.
 * @param fn Callable target or function pointer.
 */
  function1(R (*fn)(A0)) : invoke_(0), destroy_(0), copy_(0) { assign(fn); }
/**
 * @brief Construct a function1 object while initializing its fixed inline storage state.
 * @param f Callable object or function wrapper.
 */
  template <class F> function1(const F& f) : invoke_(0), destroy_(0), copy_(0) { assign(f); }
/**
 * @brief Copy-construct by cloning the other wrapper's inline target when present.
 * @param other Other object participating in the operation.
 */
  function1(const function1& other) : invoke_(0), destroy_(0), copy_(0) { other.copy_to(*this); }
/** @brief Destroy function1 and release any live inline-owned values. */
  ~function1() { reset(); }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  function1& operator=(const function1& other) { if (this != &other) { reset(); other.copy_to(*this); } return *this; }
/** @brief Report whether this wrapper currently contains a callable or value. */
  operator bool() const { return invoke_ != 0; }
/**
 * @brief Invoke the stored callable through the fixed-storage wrapper.
 * @return Result described by the function brief.
 * @param a0 First callable argument.
 */
  R operator()(A0 a0) const { return invoke_(const_cast<void*>(storage_.ptr()), a0); }
/** @brief Reset this wrapper or storage object to its empty state. */
  void reset() { if (destroy_) destroy_(storage_.ptr()); invoke_ = 0; destroy_ = 0; copy_ = 0; }
/**
 * @brief Replace the current target with a callable copied into inline storage.
 * @param f Callable object or function wrapper.
 */
  template <class F> void assign(const F& f) {
    SSTL_STATIC_ASSERT(sizeof(F) <= Sz, function1_target_fits);
    reset();
    SSTL_CONSTRUCT_AT(reinterpret_cast<F*>(storage_.ptr()), f);
    invoke_ = &invoke_impl<F>;
    destroy_ = &destroy_impl<F>;
    copy_ = &copy_impl<F>;
  }
private:
  /** @brief Erased invocation function pointer type. */
  typedef R (*invoke_fn)(void*, A0);
  /** @brief Erased destruction function pointer type. */
  typedef void (*destroy_fn)(void*);
  /** @brief Erased copy function pointer type. */
  typedef void (*copy_fn)(void*, const void*);
  /**
   * @brief Invoke a stored callable of concrete type `F`.
   * @param p Erased storage pointer.
   * @param a0 First callable argument.
   * @return Result described by the function brief.
   */
  template <class F> static R invoke_impl(void* p, A0 a0) { return (*reinterpret_cast<F*>(p))(a0); }
  /**
   * @brief Destroy a stored callable of concrete type `F`.
   * @param p Erased storage pointer.
   */
  template <class F> static void destroy_impl(void* p) { reinterpret_cast<F*>(p)->~F(); }
/**
 * @brief Copy the stored callable state into another wrapper.
 * @param dst Caller-provided destination storage.
 * @param src Caller-provided source data.
 */
  template <class F> static void copy_impl(void* dst, const void* src) { SSTL_CONSTRUCT_AT(reinterpret_cast<F*>(dst), *reinterpret_cast<const F*>(src)); }
  /**
   * @brief Copy this wrapper's callable target into `out` when one exists.
   * @param out Caller-provided destination for produced values.
   */
  void copy_to(function1& out) const { if (copy_) { copy_(out.storage_.ptr(), storage_.ptr()); out.invoke_ = invoke_; out.destroy_ = destroy_; out.copy_ = copy_; } }
  /** @brief Inline storage for the erased callable target. */
  function_storage<Sz> storage_;
  /** @brief Erased invocation thunk, or null when empty. */
  invoke_fn invoke_;
  /** @brief Erased destruction thunk, or null when empty. */
  destroy_fn destroy_;
  /** @brief Erased copy thunk, or null when empty. */
  copy_fn copy_;
};

/** @brief Fixed-storage callable wrapper for two-argument callables. */
template <class R, class A0, class A1, unsigned Sz = 32>
class function2 {
public:
/** @brief Construct a function2 object while initializing its fixed inline storage state. */
  function2() : invoke_(0), destroy_(0), copy_(0) {}
/**
 * @brief Construct the callable wrapper from a plain function pointer target.
 * @param fn Callable target or function pointer.
 */
  function2(R (*fn)(A0, A1)) : invoke_(0), destroy_(0), copy_(0) { assign(fn); }
/**
 * @brief Construct a function2 object while initializing its fixed inline storage state.
 * @param f Callable object or function wrapper.
 */
  template <class F> function2(const F& f) : invoke_(0), destroy_(0), copy_(0) { assign(f); }
/**
 * @brief Copy-construct by cloning the other wrapper's inline target when present.
 * @param other Other object participating in the operation.
 */
  function2(const function2& other) : invoke_(0), destroy_(0), copy_(0) { other.copy_to(*this); }
/** @brief Destroy function2 and release any live inline-owned values. */
  ~function2() { reset(); }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  function2& operator=(const function2& other) { if (this != &other) { reset(); other.copy_to(*this); } return *this; }
/** @brief Report whether this wrapper currently contains a callable or value. */
  operator bool() const { return invoke_ != 0; }
/**
 * @brief Invoke the stored callable through the fixed-storage wrapper.
 * @return Result described by the function brief.
 * @param a0 First callable argument.
 * @param a1 Second callable argument.
 */
  R operator()(A0 a0, A1 a1) const { return invoke_(const_cast<void*>(storage_.ptr()), a0, a1); }
/** @brief Reset this wrapper or storage object to its empty state. */
  void reset() { if (destroy_) destroy_(storage_.ptr()); invoke_ = 0; destroy_ = 0; copy_ = 0; }
/**
 * @brief Replace the current target with a callable copied into inline storage.
 * @param f Callable object or function wrapper.
 */
  template <class F> void assign(const F& f) {
    SSTL_STATIC_ASSERT(sizeof(F) <= Sz, function2_target_fits);
    reset();
    SSTL_CONSTRUCT_AT(reinterpret_cast<F*>(storage_.ptr()), f);
    invoke_ = &invoke_impl<F>;
    destroy_ = &destroy_impl<F>;
    copy_ = &copy_impl<F>;
  }
private:
  /** @brief Erased invocation function pointer type. */
  typedef R (*invoke_fn)(void*, A0, A1);
  /** @brief Erased destruction function pointer type. */
  typedef void (*destroy_fn)(void*);
  /** @brief Erased copy function pointer type. */
  typedef void (*copy_fn)(void*, const void*);
  /**
   * @brief Invoke a stored callable of concrete type `F`.
   * @param p Erased storage pointer.
   * @param a0 First callable argument.
   * @param a1 Second callable argument.
   * @return Result described by the function brief.
   */
  template <class F> static R invoke_impl(void* p, A0 a0, A1 a1) { return (*reinterpret_cast<F*>(p))(a0, a1); }
  /**
   * @brief Destroy a stored callable of concrete type `F`.
   * @param p Erased storage pointer.
   */
  template <class F> static void destroy_impl(void* p) { reinterpret_cast<F*>(p)->~F(); }
/**
 * @brief Copy the stored callable state into another wrapper.
 * @param dst Caller-provided destination storage.
 * @param src Caller-provided source data.
 */
  template <class F> static void copy_impl(void* dst, const void* src) { SSTL_CONSTRUCT_AT(reinterpret_cast<F*>(dst), *reinterpret_cast<const F*>(src)); }
  /**
   * @brief Copy this wrapper's callable target into `out` when one exists.
   * @param out Caller-provided destination for produced values.
   */
  void copy_to(function2& out) const { if (copy_) { copy_(out.storage_.ptr(), storage_.ptr()); out.invoke_ = invoke_; out.destroy_ = destroy_; out.copy_ = copy_; } }
  /** @brief Inline storage for the erased callable target. */
  function_storage<Sz> storage_;
  /** @brief Erased invocation thunk, or null when empty. */
  invoke_fn invoke_;
  /** @brief Erased destruction thunk, or null when empty. */
  destroy_fn destroy_;
  /** @brief Erased copy thunk, or null when empty. */
  copy_fn copy_;
};

/** @brief Fixed-storage callable wrapper for three-argument callables. */
template <class R, class A0, class A1, class A2, unsigned Sz = 32>
class function3 {
public:
/** @brief Construct a function3 object while initializing its fixed inline storage state. */
  function3() : invoke_(0), destroy_(0), copy_(0) {}
/**
 * @brief Construct the callable wrapper from a plain function pointer target.
 * @param fn Callable target or function pointer.
 */
  function3(R (*fn)(A0, A1, A2)) : invoke_(0), destroy_(0), copy_(0) { assign(fn); }
/**
 * @brief Construct a function3 object while initializing its fixed inline storage state.
 * @param f Callable object or function wrapper.
 */
  template <class F> function3(const F& f) : invoke_(0), destroy_(0), copy_(0) { assign(f); }
/**
 * @brief Copy-construct by cloning the other wrapper's inline target when present.
 * @param other Other object participating in the operation.
 */
  function3(const function3& other) : invoke_(0), destroy_(0), copy_(0) { other.copy_to(*this); }
/** @brief Destroy function3 and release any live inline-owned values. */
  ~function3() { reset(); }
/**
 * @brief Assign from another object or alternative while preserving fixed-storage invariants.
 * @param other Other object participating in the operation.
 * @return Result described by the function brief.
 */
  function3& operator=(const function3& other) { if (this != &other) { reset(); other.copy_to(*this); } return *this; }
/** @brief Report whether this wrapper currently contains a callable or value. */
  operator bool() const { return invoke_ != 0; }
/**
 * @brief Invoke the stored callable through the fixed-storage wrapper.
 * @return Result described by the function brief.
 * @param a0 First callable argument.
 * @param a1 Second callable argument.
 * @param a2 Third callable argument.
 */
  R operator()(A0 a0, A1 a1, A2 a2) const { return invoke_(const_cast<void*>(storage_.ptr()), a0, a1, a2); }
/** @brief Reset this wrapper or storage object to its empty state. */
  void reset() { if (destroy_) destroy_(storage_.ptr()); invoke_ = 0; destroy_ = 0; copy_ = 0; }
/**
 * @brief Replace the current target with a callable copied into inline storage.
 * @param f Callable object or function wrapper.
 */
  template <class F> void assign(const F& f) {
    SSTL_STATIC_ASSERT(sizeof(F) <= Sz, function3_target_fits);
    reset();
    SSTL_CONSTRUCT_AT(reinterpret_cast<F*>(storage_.ptr()), f);
    invoke_ = &invoke_impl<F>;
    destroy_ = &destroy_impl<F>;
    copy_ = &copy_impl<F>;
  }
private:
  /** @brief Erased invocation function pointer type. */
  typedef R (*invoke_fn)(void*, A0, A1, A2);
  /** @brief Erased destruction function pointer type. */
  typedef void (*destroy_fn)(void*);
  /** @brief Erased copy function pointer type. */
  typedef void (*copy_fn)(void*, const void*);
  /**
   * @brief Invoke a stored callable of concrete type `F`.
   * @param p Erased storage pointer.
   * @param a0 First callable argument.
   * @param a1 Second callable argument.
   * @param a2 Third callable argument.
   * @return Result described by the function brief.
   */
  template <class F> static R invoke_impl(void* p, A0 a0, A1 a1, A2 a2) { return (*reinterpret_cast<F*>(p))(a0, a1, a2); }
  /**
   * @brief Destroy a stored callable of concrete type `F`.
   * @param p Erased storage pointer.
   */
  template <class F> static void destroy_impl(void* p) { reinterpret_cast<F*>(p)->~F(); }
/**
 * @brief Copy the stored callable state into another wrapper.
 * @param dst Caller-provided destination storage.
 * @param src Caller-provided source data.
 */
  template <class F> static void copy_impl(void* dst, const void* src) { SSTL_CONSTRUCT_AT(reinterpret_cast<F*>(dst), *reinterpret_cast<const F*>(src)); }
  /**
   * @brief Copy this wrapper's callable target into `out` when one exists.
   * @param out Caller-provided destination for produced values.
   */
  void copy_to(function3& out) const { if (copy_) { copy_(out.storage_.ptr(), storage_.ptr()); out.invoke_ = invoke_; out.destroy_ = destroy_; out.copy_ = copy_; } }
  /** @brief Inline storage for the erased callable target. */
  function_storage<Sz> storage_;
  /** @brief Erased invocation thunk, or null when empty. */
  invoke_fn invoke_;
  /** @brief Erased destruction thunk, or null when empty. */
  destroy_fn destroy_;
  /** @brief Erased copy thunk, or null when empty. */
  copy_fn copy_;
};

} // namespace sstl

#endif

