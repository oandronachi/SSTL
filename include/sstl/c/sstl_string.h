/**
 * @file sstl_string.h
 * @brief C99 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_C_STRING_H
/** @def SSTL_C_STRING_H
 * @brief Include guard for sstl_string.h.
 */
#define SSTL_C_STRING_H

#include "sstl_config.h"

/** @def SSTL_STRING_NPOS
 * @brief Sentinel returned by generated fixed-string find operations when no match exists.
 */
#define SSTL_STRING_NPOS ((size_t)-1)

/**
 * @brief Declare a typed fixed-capacity C string API.
 * @param NAME Public struct/type prefix to generate.
 * @param CAP Maximum number of non-null characters.
 *
 * The generated struct stores `CAP + 1` bytes so the string remains
 * null-terminated after initialization, successful appends, and failed appends.
 */
#define SSTL_STRING_DECLARE(NAME, CAP) \
  typedef struct NAME { char data[((CAP) + 1) == 0 ? 1 : ((CAP) + 1)]; /**< @brief Inline character buffer including the trailing null byte. */ size_t size; /**< @brief Number of non-null characters currently stored. */ } NAME; \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* s); \
  /** \
   * @brief Run the generated clear operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_clear(NAME* s); \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* s); \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* s); \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* s); \
  /** \
   * @brief Run the generated full operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* s); \
  /** \
   * @brief Run the generated push back operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param c Character value. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_push_back(NAME* s, char c); \
  /** \
   * @brief Run the generated try push back operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param c Character value. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_push_back(NAME* s, char c); \
  /** \
   * @brief Run the generated pop back operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_pop_back(NAME* s); \
  /** \
   * @brief Try to remove the last character without invoking the active error policy. \
   * @param s String or set instance. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_pop_back(NAME* s); \
  /** \
   * @brief Run the generated resize operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param n Requested count or size. \
   * @param fill Value used for newly created slots. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_resize(NAME* s, size_t n, char fill); \
  /** \
   * @brief Run the generated try resize operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param n Requested count or size. \
   * @param fill Value used for newly created slots. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_resize(NAME* s, size_t n, char fill); \
  /** \
   * @brief Run the generated swap operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b); \
  /** \
   * @brief Run the generated append n operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @param n Requested count or size. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_append_n(NAME* s, const char* src, size_t n); \
  /** \
   * @brief Run the generated try append n operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @param n Requested count or size. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_append_n(NAME* s, const char* src, size_t n); \
  /** \
   * @brief Run the generated assign operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_assign(NAME* s, const char* src); \
  /** \
   * @brief Run the generated try assign operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_assign(NAME* s, const char* src); \
  /** \
   * @brief Run the generated assign n operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @param n Requested count or size. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_assign_n(NAME* s, const char* src, size_t n); \
  /** \
   * @brief Run the generated try assign n operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @param n Requested count or size. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_assign_n(NAME* s, const char* src, size_t n); \
  /** \
   * @brief Run the generated insert operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param src Caller-provided source data. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert(NAME* s, size_t pos, const char* src); \
  /** \
   * @brief Run the generated try insert operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param src Caller-provided source data. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_insert(NAME* s, size_t pos, const char* src); \
  /** \
   * @brief Run the generated insert char operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param c Character value. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert_char(NAME* s, size_t pos, char c); \
  /** \
   * @brief Run the generated try insert char operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param c Character value. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_insert_char(NAME* s, size_t pos, char c); \
  /** \
   * @brief Run the generated erase operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param count Requested element or character count. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_erase(NAME* s, size_t pos, size_t count); \
  /** \
   * @brief Run the generated replace operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param count Requested element or character count. \
   * @param src Caller-provided source data. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_replace(NAME* s, size_t pos, size_t count, const char* src); \
  /** \
   * @brief Run the generated try replace operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param count Requested element or character count. \
   * @param src Caller-provided source data. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_replace(NAME* s, size_t pos, size_t count, const char* src); \
  /** \
   * @brief Run the generated compare operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param text Null-terminated text argument. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE int NAME##_compare(const NAME* s, const char* text); \
  /** \
   * @brief Run the generated find operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param needle Null-terminated search text. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find(const NAME* s, const char* needle); \
  /** \
   * @brief Run the generated rfind operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param needle Null-terminated search text. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_rfind(const NAME* s, const char* needle); \
  /** \
   * @brief Run the generated find first of operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_first_of(const NAME* s, const char* chars); \
  /** \
   * @brief Run the generated find last of operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_last_of(const NAME* s, const char* chars); \
  /** \
   * @brief Run the generated find first not of operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_first_not_of(const NAME* s, const char* chars); \
  /** \
   * @brief Run the generated find last not of operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_last_not_of(const NAME* s, const char* chars); \
  /** \
   * @brief Run the generated substr operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param count Requested element or character count. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE NAME NAME##_substr(const NAME* s, size_t pos, size_t count); \
  /** \
   * @brief Run the generated at operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param i Zero-based logical index. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE char* NAME##_at(NAME* s, size_t i); \
  /** \
   * @brief Run the generated try at operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param i Zero-based logical index. \
   * @return Pointer to the requested object on success; null on failure. \
   */ \
  SSTL_C_INLINE char* NAME##_try_at(NAME* s, size_t i); \
  /** \
   * @brief Run the generated c str operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE const char* NAME##_c_str(const NAME* s);

/**
 * @brief Define the functions declared by `SSTL_STRING_DECLARE`.
 * @param NAME Public struct/type prefix used in the declaration macro.
 * @param CAP Maximum number of non-null characters.
 *
 * `NAME_push_back` preserves the trailing null byte in every policy mode.
 */
#define SSTL_STRING_DEFINE(NAME, CAP) \
  /** \
   * @brief Return the length of a nullable null-terminated character sequence. \
   * @param text Null-terminated text argument. \
   * @return The length of a nullable null-terminated character sequence. \
   */ \
  SSTL_C_INLINE size_t NAME##_strlen_(const char* text) { size_t n = 0u; while (text && text[n]) ++n; return n; } \
  /** \
   * @brief Compare exactly `n` bytes as unsigned character values. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @param n Requested count or size. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE int NAME##_memcmp_(const char* a, const char* b, size_t n) { size_t i; for (i = 0u; i != n; ++i) { const unsigned char ca = (unsigned char)a[i]; const unsigned char cb = (unsigned char)b[i]; if (ca != cb) return ca < cb ? -1 : 1; } return 0; } \
  /** \
   * @brief Report whether `needle` of length `n` matches the string at `pos`. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param needle Null-terminated search text. \
   * @param n Requested count or size. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_match_(const NAME* s, size_t pos, const char* needle, size_t n) { return n == 0u || (needle != 0 && pos <= s->size && n <= s->size - pos && NAME##_memcmp_(s->data + pos, needle, n) == 0); } \
  /** \
   * @brief Report whether the nullable character set contains `c`. \
   * @param chars Null-terminated character set. \
   * @param c Character value. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_contains_char_(const char* chars, char c) { size_t i = 0u; while (chars && chars[i]) { if (chars[i] == c) return true; ++i; } return false; } \
  /** \
   * @brief Run the generated init operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_init(NAME* s) { s->size = 0u; s->data[0] = 0; } \
  /** \
   * @brief Run the generated clear operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_clear(NAME* s) { s->size = 0u; s->data[0] = 0; } \
  /** \
   * @brief Run the generated size operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_size(const NAME* s) { return s->size; } \
  /** \
   * @brief Run the generated capacity operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_capacity(const NAME* s) { (void)s; return (size_t)(CAP); } \
  /** \
   * @brief Run the generated empty operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_empty(const NAME* s) { return s->size == 0u; } \
  /** \
   * @brief Run the generated full operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_full(const NAME* s) { return s->size == (size_t)(CAP); } \
  /** \
   * @brief Run the generated push back operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param c Character value. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_push_back(NAME* s, char c) { if (s->size == (size_t)(CAP)) { s->data[s->size] = 0; SSTL_C_PANIC("string full"); return false; } s->data[s->size++] = c; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated try push back operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param c Character value. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_push_back(NAME* s, char c) { if (s->size == (size_t)(CAP)) return false; s->data[s->size++] = c; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated pop back operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_pop_back(NAME* s) { if (s->size == 0u) { SSTL_C_PANIC("string pop_back"); return false; } --s->size; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Try to remove the last character without invoking the active error policy. \
   * @param s String or set instance. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_pop_back(NAME* s) { if (s->size == 0u) return false; --s->size; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated resize operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param n Requested count or size. \
   * @param fill Value used for newly created slots. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_resize(NAME* s, size_t n, char fill) { if (n > (size_t)(CAP)) { SSTL_C_PANIC("string resize"); return false; } while (s->size < n) s->data[s->size++] = fill; s->size = n; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated try resize operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param n Requested count or size. \
   * @param fill Value used for newly created slots. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_resize(NAME* s, size_t n, char fill) { if (n > (size_t)(CAP)) return false; while (s->size < n) s->data[s->size++] = fill; s->size = n; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated swap operation for this typed SSTL family. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE void NAME##_swap(NAME* a, NAME* b) { size_t i; size_t limit = a->size > b->size ? a->size : b->size; for (i = 0u; i <= limit; ++i) { char tmp = a->data[i]; a->data[i] = b->data[i]; b->data[i] = tmp; } i = a->size; a->size = b->size; b->size = i; } \
  /** \
   * @brief Run the generated append n operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @param n Requested count or size. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_append_n(NAME* s, const char* src, size_t n) { size_t i; if (n > (size_t)(CAP) - s->size) { SSTL_C_PANIC("string append"); return false; } for (i = 0u; i != n; ++i) s->data[s->size++] = src[i]; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated try append n operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @param n Requested count or size. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_append_n(NAME* s, const char* src, size_t n) { size_t i; if (n > (size_t)(CAP) - s->size) return false; for (i = 0u; i != n; ++i) s->data[s->size++] = src[i]; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated assign n operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @param n Requested count or size. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_assign_n(NAME* s, const char* src, size_t n) { if (n > (size_t)(CAP)) { SSTL_C_PANIC("string assign"); return false; } s->size = 0u; s->data[0] = 0; return NAME##_append_n(s, src, n); } \
  /** \
   * @brief Run the generated try assign n operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @param n Requested count or size. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_assign_n(NAME* s, const char* src, size_t n) { if (n > (size_t)(CAP)) return false; s->size = 0u; s->data[0] = 0; return NAME##_try_append_n(s, src, n); } \
  /** \
   * @brief Run the generated assign operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_assign(NAME* s, const char* src) { size_t n = 0u; while (src && src[n]) ++n; return NAME##_assign_n(s, src, n); } \
  /** \
   * @brief Run the generated try assign operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param src Caller-provided source data. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_assign(NAME* s, const char* src) { size_t n = 0u; while (src && src[n]) ++n; return NAME##_try_assign_n(s, src, n); } \
  /** \
   * @brief Run the generated insert char operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param c Character value. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert_char(NAME* s, size_t pos, char c) { size_t i; if (pos > s->size || s->size == (size_t)(CAP)) { SSTL_C_PANIC("string insert"); return false; } for (i = s->size; i != pos; --i) s->data[i] = s->data[i - 1u]; s->data[pos] = c; ++s->size; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated try insert char operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param c Character value. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_insert_char(NAME* s, size_t pos, char c) { size_t i; if (pos > s->size || s->size == (size_t)(CAP)) return false; for (i = s->size; i != pos; --i) s->data[i] = s->data[i - 1u]; s->data[pos] = c; ++s->size; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated insert operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param src Caller-provided source data. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_insert(NAME* s, size_t pos, const char* src) { size_t n = 0u; size_t i; while (src && src[n]) ++n; if (pos > s->size || n > (size_t)(CAP) - s->size) { SSTL_C_PANIC("string insert"); return false; } if (n == 0u) return true; for (i = s->size; i != pos; --i) s->data[i + n - 1u] = s->data[i - 1u]; for (i = 0u; i != n; ++i) s->data[pos + i] = src[i]; s->size += n; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated try insert operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param src Caller-provided source data. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_insert(NAME* s, size_t pos, const char* src) { size_t n = 0u; size_t i; while (src && src[n]) ++n; if (pos > s->size || n > (size_t)(CAP) - s->size) return false; if (n == 0u) return true; for (i = s->size; i != pos; --i) s->data[i + n - 1u] = s->data[i - 1u]; for (i = 0u; i != n; ++i) s->data[pos + i] = src[i]; s->size += n; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated erase operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param count Requested element or character count. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_erase(NAME* s, size_t pos, size_t count) { size_t i; if (pos > s->size) { SSTL_C_PANIC("string erase"); return false; } if (count > s->size - pos) count = s->size - pos; for (i = pos + count; i <= s->size; ++i) s->data[i - count] = s->data[i]; s->size -= count; return true; } \
  /** \
   * @brief Run the generated replace operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param count Requested element or character count. \
   * @param src Caller-provided source data. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  SSTL_C_INLINE bool NAME##_replace(NAME* s, size_t pos, size_t count, const char* src) { size_t n = 0u; size_t i; while (src && src[n]) ++n; if (pos > s->size) { SSTL_C_PANIC("string replace"); return false; } if (count > s->size - pos) count = s->size - pos; if (n > (size_t)(CAP) - (s->size - count)) { SSTL_C_PANIC("string replace"); return false; } if (n < count) { for (i = pos + count; i <= s->size; ++i) s->data[i - (count - n)] = s->data[i]; } else if (n > count) { for (i = s->size + 1u; i != pos + count; --i) s->data[i + (n - count) - 1u] = s->data[i - 1u]; } for (i = 0u; i != n; ++i) s->data[pos + i] = src[i]; s->size = s->size - count + n; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated try replace operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param count Requested element or character count. \
   * @param src Caller-provided source data. \
   * @return `true` on success; otherwise `false` without invoking the panic policy. \
   */ \
  SSTL_C_INLINE bool NAME##_try_replace(NAME* s, size_t pos, size_t count, const char* src) { size_t n = 0u; size_t i; while (src && src[n]) ++n; if (pos > s->size) return false; if (count > s->size - pos) count = s->size - pos; if (n > (size_t)(CAP) - (s->size - count)) return false; if (n < count) { for (i = pos + count; i <= s->size; ++i) s->data[i - (count - n)] = s->data[i]; } else if (n > count) { for (i = s->size + 1u; i != pos + count; --i) s->data[i + (n - count) - 1u] = s->data[i - 1u]; } for (i = 0u; i != n; ++i) s->data[pos + i] = src[i]; s->size = s->size - count + n; s->data[s->size] = 0; return true; } \
  /** \
   * @brief Run the generated compare operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param text Null-terminated text argument. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE int NAME##_compare(const NAME* s, const char* text) { const size_t n = NAME##_strlen_(text); const size_t common = s->size < n ? s->size : n; int cmp = common == 0u ? 0 : NAME##_memcmp_(s->data, text, common); if (cmp != 0) return cmp; return s->size < n ? -1 : (s->size > n ? 1 : 0); } \
  /** \
   * @brief Run the generated find operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param needle Null-terminated search text. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find(const NAME* s, const char* needle) { size_t i; const size_t n = NAME##_strlen_(needle); if (n == 0u) return 0u; if (needle == 0 || n > s->size) return SSTL_STRING_NPOS; for (i = 0u; i <= s->size - n; ++i) if (NAME##_match_(s, i, needle, n)) return i; return SSTL_STRING_NPOS; } \
  /** \
   * @brief Run the generated rfind operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param needle Null-terminated search text. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_rfind(const NAME* s, const char* needle) { size_t pos; const size_t n = NAME##_strlen_(needle); if (n == 0u) return s->size; if (needle == 0 || n > s->size) return SSTL_STRING_NPOS; pos = s->size - n; for (;;) { if (NAME##_match_(s, pos, needle, n)) return pos; if (pos == 0u) break; --pos; } return SSTL_STRING_NPOS; } \
  /** \
   * @brief Run the generated find first of operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_first_of(const NAME* s, const char* chars) { size_t i; if (chars == 0) return SSTL_STRING_NPOS; for (i = 0u; i != s->size; ++i) if (NAME##_contains_char_(chars, s->data[i])) return i; return SSTL_STRING_NPOS; } \
  /** \
   * @brief Run the generated find last of operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_last_of(const NAME* s, const char* chars) { size_t pos; if (s->size == 0u || chars == 0) return SSTL_STRING_NPOS; pos = s->size; while (pos != 0u) { --pos; if (NAME##_contains_char_(chars, s->data[pos])) return pos; } return SSTL_STRING_NPOS; } \
  /** \
   * @brief Run the generated find first not of operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_first_not_of(const NAME* s, const char* chars) { size_t i; if (s->size == 0u) return SSTL_STRING_NPOS; if (chars == 0) return 0u; for (i = 0u; i != s->size; ++i) if (!NAME##_contains_char_(chars, s->data[i])) return i; return SSTL_STRING_NPOS; } \
  /** \
   * @brief Run the generated find last not of operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE size_t NAME##_find_last_not_of(const NAME* s, const char* chars) { size_t pos; if (s->size == 0u) return SSTL_STRING_NPOS; if (chars == 0) return s->size - 1u; pos = s->size; while (pos != 0u) { --pos; if (!NAME##_contains_char_(chars, s->data[pos])) return pos; } return SSTL_STRING_NPOS; } \
  /** \
   * @brief Run the generated substr operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param count Requested element or character count. \
   * @return Result described by the function brief. \
   */ \
  SSTL_C_INLINE NAME NAME##_substr(const NAME* s, size_t pos, size_t count) { NAME out; NAME##_init(&out); if (pos > s->size) { SSTL_C_PANIC("string substr"); return out; } if (count > s->size - pos) count = s->size - pos; (void)NAME##_append_n(&out, s->data + pos, count); return out; } \
  /** \
   * @brief Run the generated at operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param i Zero-based logical index. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE char* NAME##_at(NAME* s, size_t i) { if (i >= s->size) { SSTL_C_PANIC("string at"); return 0; } return &s->data[i]; } \
  /** \
   * @brief Run the generated try at operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @param i Zero-based logical index. \
   * @return Pointer to the requested object on success; null on failure. \
   */ \
  SSTL_C_INLINE char* NAME##_try_at(NAME* s, size_t i) { return i < s->size ? &s->data[i] : 0; } \
  /** \
   * @brief Run the generated c str operation for this typed SSTL family. \
   * @param s String or set instance. \
   * @return Pointer described by the function brief, or null for probe-style failure cases. \
   */ \
  SSTL_C_INLINE const char* NAME##_c_str(const NAME* s) { return s->data; }

#endif

