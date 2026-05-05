/**
 * @file type_traits.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_TYPE_TRAITS_HPP
/** @def SSTL_TYPE_TRAITS_HPP
 * @brief Include guard for type_traits.hpp.
 */
#define SSTL_TYPE_TRAITS_HPP

namespace sstl {

/**
 * @brief Compile-time true/false carrier used by the small type-trait subset.
 *
 * SSTL cannot rely on C++11 `<type_traits>`, but a few utility components need
 * the same basic vocabulary. These traits intentionally expose only the pieces
 * used by the headers so the library remains small and freestanding-friendly.
 */
template <bool V>
struct bool_constant {
  enum { value = V };
};

/** @brief SFINAE helper with no nested type when the condition is false. */
template <bool Cond, class T = void>
struct enable_if {};

/** @brief SFINAE helper specialization exposing `type` when the condition is true. */
template <class T>
struct enable_if<true, T> {
  /** @brief Type made available only for the true specialization. */
  typedef T type;
};

/** @brief Trait that reports whether two types are exactly the same type. */
template <class A, class B>
struct is_same : bool_constant<false> {};

/** @brief `is_same` specialization for identical types. */
template <class A>
struct is_same<A, A> : bool_constant<true> {};

/** @brief Trait that reports whether a type is one of the supported integral spellings. */
template <class T>
struct is_integral : bool_constant<false> {};

/** @brief `is_integral` specialization for bool. */
template <>
struct is_integral<bool> : bool_constant<true> {};

/** @brief `is_integral` specialization for char. */
template <>
struct is_integral<char> : bool_constant<true> {};

/** @brief `is_integral` specialization for signed char. */
template <>
struct is_integral<signed char> : bool_constant<true> {};

/** @brief `is_integral` specialization for unsigned char. */
template <>
struct is_integral<unsigned char> : bool_constant<true> {};

/** @brief `is_integral` specialization for short. */
template <>
struct is_integral<short> : bool_constant<true> {};

/** @brief `is_integral` specialization for unsigned short. */
template <>
struct is_integral<unsigned short> : bool_constant<true> {};

/** @brief `is_integral` specialization for int. */
template <>
struct is_integral<int> : bool_constant<true> {};

/** @brief `is_integral` specialization for unsigned int. */
template <>
struct is_integral<unsigned int> : bool_constant<true> {};

/** @brief `is_integral` specialization for long. */
template <>
struct is_integral<long> : bool_constant<true> {};

/** @brief `is_integral` specialization for unsigned long. */
template <>
struct is_integral<unsigned long> : bool_constant<true> {};

/**
 * @brief Marker for future optimized relocation paths.
 *
 * The default remains conservative. Containers destroy and reconstruct
 * elements explicitly unless a future specialization opts into a faster path.
 */
template <class T>
struct is_relocatable {
  enum { value = 0 };
};

} // namespace sstl

#endif

