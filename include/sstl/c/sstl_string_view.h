/**
 * @file sstl_string_view.h
 * @brief C99 SSTL public header for non-owning string views.
 *
 * The generated type stores only a pointer and a length. It never allocates,
 * never copies the viewed characters, and never calls hosted string library
 * functions. Callers remain responsible for keeping the referenced character
 * range alive for as long as the view is used.
 */
#ifndef SSTL_C_STRING_VIEW_H
/** @def SSTL_C_STRING_VIEW_H
 * @brief Include guard for sstl_string_view.h.
 */
#define SSTL_C_STRING_VIEW_H

#include "sstl_config.h"

/** @def SSTL_STRING_VIEW_NPOS
 * @brief Sentinel returned by generated find operations when no match exists.
 */
#define SSTL_STRING_VIEW_NPOS ((size_t)-1)

/**
 * @brief Declare a typed non-owning C string_view API.
 * @param NAME Public struct/type prefix to generate.
 *
 * The generated struct is intentionally tiny and contains no inline character
 * storage. It can view string literals, fixed-capacity C strings, or any other
 * caller-owned byte range that uses char-compatible storage.
 */
#define SSTL_STRING_VIEW_DECLARE(NAME) \
  typedef struct NAME { const char* data; /**< @brief Pointer to the first viewed character, or null for an empty/null view. */ size_t size; /**< @brief Number of characters visible through the view. */ } NAME; \
  /** \
   * @brief Create a view over `size` characters starting at `data`. \
   * @param data Caller-supplied argument used by this operation. \
   * @param size Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED NAME NAME##_make(const char* data, size_t size); \
  /** \
   * @brief Create a view over a null-terminated character sequence. \
   * @param text Null-terminated text argument. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED NAME NAME##_from_c_str(const char* text); \
  /** \
   * @brief Return the number of characters in the view. \
   * @param s String or set instance. \
   * @return The number of characters in the view. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* s); \
  /** \
   * @brief Report whether the view contains zero characters. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* s); \
  /** \
   * @brief Return the pointer stored by the view. \
   * @param s String or set instance. \
   * @return The pointer stored by the view. \
   */ \
  static SSTL_C_UNUSED const char* NAME##_data(const NAME* s); \
  /** \
   * @brief Return a pointer to character `i`, applying the active error policy when out of range. \
   * @param s String or set instance. \
   * @param i Zero-based logical index. \
   * @return A pointer to character `i`, applying the active error policy when out of range. \
   */ \
  static SSTL_C_UNUSED const char* NAME##_at(const NAME* s, size_t i); \
  /** \
   * @brief Return a pointer to character `i`, or null when `i` is out of range. \
   * @param s String or set instance. \
   * @param i Zero-based logical index. \
   * @return A pointer to character `i`, or null when `i` is out of range. \
   */ \
  static SSTL_C_UNUSED const char* NAME##_try_at(const NAME* s, size_t i); \
  /** \
   * @brief Return the first offset where `needle` appears, or `SSTL_STRING_VIEW_NPOS`. \
   * @param s String or set instance. \
   * @param needle Null-terminated search text. \
   * @return The first offset where `needle` appears, or `sstl_string_view_npos`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find(const NAME* s, const char* needle); \
  /** \
   * @brief Return the final offset where `needle` appears, or `SSTL_STRING_VIEW_NPOS`. \
   * @param s String or set instance. \
   * @param needle Null-terminated search text. \
   * @return The final offset where `needle` appears, or `sstl_string_view_npos`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_rfind(const NAME* s, const char* needle); \
  /** \
   * @brief Return the first offset containing any character from `chars`. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return The first offset containing any character from `chars`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_first_of(const NAME* s, const char* chars); \
  /** \
   * @brief Return the final offset containing any character from `chars`. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return The final offset containing any character from `chars`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_last_of(const NAME* s, const char* chars); \
  /** \
   * @brief Return the first offset containing no character from `chars`. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return The first offset containing no character from `chars`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_first_not_of(const NAME* s, const char* chars); \
  /** \
   * @brief Return the final offset containing no character from `chars`. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return The final offset containing no character from `chars`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_last_not_of(const NAME* s, const char* chars); \
  /** \
   * @brief Lexicographically compare the view with a null-terminated character sequence. \
   * @param s String or set instance. \
   * @param text Null-terminated text argument. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED int NAME##_compare(const NAME* s, const char* text); \
  /** \
   * @brief Report whether the view begins with `prefix`. \
   * @param s String or set instance. \
   * @param prefix Caller-supplied argument used by this operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_starts_with(const NAME* s, const char* prefix); \
  /** \
   * @brief Report whether the view ends with `suffix`. \
   * @param s String or set instance. \
   * @param suffix Caller-supplied argument used by this operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_ends_with(const NAME* s, const char* suffix); \
  /** \
   * @brief Drop up to `n` characters from the front of the view. \
   * @param s String or set instance. \
   * @param n Requested count or size. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_remove_prefix(NAME* s, size_t n); \
  /** \
   * @brief Drop up to `n` characters from the back of the view. \
   * @param s String or set instance. \
   * @param n Requested count or size. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_remove_suffix(NAME* s, size_t n);

/**
 * @brief Define the functions declared by `SSTL_STRING_VIEW_DECLARE`.
 * @param NAME Public struct/type prefix used in the declaration macro.
 */
#define SSTL_STRING_VIEW_DEFINE(NAME) \
  /** \
   * @brief Return the length of a nullable null-terminated character sequence. \
   * @param text Null-terminated text argument. \
   * @return The length of a nullable null-terminated character sequence. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_strlen_(const char* text) { size_t n = 0u; while (text && text[n]) ++n; return n; } \
  /** \
   * @brief Compare exactly `n` bytes as unsigned character values. \
   * @param a First operand or first range start. \
   * @param b Second operand or second range start. \
   * @param n Requested count or size. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED int NAME##_memcmp_(const char* a, const char* b, size_t n) { size_t i; for (i = 0u; i != n; ++i) { const unsigned char ca = (unsigned char)a[i]; const unsigned char cb = (unsigned char)b[i]; if (ca != cb) return ca < cb ? -1 : 1; } return 0; } \
  /** \
   * @brief Report whether `needle` of length `n` matches the view at `pos`. \
   * @param s String or set instance. \
   * @param pos Zero-based logical position. \
   * @param needle Null-terminated search text. \
   * @param n Requested count or size. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_match_(const NAME* s, size_t pos, const char* needle, size_t n) { return n == 0u || (s->data != 0 && needle != 0 && pos <= s->size && n <= s->size - pos && NAME##_memcmp_(s->data + pos, needle, n) == 0); } \
  /** \
   * @brief Report whether the nullable character set contains `c`. \
   * @param chars Null-terminated character set. \
   * @param c Character value. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_contains_char_(const char* chars, char c) { size_t i = 0u; while (chars && chars[i]) { if (chars[i] == c) return true; ++i; } return false; } \
  /** \
   * @brief Create a view over `size` characters starting at `data`. \
   * @param data Caller-supplied argument used by this operation. \
   * @param size Caller-supplied argument used by this operation. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED NAME NAME##_make(const char* data, size_t size) { NAME s; s.data = data; s.size = size; return s; } \
  /** \
   * @brief Create a view over a null-terminated character sequence. \
   * @param text Null-terminated text argument. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED NAME NAME##_from_c_str(const char* text) { NAME s; s.data = text; s.size = NAME##_strlen_(text); return s; } \
  /** \
   * @brief Return the number of characters in the view. \
   * @param s String or set instance. \
   * @return The number of characters in the view. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_size(const NAME* s) { return s->size; } \
  /** \
   * @brief Report whether the view contains zero characters. \
   * @param s String or set instance. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_empty(const NAME* s) { return s->size == 0u; } \
  /** \
   * @brief Return the pointer stored by the view. \
   * @param s String or set instance. \
   * @return The pointer stored by the view. \
   */ \
  static SSTL_C_UNUSED const char* NAME##_data(const NAME* s) { return s->data; } \
  /** \
   * @brief Return a pointer to character `i`, applying the active error policy when out of range. \
   * @param s String or set instance. \
   * @param i Zero-based logical index. \
   * @return A pointer to character `i`, applying the active error policy when out of range. \
   */ \
  static SSTL_C_UNUSED const char* NAME##_at(const NAME* s, size_t i) { if (i >= s->size || s->data == 0) { SSTL_C_PANIC("string_view at"); return 0; } return s->data + i; } \
  /** \
   * @brief Return a pointer to character `i`, or null when `i` is out of range. \
   * @param s String or set instance. \
   * @param i Zero-based logical index. \
   * @return A pointer to character `i`, or null when `i` is out of range. \
   */ \
  static SSTL_C_UNUSED const char* NAME##_try_at(const NAME* s, size_t i) { return (i < s->size && s->data != 0) ? s->data + i : 0; } \
  /** \
   * @brief Return the first offset where `needle` appears, or `SSTL_STRING_VIEW_NPOS`. \
   * @param s String or set instance. \
   * @param needle Null-terminated search text. \
   * @return The first offset where `needle` appears, or `sstl_string_view_npos`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find(const NAME* s, const char* needle) { size_t i; const size_t n = NAME##_strlen_(needle); if (n == 0u) return 0u; if (s->data == 0 || needle == 0 || n > s->size) return SSTL_STRING_VIEW_NPOS; for (i = 0u; i <= s->size - n; ++i) if (NAME##_match_(s, i, needle, n)) return i; return SSTL_STRING_VIEW_NPOS; } \
  /** \
   * @brief Return the final offset where `needle` appears, or `SSTL_STRING_VIEW_NPOS`. \
   * @param s String or set instance. \
   * @param needle Null-terminated search text. \
   * @return The final offset where `needle` appears, or `sstl_string_view_npos`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_rfind(const NAME* s, const char* needle) { size_t pos; const size_t n = NAME##_strlen_(needle); if (n == 0u) return s->size; if (s->data == 0 || needle == 0 || n > s->size) return SSTL_STRING_VIEW_NPOS; pos = s->size - n; for (;;) { if (NAME##_match_(s, pos, needle, n)) return pos; if (pos == 0u) break; --pos; } return SSTL_STRING_VIEW_NPOS; } \
  /** \
   * @brief Return the first offset containing any character from `chars`. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return The first offset containing any character from `chars`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_first_of(const NAME* s, const char* chars) { size_t i; if (s->data == 0 || chars == 0) return SSTL_STRING_VIEW_NPOS; for (i = 0u; i != s->size; ++i) if (NAME##_contains_char_(chars, s->data[i])) return i; return SSTL_STRING_VIEW_NPOS; } \
  /** \
   * @brief Return the final offset containing any character from `chars`. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return The final offset containing any character from `chars`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_last_of(const NAME* s, const char* chars) { size_t pos; if (s->data == 0 || s->size == 0u || chars == 0) return SSTL_STRING_VIEW_NPOS; pos = s->size; while (pos != 0u) { --pos; if (NAME##_contains_char_(chars, s->data[pos])) return pos; } return SSTL_STRING_VIEW_NPOS; } \
  /** \
   * @brief Return the first offset containing no character from `chars`. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return The first offset containing no character from `chars`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_first_not_of(const NAME* s, const char* chars) { size_t i; if (s->data == 0 || s->size == 0u) return SSTL_STRING_VIEW_NPOS; if (chars == 0) return 0u; for (i = 0u; i != s->size; ++i) if (!NAME##_contains_char_(chars, s->data[i])) return i; return SSTL_STRING_VIEW_NPOS; } \
  /** \
   * @brief Return the final offset containing no character from `chars`. \
   * @param s String or set instance. \
   * @param chars Null-terminated character set. \
   * @return The final offset containing no character from `chars`. \
   */ \
  static SSTL_C_UNUSED size_t NAME##_find_last_not_of(const NAME* s, const char* chars) { size_t pos; if (s->data == 0 || s->size == 0u) return SSTL_STRING_VIEW_NPOS; if (chars == 0) return s->size - 1u; pos = s->size; while (pos != 0u) { --pos; if (!NAME##_contains_char_(chars, s->data[pos])) return pos; } return SSTL_STRING_VIEW_NPOS; } \
  /** \
   * @brief Lexicographically compare the view with a null-terminated character sequence. \
   * @param s String or set instance. \
   * @param text Null-terminated text argument. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED int NAME##_compare(const NAME* s, const char* text) { const size_t n = NAME##_strlen_(text); const size_t common = s->size < n ? s->size : n; int cmp = 0; if (common != 0u && s->data != 0 && text != 0) cmp = NAME##_memcmp_(s->data, text, common); else if (common != 0u) cmp = s->data == 0 ? -1 : 1; if (cmp != 0) return cmp; return s->size < n ? -1 : (s->size > n ? 1 : 0); } \
  /** \
   * @brief Report whether the view begins with `prefix`. \
   * @param s String or set instance. \
   * @param prefix Caller-supplied argument used by this operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_starts_with(const NAME* s, const char* prefix) { const size_t n = NAME##_strlen_(prefix); return n <= s->size && NAME##_match_(s, 0u, prefix, n); } \
  /** \
   * @brief Report whether the view ends with `suffix`. \
   * @param s String or set instance. \
   * @param suffix Caller-supplied argument used by this operation. \
   * @return `true` when the documented condition holds; otherwise `false`. \
   */ \
  static SSTL_C_UNUSED bool NAME##_ends_with(const NAME* s, const char* suffix) { const size_t n = NAME##_strlen_(suffix); return n <= s->size && NAME##_match_(s, s->size - n, suffix, n); } \
  /** \
   * @brief Drop up to `n` characters from the front of the view. \
   * @param s String or set instance. \
   * @param n Requested count or size. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_remove_prefix(NAME* s, size_t n) { if (n > s->size) { SSTL_C_PANIC("string_view remove_prefix"); n = s->size; } if (s->data != 0) s->data += n; s->size -= n; } \
  /** \
   * @brief Drop up to `n` characters from the back of the view. \
   * @param s String or set instance. \
   * @param n Requested count or size. \
   * @return Result described by the function brief. \
   */ \
  static SSTL_C_UNUSED void NAME##_remove_suffix(NAME* s, size_t n) { if (n > s->size) { SSTL_C_PANIC("string_view remove_suffix"); n = s->size; } s->size -= n; }

#endif

