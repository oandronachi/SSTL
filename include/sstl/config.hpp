#ifndef SSTL_CONFIG_HPP
/** @def SSTL_CONFIG_HPP
 * @brief Include guard for config.hpp.
 */
#define SSTL_CONFIG_HPP

/**
 * @file config.hpp
 * @brief Core C++03 configuration, error policy selection, and inline-storage helpers.
 *
 * This header is intentionally small because almost every public SSTL header
 * includes it. It centralizes the compiler-specific alignment hooks, error
 * policy constants, fixed-storage construction helpers, and default comparison
 * utilities used by the containers. Keep it free of hosted STL dependencies:
 * the implementation may be used in freestanding or embedded builds where only
 * the language runtime and placement new are available.
 *
 * This project is intentionally C++03-first. That means no constexpr,
 * no noexcept, no initializer_list, and no standard aligned_storage. The error
 * policy is a compile-time integer so both C and C++ tests can use preprocessor
 * checks such as `#if SSTL_ON_ERROR == SSTL_RETURN`.
 */

/**
 * @dir include
 * @brief Public include tree for SSTL headers.
 */

/**
 * @dir include/sstl
 * @brief Public C++03 SSTL header directory.
 */

/**
 * @dir include/sstl/c
 * @brief Public C99 SSTL header directory.
 */

#include <stddef.h>
#include <new>

/** @brief Error policy: bounds/capacity violations call the configured panic hook. */
#define SSTL_PANIC 1
/** @brief Error policy: operations report failure through bool/null/end-style returns. */
#define SSTL_RETURN 2
/** @brief Error policy: contract-required caller checks may be omitted; violations are undefined behavior. */
#define SSTL_UB 3

#ifndef SSTL_ON_ERROR
/** @brief Debug-build default error policy used when applications do not select one. */
# ifndef NDEBUG
#  define SSTL_ON_ERROR SSTL_PANIC
/** @brief Release-build default error policy used when applications do not select one. */
# else
#  define SSTL_ON_ERROR SSTL_RETURN
# endif
#endif

#ifndef SSTL_TRAP
# if defined(__GNUC__) || defined(__clang__)
/** @def SSTL_TRAP
 * @brief Terminate immediately through the compiler trap intrinsic.
 */
#  define SSTL_TRAP() __builtin_trap()
# else
/** @def SSTL_TRAP
 * @brief Terminate immediately through a volatile null write fallback.
 */
#  define SSTL_TRAP() do { *((volatile int*)0) = 0; for (;;) {} } while (0)
# endif
#endif

#ifndef SSTL_UNREACHABLE
# if defined(__GNUC__) || defined(__clang__)
/** @def SSTL_UNREACHABLE
 * @brief Mark a defensive guard as unreachable after a violated UB-policy contract.
 */
#  define SSTL_UNREACHABLE() __builtin_unreachable()
# elif defined(_MSC_VER)
/** @def SSTL_UNREACHABLE
 * @brief Mark a defensive guard as unreachable after a violated UB-policy contract.
 */
#  define SSTL_UNREACHABLE() __assume(0)
# else
/** @def SSTL_UNREACHABLE
 * @brief Fall back to trapping when a compiler has no unreachable intrinsic.
 */
#  define SSTL_UNREACHABLE() SSTL_TRAP()
# endif
#endif

#if defined(_MSC_VER)
# ifndef SSTL_ALIGNAS
/** @def SSTL_ALIGNAS
 * @brief Apply explicit object or member alignment with MSVC syntax.
 */
#  define SSTL_ALIGNAS(N) __declspec(align(N))
# endif
# ifndef SSTL_ALIGNOF
/** @def SSTL_ALIGNOF
 * @brief Return the compiler-reported alignment requirement for type `T` on MSVC.
 * @return The compiler-reported alignment requirement for type `t` on msvc.
 */
#  define SSTL_ALIGNOF(T) __alignof(T)
# endif
/** @def SSTL_ALIGNED_MEMBER
 * @brief Apply MSVC member alignment so inline byte storage is safe for `T`.
 */
# define SSTL_ALIGNED_MEMBER(T) __declspec(align(__alignof(T)))
#else
# ifndef SSTL_ALIGNAS
/** @def SSTL_ALIGNAS
 * @brief Apply explicit object or member alignment with GCC/Clang syntax.
 */
#  define SSTL_ALIGNAS(N) __attribute__((aligned(N)))
# endif
# ifndef SSTL_ALIGNOF
/** @def SSTL_ALIGNOF
 * @brief Return the compiler-reported alignment requirement for type `T` on GCC/Clang.
 * @return The compiler-reported alignment requirement for type `t` on gcc/clang.
 */
#  define SSTL_ALIGNOF(T) __alignof__(T)
# endif
/** @def SSTL_ALIGNED_MEMBER
 * @brief Apply GCC/Clang member alignment so inline byte storage is safe for `T`.
 */
# define SSTL_ALIGNED_MEMBER(T) __attribute__((aligned(__alignof__(T))))
#endif

#if defined(SSTL_TEST_ITERATOR_DEBUG) && !defined(SSTL_TEST_ITERATOR_IS_VALID)
/** @def SSTL_TEST_ITERATOR_IS_VALID
 * @brief Test-only adapter that asks a container whether an iterator belongs to its active epoch.
 */
# define SSTL_TEST_ITERATOR_IS_VALID(container, iterator) ((container).is_valid_iterator(iterator))
#endif

#ifndef SSTL_STATIC_ASSERT
/** @brief C++03-compatible static assertion with a caller-supplied diagnostic token. */
#define SSTL_STATIC_ASSERT(cond, name) \
  typedef char sstl_static_assert_##name[(cond) ? 1 : -1]; \
  enum { sstl_static_assert_use_##name = sizeof(sstl_static_assert_##name) }
#endif

#if SSTL_ON_ERROR == SSTL_PANIC && !defined(SSTL_NO_DEFAULT_PANIC) && !defined(SSTL_PANIC_HOOK_DEFINED)
/*
  Header-only default panic hook.

  Embedded applications normally provide their own sstl_panic, but tests and
  quick-start builds need a linkable default. The hook is an inline C-linkage
  function so the C and C++ config headers agree when both are included by a
  mixed-language adapter. The default traps immediately so panic-policy builds
  fail loudly even when the application has not installed a project-specific
  hook. A project-wide hook can still be supplied by defining
  SSTL_NO_DEFAULT_PANIC.
*/
/**
 * @brief Default C-linkage panic hook used when the application does not provide one.
 * @param msg Static diagnostic text identifying the failed operation.
 *
 * The default hook deliberately traps so panic-policy builds fail loudly during
 * bring-up and testing. Define `SSTL_NO_DEFAULT_PANIC` when the application
 * supplies its own hook elsewhere.
 */
extern "C" inline void sstl_panic(const char* msg) { (void)msg; SSTL_TRAP(); } /* LCOV_EXCL_LINE: the default hook intentionally terminates the process. */
/** @def SSTL_PANIC_HOOK_DEFINED
 * @brief Marks that a C-linkage default panic hook was already supplied.
 */
# define SSTL_PANIC_HOOK_DEFINED 1
#elif SSTL_ON_ERROR == SSTL_PANIC && defined(SSTL_NO_DEFAULT_PANIC)
/**
 * @brief Application-provided panic hook used when default hook generation is disabled.
 * @param msg Diagnostic text passed to the panic hook.
 */
extern "C" void sstl_panic(const char* msg);
#endif

namespace sstl {

/** @brief SSTL-local alias for the implementation platform's object size type. */
typedef ::size_t size_t;
/** @brief SSTL-local alias for the implementation platform's pointer-difference type. */
typedef ::ptrdiff_t ptrdiff_t;

/**
 * @brief Dispatch a contract violation according to `SSTL_ON_ERROR`.
 * @param msg Static diagnostic string identifying the failed operation.
 *
 * Under `SSTL_PANIC`, this calls the active panic hook. Under `SSTL_UB`, the
 * caller must already satisfy the operation contract; implementations may omit
 * recovery checks, and any retained defensive guard is marked unreachable
 * instead of returning a failure sentinel. Under `SSTL_RETURN`, this helper is
 * intentionally silent and callers return their documented failure sentinel.
 */
inline void handle_error(const char* msg) {
#if SSTL_ON_ERROR == SSTL_PANIC
  ::sstl_panic(msg);
#elif SSTL_ON_ERROR == SSTL_UB
  (void)msg;
  SSTL_UNREACHABLE();
#else
  (void)msg;
#endif
}

/* LCOV_EXCL_START: fail_* helpers intentionally terminate for non-sentinel contracts. */
/**
 * @brief Report a contract violation for APIs that cannot return a sentinel.
 * @param msg Static diagnostic string identifying the failed operation.
 *
 * Reference-returning and value-returning APIs such as `front()`, `at()`,
 * `operator[]`, `optional::value()`, and `variant::get<I>()` cannot express
 * recoverable failure through their return type. Under RETURN policy, callers
 * must use the corresponding `try_*` API for sentinel/status behavior. This
 * helper guarantees invalid calls never silently continue into non-live storage
 * or shared fallback objects.
 */
inline void fail_contract(const char* msg) {
  handle_error(msg);
  SSTL_TRAP();
}

/**
 * @brief Fail a contract for an API whose signature requires `T&`.
 * @tparam T Referenced object type.
 * @param msg Static diagnostic string identifying the failed operation.
 * @return Never returns.
 */
template <class T>
inline T& fail_reference(const char* msg) {
  fail_contract(msg);
#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wnull-dereference"
#endif
  return *static_cast<T*>(0);
#if defined(__clang__)
# pragma clang diagnostic pop
#endif
}

/**
 * @brief Return adapter used by fail_value for non-reference return types.
 * @tparam T Value return type.
 */
template <class T>
struct fail_value_return {
  /**
   * @brief Fail the active contract and satisfy a value-returning signature.
   * @param msg Static diagnostic string identifying the failed operation.
   * @return Never returns.
   */
  static T get(const char* msg) {
    fail_contract(msg);
#if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wnull-dereference"
#endif
    return *static_cast<T*>(0);
#if defined(__clang__)
# pragma clang diagnostic pop
#endif
  }
};

/**
 * @brief Return adapter used by fail_value for reference return types.
 * @tparam T Referenced object type.
 */
template <class T>
struct fail_value_return<T&> {
  /**
   * @brief Fail the active contract and satisfy a reference-returning signature.
   * @param msg Static diagnostic string identifying the failed operation.
   * @return Never returns.
   */
  static T& get(const char* msg) {
    return fail_reference<T>(msg);
  }
};

/** @brief Return adapter used by fail_value for void return types. */
template <>
struct fail_value_return<void> {
  /**
   * @brief Fail the active contract and satisfy a void-returning signature.
   * @param msg Static diagnostic string identifying the failed operation.
   */
  static void get(const char* msg) {
    fail_contract(msg);
  }
};

/**
 * @brief Fail a contract for an API whose signature requires a value return.
 * @tparam T Return type.
 * @param msg Static diagnostic string identifying the failed operation.
 * @return Never returns.
 */
template <class T>
inline T fail_value(const char* msg) {
  return fail_value_return<T>::get(msg);
}

/** @brief Void specialization for fail_value. */
template <>
inline void fail_value<void>(const char* msg) {
  fail_value_return<void>::get(msg);
}
/* LCOV_EXCL_STOP */

/**
 * @brief Construct an object in caller-owned raw storage.
 * @tparam T Object type to construct.
 * @param p Suitably aligned storage large enough for `T`.
 * @param value Source value copied into the new object.
 * @return Pointer to the constructed object.
 */
template <class T>
inline T* construct_at(void* p, const T& value) {
  return new (p) T(value);
}

/**
 * @brief Construct an object through typed caller-owned raw storage.
 * @tparam T Object type to construct.
 * @param p Suitably aligned typed storage large enough for `T`.
 * @param value Source value copied into the new object.
 * @return Pointer to the constructed object.
 */
template <class T>
inline T* construct_at(T* p, const T& value) {
  return new (static_cast<void*>(p)) T(value);
}

/**
 * @brief Construct a const object through typed caller-owned raw storage.
 * @tparam T Object type to construct.
 * @param p Suitably aligned typed storage large enough for `T`.
 * @param value Source value copied into the new object.
 * @return Pointer to the constructed object.
 */
template <class T>
inline const T* construct_at(const T* p, const T& value) {
  return new (const_cast<void*>(static_cast<const void*>(p))) const T(value);
}

/**
 * @brief Destroy an object previously created by `construct_at`.
 * @tparam T Object type to destroy.
 * @param p Pointer to a live object.
 */
template <class T>
inline void destroy_at(T* p) {
  p->~T();
}

/** @def SSTL_CONSTRUCT_AT
 * @brief Audited placement-construction shim for caller-owned SSTL storage.
 */
#define SSTL_CONSTRUCT_AT(ptr, value) ::sstl::construct_at((ptr), (value))

/** @def SSTL_DESTROY_AT
 * @brief Audited destruction shim for objects created in caller-owned SSTL storage.
 */
#define SSTL_DESTROY_AT(ptr) ::sstl::destroy_at((ptr))

/**
 * @brief Recursive primality step for compile-time hash bucket sizing.
 * @tparam Candidate Number being tested for primality.
 * @tparam Divisor Current trial divisor.
 * @tparam Done True when no remaining divisor can prove compositeness.
 */
template <size_t Candidate, size_t Divisor, bool Done>
struct prime_test_step;

/**
 * @brief Terminal primality step reached when `Divisor * Divisor > Candidate`.
 */
template <size_t Candidate, size_t Divisor>
struct prime_test_step<Candidate, Divisor, true> {
  /** @brief Reports that no divisor was found and the candidate is prime. */
  static const bool value = true;
};

/**
 * @brief Recursive primality step that rejects divisible candidates.
 */
template <size_t Candidate, size_t Divisor>
struct prime_test_step<Candidate, Divisor, false> {
  /** @brief True when `Candidate` survives this divisor and all later divisors. */
  static const bool value = (Candidate % Divisor != 0u) &&
                            prime_test_step<Candidate, Divisor + 1u,
                              ((Divisor + 1u) > Candidate / (Divisor + 1u))>::value;
};

/**
 * @brief Selects the small-number or trial-division primality path.
 * @tparam Candidate Number being tested.
 * @tparam TooSmall True for values below the first prime.
 */
template <size_t Candidate, bool TooSmall>
struct prime_test_impl;

/**
 * @brief Reports non-prime for 0 and 1.
 */
template <size_t Candidate>
struct prime_test_impl<Candidate, true> {
  /** @brief False because values below 2 are not prime. */
  static const bool value = false;
};

/**
 * @brief Runs trial division for candidates greater than or equal to 2.
 */
template <size_t Candidate>
struct prime_test_impl<Candidate, false> {
  /** @brief True when trial division finds no divisor. */
  static const bool value = prime_test_step<Candidate, 2u, (2u > Candidate / 2u)>::value;
};

/**
 * @brief Compile-time primality predicate used by unordered containers.
 * @tparam Candidate Number to test.
 */
template <size_t Candidate>
struct is_prime {
  /** @brief True when `Candidate` is prime. */
  static const bool value = prime_test_impl<Candidate, (Candidate < 2u)>::value;
};

/**
 * @brief Recursive search state for the first prime at or above a candidate.
 * @tparam Candidate Number currently being considered.
 * @tparam Prime True when the current candidate is already prime.
 */
template <size_t Candidate, bool Prime>
struct next_prime_find;

/**
 * @brief Terminal prime search state.
 */
template <size_t Candidate>
struct next_prime_find<Candidate, true> {
  /** @brief The first prime found by the recursive search. */
  static const size_t value = Candidate;
};

/**
 * @brief Recursive prime search state that advances to the next candidate.
 */
template <size_t Candidate>
struct next_prime_find<Candidate, false> {
  /** @brief First prime greater than the current composite candidate. */
  static const size_t value = next_prime_find<Candidate + 1u, is_prime<Candidate + 1u>::value>::value;
};

/**
 * @brief Compile-time default bucket count for fixed-capacity unordered containers.
 * @tparam N Requested minimum bucket count.
 */
template <size_t N>
struct next_prime_ge {
  /** @brief First prime number greater than or equal to `N`. */
  static const size_t value = next_prime_find<N, is_prime<N>::value>::value;
};

/**
 * @brief Comparator that orders values in ascending `operator<` order.
 */
template <class T>
struct less {
  /**
   * @brief Return true when `a` is ordered before `b` using `operator<`.
   * @return True when `a` is ordered before `b` using `operator<`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  bool operator()(const T& a, const T& b) const { return a < b; }
};

/** @brief Comparator that orders values in descending `operator<` order. */
template <class T>
struct greater {
  /**
   * @brief Return true when `a` is ordered after `b` using `operator<`.
   * @return True when `a` is ordered after `b` using `operator<`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  bool operator()(const T& a, const T& b) const { return b < a; }
};

/** @brief Equality predicate that delegates to `operator==`. */
template <class T>
struct equal_to {
  /**
   * @brief Return true when `a` and `b` compare equal.
   * @return True when `a` and `b` compare equal.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  bool operator()(const T& a, const T& b) const { return a == b; }
};

/** @brief Primary hash template. Only documented SSTL hash specializations are defined. */
template <class T>
struct hash;

/** @brief Hash an unsigned integer value with FNV-1a over its stable value bytes. */
inline unsigned hash_unsigned_long_value(unsigned long v) {
  unsigned h = 2166136261u;
  for (unsigned i = 0u; i != sizeof(v); ++i) {
    h ^= static_cast<unsigned char>((v >> (i * 8u)) & 0xffu);
    h *= 16777619u;
  }
  return h;
}

/** @brief Hash a pointer object representation with FNV-1a. */
template <class T>
inline unsigned hash_pointer_value(T* p) {
  const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&p);
  unsigned h = 2166136261u;
  for (unsigned i = 0u; i != sizeof(p); ++i) {
    h ^= bytes[i];
    h *= 16777619u;
  }
  return h;
}

#define SSTL_DEFINE_INTEGRAL_HASH(type_name) \
template <> \
struct hash<type_name> { \
  unsigned operator()(type_name v) const { return hash_unsigned_long_value(static_cast<unsigned long>(v)); } \
}

SSTL_DEFINE_INTEGRAL_HASH(bool);
SSTL_DEFINE_INTEGRAL_HASH(char);
SSTL_DEFINE_INTEGRAL_HASH(signed char);
SSTL_DEFINE_INTEGRAL_HASH(unsigned char);
SSTL_DEFINE_INTEGRAL_HASH(short);
SSTL_DEFINE_INTEGRAL_HASH(unsigned short);
SSTL_DEFINE_INTEGRAL_HASH(int);
SSTL_DEFINE_INTEGRAL_HASH(unsigned);
SSTL_DEFINE_INTEGRAL_HASH(long);
SSTL_DEFINE_INTEGRAL_HASH(unsigned long);

#undef SSTL_DEFINE_INTEGRAL_HASH

/** @brief Hash specialization for object pointers. */
template <class T>
struct hash<T*> {
  unsigned operator()(T* p) const { return hash_pointer_value(p); }
};

/**
 * @brief Inline byte storage for `N` instances of `T`.
 *
 * Containers use this instead of heap allocation. The zero-capacity case keeps
 * a one-byte dummy buffer because C++ does not allow zero-length arrays, while
 * the public capacity remains exactly zero and no `T` is ever constructed.
 */
template <class T, size_t N>
struct raw_storage {
  /*
    Zero-capacity owning containers are valid public types.  We therefore
    reserve one byte worth of storage when N==0, but never construct a T in it.
    The alignment attribute keeps the container itself suitably aligned for T
    when N>0.
  */
  /** @brief Raw aligned byte buffer; `T` objects are constructed here explicitly. */
  SSTL_ALIGNED_MEMBER(T) unsigned char bytes[(N == 0 ? 1 : N) * sizeof(T)];

  /**
   * @brief Return a mutable typed pointer to slot `i` within the raw byte buffer.
   * @param i Zero-based logical index.
   * @return A mutable typed pointer to slot `i` within the raw byte buffer.
   */
  T* ptr(unsigned i) { return reinterpret_cast<T*>(bytes + i * sizeof(T)); }
  /**
   * @brief Return a const typed pointer to slot `i` within the raw byte buffer.
   * @param i Zero-based logical index.
   * @return A const typed pointer to slot `i` within the raw byte buffer.
   */
  const T* ptr(unsigned i) const { return reinterpret_cast<const T*>(bytes + i * sizeof(T)); }
};

} // namespace sstl

#endif

