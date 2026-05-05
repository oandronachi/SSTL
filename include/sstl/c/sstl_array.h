/**
 * @file sstl_array.h
 * @brief C99 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_C_ARRAY_H
/** @def SSTL_C_ARRAY_H
 * @brief Include guard for sstl_array.h.
 */
#define SSTL_C_ARRAY_H
#include "sstl_config.h"

/**
 * @brief Declare a typed fixed-size C array API.
 * @param NAME Public struct/type prefix to generate.
 * @param T Element type.
 * @param CAP Number of public elements.
 */
#define SSTL_ARRAY_DECLARE(NAME, T, CAP) \
  typedef struct NAME { T data[(CAP) == 0 ? 1 : (CAP)]; /**< @brief Inline element array; slot zero exists even when CAP is zero. */ } NAME; \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* a); \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_capacity(const NAME* a); \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* a); \
  /** \
   * @brief Run the generated data operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_data(NAME* a); \
  /** \
   * @brief Run the generated front operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_front(NAME* a); \
  /** \
   * @brief Run the generated back operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_back(NAME* a); \
  /** \
   * @brief Run the generated at operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param i Zero-based logical index. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_at(NAME* a, size_t i); \
  /** \
   * @brief Run the generated try at operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param i Zero-based logical index. \
   * @return Pointer to the requested object on success; null on failure. \
   */ \
  static SSTL_C_UNUSED T* NAME##_try_at(NAME* a, size_t i); \
  /** \
   * @brief Run the generated cmp operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @param cmp Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED int NAME##_cmp(const NAME* a, const NAME* b, sstl_cmp_fn cmp); \
  /** \
   * @brief Run the generated fill operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_fill(NAME* a, T value); \
  /** \
   * @brief Run the generated swap operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_swap(NAME* a, NAME* b);

/**
 * @brief Define the functions declared by `SSTL_ARRAY_DECLARE`.
 */
#define SSTL_ARRAY_DEFINE(NAME, T, CAP) \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* a) { (void)a; return (size_t)(CAP); } \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_capacity(const NAME* a) { (void)a; return (size_t)(CAP); } \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* a) { (void)a; return (size_t)(CAP) == 0u; } \
  /** \
   * @brief Run the generated data operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_data(NAME* a) { return a->data; } \
  /** \
   * @brief Run the generated front operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_front(NAME* a) { if ((size_t)(CAP) == 0u) { SSTL_C_PANIC("array front"); return 0; } return &a->data[0]; } \
  /** \
   * @brief Run the generated back operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_back(NAME* a) { if ((size_t)(CAP) == 0u) { SSTL_C_PANIC("array back"); return 0; } return &a->data[(size_t)(CAP) - 1u]; } \
  /** \
   * @brief Run the generated at operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param i Zero-based logical index. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  static SSTL_C_UNUSED T* NAME##_at(NAME* a, size_t i) { if (i >= (size_t)(CAP)) { SSTL_C_PANIC("array at"); return 0; } return &a->data[i]; } \
  /** \
   * @brief Run the generated try at operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param i Zero-based logical index. \
   * @return Pointer to the requested object on success; null on failure. \
   */ \
  static SSTL_C_UNUSED T* NAME##_try_at(NAME* a, size_t i) { return i < (size_t)(CAP) ? &a->data[i] : 0; } \
  /** \
   * @brief Run the generated cmp operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @param cmp Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED int NAME##_cmp(const NAME* a, const NAME* b, sstl_cmp_fn cmp) { size_t i; for (i = 0u; i != (size_t)(CAP); ++i) { int r = cmp((const void*)&a->data[i], (const void*)&b->data[i]); if (r != 0) return r; } return 0; } \
  /** \
   * @brief Run the generated fill operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param value Value supplied for comparison, assignment, insertion, or lookup. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_fill(NAME* a, T value) { size_t i; for (i = 0u; i != (size_t)(CAP); ++i) a->data[i] = value; } \
  /** \
   * @brief Run the generated swap operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_swap(NAME* a, NAME* b) { size_t i; for (i = 0u; i != (size_t)(CAP); ++i) { T tmp = a->data[i]; a->data[i] = b->data[i]; b->data[i] = tmp; } }

#endif

