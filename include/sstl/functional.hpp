/**
 * @file functional.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_FUNCTIONAL_HPP
/** @def SSTL_FUNCTIONAL_HPP
 * @brief Include guard for functional.hpp.
 */
#define SSTL_FUNCTIONAL_HPP

#include "config.hpp"

namespace sstl {

/** @brief Inequality predicate that delegates to `operator!=`. */
template <class T>
struct not_equal_to {
  /**
   * @brief Return true when `a` and `b` compare unequal.
   * @return True when `a` and `b` compare unequal.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  bool operator()(const T& a, const T& b) const { return a != b; }
};

/** @brief Addition function object that delegates to `operator+`. */
template <class T>
struct plus {
  /**
   * @brief Return `a + b`.
   * @return `a + b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  T operator()(const T& a, const T& b) const { return a + b; }
};

/** @brief Subtraction function object that delegates to `operator-`. */
template <class T>
struct minus {
  /**
   * @brief Return `a - b`.
   * @return `a - b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  T operator()(const T& a, const T& b) const { return a - b; }
};

/** @brief Multiplication function object that delegates to `operator*`. */
template <class T>
struct multiplies {
  /**
   * @brief Return `a * b`.
   * @return `a * b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  T operator()(const T& a, const T& b) const { return a * b; }
};

/** @brief Division function object that delegates to `operator/`. */
template <class T>
struct divides {
  /**
   * @brief Return `a / b`.
   * @return `a / b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  T operator()(const T& a, const T& b) const { return a / b; }
};

/** @brief Remainder function object that delegates to `operator%`. */
template <class T>
struct modulus {
  /**
   * @brief Return `a % b`.
   * @return `a % b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  T operator()(const T& a, const T& b) const { return a % b; }
};

/** @brief Arithmetic negation function object that delegates to unary `operator-`. */
template <class T>
struct negate {
  /**
   * @brief Return `-v`.
   * @return `-v`.
   * @param v Value or object being inspected.
   */
  T operator()(const T& v) const { return -v; }
};

/** @brief Logical AND function object that delegates to `operator&&`. */
template <class T>
struct logical_and {
  /**
   * @brief Return `a && b`.
   * @return `a && b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  bool operator()(const T& a, const T& b) const { return a && b; }
};

/** @brief Logical OR function object that delegates to `operator||`. */
template <class T>
struct logical_or {
  /**
   * @brief Return `a || b`.
   * @return `a || b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  bool operator()(const T& a, const T& b) const { return a || b; }
};

/** @brief Logical NOT function object that delegates to `operator!`. */
template <class T>
struct logical_not {
  /**
   * @brief Return `!v`.
   * @return `!v`.
   * @param v Value or object being inspected.
   */
  bool operator()(const T& v) const { return !v; }
};

/** @brief Bitwise AND function object that delegates to `operator&`. */
template <class T>
struct bit_and {
  /**
   * @brief Return `a & b`.
   * @return `a & b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  T operator()(const T& a, const T& b) const { return a & b; }
};

/** @brief Bitwise OR function object that delegates to `operator|`. */
template <class T>
struct bit_or {
  /**
   * @brief Return `a | b`.
   * @return `a | b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  T operator()(const T& a, const T& b) const { return a | b; }
};

/** @brief Bitwise XOR function object that delegates to `operator^`. */
template <class T>
struct bit_xor {
  /**
   * @brief Return `a ^ b`.
   * @return `a ^ b`.
   * @param a First operand or first range start.
   * @param b Second operand or second range start.
   */
  T operator()(const T& a, const T& b) const { return a ^ b; }
};

/** @brief Bitwise NOT function object that delegates to unary `operator~`. */
template <class T>
struct bit_not {
  /**
   * @brief Return `~v`.
   * @return `~v`.
   * @param v Value or object being inspected.
   */
  T operator()(const T& v) const { return ~v; }
};

} // namespace sstl

#endif

