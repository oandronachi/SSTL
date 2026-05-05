/**
 * @file optional.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_OPTIONAL_HPP
/** @def SSTL_OPTIONAL_HPP
 * @brief Include guard for optional.hpp.
 */
#define SSTL_OPTIONAL_HPP

#include "config.hpp"

namespace sstl {

/** @brief Tag type used to construct or assign an empty optional. */
struct nullopt_t {
  /**
   * @brief Construct the singleton tag through an internal integer token.
   */
  explicit nullopt_t(int) {}
};
/**
 * @brief Singleton empty-optional tag value.
 * @return Result described by the function brief.
 */
static const nullopt_t nullopt(0);

/** @brief Fixed-storage optional value for C++03 code. */
template <class T>
class optional {
public:
  /** @brief Construct an empty optional. */
  optional() : has_(false) {}
  /** @brief Construct an empty optional from the empty tag. */
  optional(nullopt_t) : has_(false) {}
  /**
   * @brief Construct an optional containing `v`.
   * @param v Generated object or variant instance.
   */
  optional(const T& v) : has_(false) { SSTL_CONSTRUCT_AT(storage_.ptr(0), v); has_ = true; }
  /**
   * @brief Copy-construct from another optional, preserving empty state.
   * @param o Caller-supplied argument used by this operation.
   */
  optional(const optional& o) : has_(false) { if (o.has_) { SSTL_CONSTRUCT_AT(storage_.ptr(0), *o); has_ = true; } }
  /** @brief Destroy any contained value. */
  ~optional() { reset(); }
  /**
   * @brief Assign a contained value, constructing storage when currently empty.
   * @param v Generated object or variant instance.
   * @return Result described by the function brief.
   */
  optional& operator=(const T& v) {
    if (has_) **this = v;
    else { SSTL_CONSTRUCT_AT(storage_.ptr(0), v); has_ = true; }
    return *this;
  }
  /**
   * @brief Assign from another optional, including resetting to empty.
   * @param o Caller-supplied argument used by this operation.
   * @return Result described by the function brief.
   */
  optional& operator=(const optional& o) {
    if (o.has_) *this = *o;
    else reset();
    return *this;
  }
  /**
   * @brief Reset this optional to the empty state.
   * @return Result described by the function brief.
   */
  optional& operator=(nullopt_t) { reset(); return *this; }
  /** @brief Destroy the contained value when present and mark the optional empty. */
  void reset() { if (has_) { SSTL_DESTROY_AT(storage_.ptr(0)); has_ = false; } }
  /**
   * @brief Report whether a value is currently present.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool has_value() const { return has_; }
  /** @brief Boolean conversion equivalent to `has_value()`. */
  operator bool() const { return has_; }
  /**
   * @brief Dereference the contained value; caller must ensure it is present.
   * @return Result described by the function brief.
   */
  T& operator*() { return *storage_.ptr(0); }
  /**
   * @brief Const-dereference the contained value; caller must ensure it is present.
   * @return Result described by the function brief.
   */
  const T& operator*() const { return *storage_.ptr(0); }
  /**
   * @brief Access the contained value pointer; caller must ensure it is present.
   * @return Pointer described by the function brief, or null for probe-style failure cases.
   */
  T* operator->() { return storage_.ptr(0); }
  /**
   * @brief Access the const contained value pointer; caller must ensure it is present.
   * @return Pointer described by the function brief, or null for probe-style failure cases.
   */
  const T* operator->() const { return storage_.ptr(0); }
  /**
   * @brief Return the contained value pointer, or null when empty.
   * @return The contained value pointer, or null when empty.
   */
  T* try_value() { return has_ ? storage_.ptr(0) : 0; }
  /**
   * @brief Return the const contained value pointer, or null when empty.
   * @return The const contained value pointer, or null when empty.
   */
  const T* try_value() const { return has_ ? storage_.ptr(0) : 0; }
  /**
   * @brief Return the contained value, following the active error policy when empty.
   * @return The contained value, following the active error policy when empty.
   */
  T& value() {
    if (!has_) handle_error("optional::value");
    return *storage_.ptr(0);
  }
  /**
   * @brief Return the const contained value, following the active error policy when empty.
   * @return The const contained value, following the active error policy when empty.
   */
  const T& value() const {
    if (!has_) handle_error("optional::value");
    return *storage_.ptr(0);
  }
  /**
   * @brief Return the contained value or a caller-provided fallback.
   * @param fallback Caller-supplied argument used by this operation.
   * @return The contained value or a caller-provided fallback.
   */
  T value_or(const T& fallback) const { return has_ ? *storage_.ptr(0) : fallback; }
  /**
   * @brief Replace the current state with a newly constructed value.
   * @param value Value supplied for comparison, assignment, insertion, or lookup.
   */
  void emplace(const T& value) { reset(); SSTL_CONSTRUCT_AT(storage_.ptr(0), value); has_ = true; }
private:
  /** @brief Inline storage used only when `has_` is true. */
  raw_storage<T, 1> storage_;
  /** @brief Presence flag indicating whether `storage_` currently contains a live `T`. */
  bool has_;
};

/**
 * @brief Compare an optional to the empty tag for equality.
 * @param a First operand or first range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T>
inline bool operator==(const optional<T>& a, nullopt_t) { return !a.has_value(); }
/**
 * @brief Compare an optional to the empty tag for inequality.
 * @param a First operand or first range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T>
inline bool operator!=(const optional<T>& a, nullopt_t) { return a.has_value(); }
/**
 * @brief Compare the empty tag to an optional for equality.
 * @param a First operand or first range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T>
inline bool operator==(nullopt_t, const optional<T>& a) { return !a.has_value(); }
/**
 * @brief Compare the empty tag to an optional for inequality.
 * @param a First operand or first range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T>
inline bool operator!=(nullopt_t, const optional<T>& a) { return a.has_value(); }
/**
 * @brief Compare two optionals, treating two empty values as equal.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T>
inline bool operator==(const optional<T>& a, const optional<T>& b) {
  if (a.has_value() != b.has_value()) return false;
  return !a.has_value() || *a == *b;
}
/**
 * @brief Compare two optionals for inequality.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T>
inline bool operator!=(const optional<T>& a, const optional<T>& b) { return !(a == b); }
/**
 * @brief Compare a present optional to a raw value.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T>
inline bool operator==(const optional<T>& a, const T& b) { return a.has_value() && *a == b; }
/**
 * @brief Compare an optional to a raw value for inequality.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <class T>
inline bool operator!=(const optional<T>& a, const T& b) { return !(a == b); }

} // namespace sstl

#endif

