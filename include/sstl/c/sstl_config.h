/**
 * @file sstl_config.h
 * @brief C99 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_C_CONFIG_H
/** @def SSTL_C_CONFIG_H
 * @brief Include guard for sstl_config.h.
 */
#define SSTL_C_CONFIG_H

#include <stddef.h>
#include <stdbool.h>

#if defined(__GNUC__) || defined(__clang__)
/** @def SSTL_C_UNUSED
 * @brief Mark generated C functions as intentionally unused on GCC/Clang.
 */
# define SSTL_C_UNUSED __attribute__((unused))
#else
/** @def SSTL_C_UNUSED
 * @brief Empty portability marker for compilers without an unused-function attribute.
 */
# define SSTL_C_UNUSED
#endif

#if defined(__GNUC__) || defined(__clang__)
/** @def SSTL_C_DEPRECATED
 * @brief Mark legacy C helper hooks that remain only for source compatibility.
 */
# define SSTL_C_DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
/** @def SSTL_C_DEPRECATED
 * @brief Mark legacy C helper hooks that remain only for source compatibility.
 */
# define SSTL_C_DEPRECATED __declspec(deprecated)
#else
/** @def SSTL_C_DEPRECATED
 * @brief Empty portability marker when the compiler has no deprecation attribute.
 */
# define SSTL_C_DEPRECATED
#endif

/** @brief Canonical C three-way comparator hook over addresses of stored objects. */
typedef int (*sstl_cmp_fn)(const void* a, const void* b);
/** @brief Canonical C equality hook over addresses of stored objects. */
typedef bool (*sstl_eq_fn)(const void* a, const void* b);
/** @brief Canonical C hash hook over the address of one stored object. */
typedef size_t (*sstl_hash_fn)(const void* value);
/** @brief Canonical C predicate hook over the address of one stored object. */
typedef bool (*sstl_pred_fn)(const void* value);
/** @brief Canonical C unary transform hook writing one destination object. */
typedef void (*sstl_transform_fn)(void* dst, const void* src);

/**
 * @brief Compare two signed integers through the canonical address-based hook shape.
 * @param a Address of the first `int`.
 * @param b Address of the second `int`.
 * @return Negative when first is less, positive when first is greater, zero when equal.
 */
static SSTL_C_UNUSED int sstl_cmp_int_v(const void* a, const void* b) {
  const int av = *(const int*)a;
  const int bv = *(const int*)b;
  return (av > bv) - (av < bv);
}

/**
 * @brief Compare two unsigned integers through the canonical address-based hook shape.
 * @param a Address of the first `unsigned`.
 * @param b Address of the second `unsigned`.
 * @return Negative when first is less, positive when first is greater, zero when equal.
 */
static SSTL_C_UNUSED int sstl_cmp_uint_v(const void* a, const void* b) {
  const unsigned av = *(const unsigned*)a;
  const unsigned bv = *(const unsigned*)b;
  return (av > bv) - (av < bv);
}

/**
 * @brief Compare two `const char*` values without relying on libc.
 * @param a Address of the first `const char*`.
 * @param b Address of the second `const char*`.
 * @return Negative, positive, or zero using bytewise lexicographic ordering.
 */
static SSTL_C_UNUSED int sstl_cmp_str_v(const void* a, const void* b) {
  const char* lhs = *(const char* const*)a;
  const char* rhs = *(const char* const*)b;
  while (lhs && rhs && *lhs && *rhs && *lhs == *rhs) { ++lhs; ++rhs; }
  {
    const unsigned char lc = (unsigned char)(lhs ? *lhs : 0);
    const unsigned char rc = (unsigned char)(rhs ? *rhs : 0);
    return (lc > rc) - (lc < rc);
  }
}

/**
 * @brief Return true when two addressed `int` objects contain the same value.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return True when two addressed `int` objects contain the same value.
 */
static SSTL_C_UNUSED bool sstl_eq_int_v(const void* a, const void* b) {
  return *(const int*)a == *(const int*)b;
}

/**
 * @brief Return true when two addressed `unsigned` objects contain the same value.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return True when two addressed `unsigned` objects contain the same value.
 */
static SSTL_C_UNUSED bool sstl_eq_uint_v(const void* a, const void* b) {
  return *(const unsigned*)a == *(const unsigned*)b;
}

/**
 * @brief Return true when two addressed `const char*` objects name equal strings.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return True when two addressed `const char*` objects name equal strings.
 */
static SSTL_C_UNUSED bool sstl_eq_str_v(const void* a, const void* b) {
  return sstl_cmp_str_v(a, b) == 0;
}

/**
 * @brief Hash a raw byte span with the FNV-1a algorithm.
 * @param data First byte to hash.
 * @param n Number of bytes to include.
 * @return Deterministic non-cryptographic hash value.
 */
static SSTL_C_UNUSED size_t sstl_hash_bytes_v(const void* data, size_t n) {
  const unsigned char* p = (const unsigned char*)data;
  size_t h = (size_t)2166136261u;
  while (n--) { h ^= (size_t)*p++; h *= (size_t)16777619u; }
  return h;
}

/**
 * @brief Hash one addressed `int` object through the canonical one-argument hook.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return Result described by the function brief.
 */
static SSTL_C_UNUSED size_t sstl_hash_int_v(const void* value) {
  return sstl_hash_bytes_v(value, sizeof(int));
}

/**
 * @brief Hash one addressed `unsigned` object through the canonical one-argument hook.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return Result described by the function brief.
 */
static SSTL_C_UNUSED size_t sstl_hash_uint_v(const void* value) {
  return sstl_hash_bytes_v(value, sizeof(unsigned));
}

/**
 * @brief Hash one addressed `const char*` object by the bytes in the pointed string.
 * @param value Value supplied for comparison, assignment, insertion, or lookup.
 * @return Result described by the function brief.
 */
static SSTL_C_UNUSED size_t sstl_hash_str_v(const void* value) {
  const char* s = *(const char* const*)value;
  size_t h = (size_t)2166136261u;
  while (s && *s) { h ^= (size_t)(unsigned char)*s++; h *= (size_t)16777619u; }
  return h;
}

/* LCOV_EXCL_START: deprecated compatibility wrappers are intentionally not exercised by internal tests. */
/**
 * @brief Deprecated value-based integer comparator retained for temporary compatibility.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return Result described by the function brief.
 */
static SSTL_C_UNUSED SSTL_C_DEPRECATED int sstl_cmp_int(int a, int b) { return (a > b) - (a < b); }

/**
 * @brief Deprecated value-based unsigned comparator retained for temporary compatibility.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return Result described by the function brief.
 */
static SSTL_C_UNUSED SSTL_C_DEPRECATED int sstl_cmp_uint(unsigned a, unsigned b) { return (a > b) - (a < b); }

/**
 * @brief Deprecated value-based string comparator retained for temporary compatibility.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return Result described by the function brief.
 */
static SSTL_C_UNUSED SSTL_C_DEPRECATED int sstl_cmp_str(const char* a, const char* b) {
  while (a && b && *a && *b && *a == *b) { ++a; ++b; }
  {
    const unsigned char ac = (unsigned char)(a ? *a : 0);
    const unsigned char bc = (unsigned char)(b ? *b : 0);
    return (ac > bc) - (ac < bc);
  }
}

/**
 * @brief Deprecated value-based integer equality hook retained for temporary compatibility.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
static SSTL_C_UNUSED SSTL_C_DEPRECATED bool sstl_eq_int(int a, int b) { return a == b; }

/**
 * @brief Deprecated value-based unsigned equality hook retained for temporary compatibility.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
static SSTL_C_UNUSED SSTL_C_DEPRECATED bool sstl_eq_uint(unsigned a, unsigned b) { return a == b; }

/**
 * @brief Deprecated value-based string equality hook retained for temporary compatibility.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
static SSTL_C_UNUSED SSTL_C_DEPRECATED bool sstl_eq_str(const char* a, const char* b) {
  while (a && b && *a && *b && *a == *b) { ++a; ++b; }
  return (a ? *a : 0) == (b ? *b : 0);
}

/**
 * @brief Deprecated two-argument byte hash retained for temporary compatibility.
 * @param data Caller-supplied argument used by this operation.
 * @param n Requested count or size.
 * @return Result described by the function brief.
 */
static SSTL_C_UNUSED SSTL_C_DEPRECATED unsigned sstl_hash_bytes(const void* data, size_t n) {
  return (unsigned)sstl_hash_bytes_v(data, n);
}
/* LCOV_EXCL_STOP */

/** @brief Error policy: invalid C API calls report through `sstl_panic`. */
#define SSTL_PANIC 1
/** @brief Error policy: invalid C API calls return false/null sentinels. */
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
/** @def SSTL_ALIGNOF
 * @brief Return the compiler-reported alignment requirement for type `T` on MSVC.
 * @return The compiler-reported alignment requirement for type `t` on msvc.
 */
# define SSTL_ALIGNOF(T) __alignof(T)
#else
# ifndef SSTL_ALIGNAS
/** @def SSTL_ALIGNAS
 * @brief Apply explicit object or member alignment with GCC/Clang syntax.
 */
#  define SSTL_ALIGNAS(N) __attribute__((aligned(N)))
# endif
/** @def SSTL_ALIGNOF
 * @brief Return the compiler-reported alignment requirement for type `T` on GCC/Clang.
 * @return The compiler-reported alignment requirement for type `t` on gcc/clang.
 */
# define SSTL_ALIGNOF(T) __alignof__(T)
#endif

#if SSTL_ON_ERROR == SSTL_PANIC && !defined(SSTL_NO_DEFAULT_PANIC) && !defined(SSTL_PANIC_HOOK_DEFINED)
/*
 * The default panic hook traps immediately. C++ translation units receive an
 * inline C-linkage definition, while pure C translation units keep a
 * header-local helper so there is no cross-file symbol requirement.
 */
#if defined(__cplusplus)
/**
 * @brief Header-local default panic hook used when applications do not provide one.
 * @param msg Diagnostic text passed to the panic hook.
 */
extern "C" inline void sstl_panic(const char* msg) { (void)msg; SSTL_TRAP(); } /* LCOV_EXCL_LINE: the default hook intentionally terminates the process. */
#else
/**
 * @brief Header-local default panic hook used when applications do not provide one.
 * @param msg Diagnostic text passed to the panic hook.
 */
static SSTL_C_UNUSED void sstl_panic(const char* msg) { (void)msg; SSTL_TRAP(); } /* LCOV_EXCL_LINE: the default hook intentionally terminates the process. */
#endif
/** @def SSTL_PANIC_HOOK_DEFINED
 * @brief Marks that a compatible default panic hook was already supplied.
 */
# define SSTL_PANIC_HOOK_DEFINED 1
#elif SSTL_ON_ERROR == SSTL_PANIC && defined(SSTL_NO_DEFAULT_PANIC)
#if defined(__cplusplus)
extern "C" {
#endif
/**
 * @brief Application-provided C panic hook used when default hook generation is disabled.
 * @param msg Diagnostic text passed to the panic hook.
 */
void sstl_panic(const char* msg);
#if defined(__cplusplus)
}
#endif
#endif

#if SSTL_ON_ERROR == SSTL_PANIC
/** @brief Dispatch a C contract violation to the active panic hook. */
# define SSTL_C_PANIC(msg) sstl_panic(msg)
#elif SSTL_ON_ERROR == SSTL_UB
/** @brief Treat a C contract violation as undefined behavior instead of a recoverable failure. */
# define SSTL_C_PANIC(msg) do { (void)(msg); SSTL_UNREACHABLE(); } while (0)
#else
/** @brief Compile out C panic calls for RETURN policy builds. */
# define SSTL_C_PANIC(msg) ((void)0)
#endif

#endif

