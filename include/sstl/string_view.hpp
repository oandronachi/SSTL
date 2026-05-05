/**
 * @file string_view.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_STRING_VIEW_HPP
/** @def SSTL_STRING_VIEW_HPP
 * @brief Include guard for string_view.hpp.
 */
#define SSTL_STRING_VIEW_HPP

#include "config.hpp"
#include "iterator.hpp"
#include "string.hpp"

namespace sstl {

/** @brief Non-owning view over a contiguous character sequence. */
class string_view {
public:
  /** @brief Character type exposed by the view. */
  typedef char value_type;
  /** @brief Unsigned size type used for view lengths and indexes. */
  typedef size_t size_type;
  /** @brief Const iterator type; string_view never exposes mutable characters. */
  typedef contiguous_iterator<const char> const_iterator;
  /** @brief Iterator alias matching the immutable `const_iterator` type. */
  typedef const_iterator iterator;

  /** @brief Construct an empty view over a static empty string. */
  string_view() : ptr_(""), size_(0) {}
  /**
   * @brief Construct a view from pointer and explicit length.
   * @param p Erased storage pointer.
   * @param n Requested count or size.
   */
  string_view(const char* p, size_type n) : ptr_(p), size_(n) {}
  /**
   * @brief Construct a view over a null-terminated string.
   * @param p Erased storage pointer.
   */
  string_view(const char* p) : ptr_(p), size_(cstrlen(p)) {}

  /**
   * @brief Return the number of viewed characters.
   * @return The number of viewed characters.
   */
  size_type size() const { return size_; }
  /**
   * @brief Report whether the view contains no characters.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return size_ == 0; }
  /**
   * @brief Return the pointer to the first viewed character.
   * @return The pointer to the first viewed character.
   */
  const char* data() const { return ptr_; }
  /**
   * @brief Access character `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  char operator[](size_type i) const { return ptr_[i]; }
  /**
   * @brief Access character `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const char& at(size_type i) const {
    if (i >= size_) handle_error("string_view::at"); // LCOV_EXCL_BR_LINE
    return ptr_[i];
  }
  /**
   * @brief Access the first viewed character, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  const char& front() const {
    if (empty()) handle_error("string_view::front"); // LCOV_EXCL_BR_LINE
    return ptr_[0];
  }
  /**
   * @brief Access the final viewed character, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  const char& back() const {
    if (empty()) handle_error("string_view::back"); // LCOV_EXCL_BR_LINE
    return ptr_[size_ - 1u];
  }
  /**
   * @brief Return an iterator to the first viewed character.
   * @return An iterator to the first viewed character.
   */
  const_iterator begin() const { return const_iterator(ptr_); }
  /**
   * @brief Return a const iterator to the first viewed character.
   * @return A const iterator to the first viewed character.
   */
  const_iterator cbegin() const { return const_iterator(ptr_); }
  /**
   * @brief Return an iterator one past the final viewed character.
   * @return An iterator one past the final viewed character.
   */
  const_iterator end() const { return const_iterator(ptr_ + size_); }
  /**
   * @brief Return a const iterator one past the final viewed character.
   * @return A const iterator one past the final viewed character.
   */
  const_iterator cend() const { return const_iterator(ptr_ + size_); }

  /**
   * @brief Find the first occurrence of `needle` in the viewed range at or after `pos`.
   * @param needle Null-terminated search text.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find(const char* needle, size_type pos = 0) const {
    if (!needle || pos > size_) return npos;
    size_type nl = cstrlen(needle);
    if (nl == 0u) return pos;
    for (size_type i = pos; i + nl <= size_; ++i) {
      if (cmemcmp(ptr_ + i, needle, nl) == 0) return i;
    }
    return npos;
  }
  /**
   * @brief Find the first occurrence of character `c` at or after `pos`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find(char c, size_type pos = 0) const {
    if (pos >= size_) return npos;
    for (size_type i = pos; i != size_; ++i) if (ptr_[i] == c) return i;
    return npos;
  }
  /**
   * @brief Preserve C++03 null-pointer-literal calls such as `find(0)`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find(int c, size_type pos = 0) const {
    return c == 0 ? npos : find(static_cast<char>(c), pos);
  }
  /**
   * @brief Find the first character at or after `pos` that belongs to `chars`.
   * @param chars Null-terminated character set.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_first_of(const char* chars, size_type pos = 0) const {
    if (!chars || pos >= size_) return npos;
    for (size_type i = pos; i != size_; ++i) if (char_in_set(ptr_[i], chars)) return i;
    return npos;
  }
  /**
   * @brief Find the first occurrence of character `c` at or after `pos`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_first_of(char c, size_type pos = 0) const {
    if (pos >= size_) return npos;
    for (size_type i = pos; i != size_; ++i) if (ptr_[i] == c) return i;
    return npos;
  }
  /**
   * @brief Preserve C++03 null-pointer-literal calls such as `find_first_of(0)`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_first_of(int c, size_type pos = 0) const {
    return c == 0 ? npos : find_first_of(static_cast<char>(c), pos);
  }
  /**
   * @brief Find the first character at or after `pos` that does not belong to `chars`.
   * @param chars Null-terminated character set.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_first_not_of(const char* chars, size_type pos = 0) const {
    if (!chars || pos >= size_) return npos;
    for (size_type i = pos; i != size_; ++i) if (!char_in_set(ptr_[i], chars)) return i;
    return npos;
  }
  /**
   * @brief Find the first character at or after `pos` that is not `c`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_first_not_of(char c, size_type pos = 0) const {
    if (pos >= size_) return npos;
    for (size_type i = pos; i != size_; ++i) if (ptr_[i] != c) return i;
    return npos;
  }
  /**
   * @brief Preserve C++03 null-pointer-literal calls such as `find_first_not_of(0)`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_first_not_of(int c, size_type pos = 0) const {
    return c == 0 ? npos : find_first_not_of(static_cast<char>(c), pos);
  }
  /**
   * @brief Find the last occurrence of `needle` in the viewed range at or before `pos`.
   * @param needle Null-terminated search text.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type rfind(const char* needle, size_type pos = npos) const {
    if (!needle) return npos;
    size_type nl = cstrlen(needle);
    if (nl == 0u) return pos == npos || pos > size_ ? size_ : pos;
    if (nl > size_) return npos;
    size_type start = size_ - nl;
    if (pos != npos && pos < start) start = pos;
    for (size_type i = start + 1u; i != 0; --i) {
      size_type candidate = i - 1;
      if (cmemcmp(ptr_ + candidate, needle, nl) == 0) return candidate;
    }
    return npos;
  }
  /**
   * @brief Find the last character at or before `pos` that belongs to `chars`.
   * @param chars Null-terminated character set.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_last_of(const char* chars, size_type pos = npos) const {
    if (!chars || size_ == 0u) return npos;
    unsigned i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (char_in_set(ptr_[i], chars)) return i;
      if (i == 0u) break;
      --i;
    }
    return npos;
  }
  /**
   * @brief Find the last occurrence of character `c` at or before `pos`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_last_of(char c, size_type pos = npos) const {
    if (size_ == 0u) return npos;
    unsigned i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (ptr_[i] == c) return i;
      if (i == 0u) break;
      --i;
    }
    return npos;
  }
  /**
   * @brief Preserve C++03 null-pointer-literal calls such as `find_last_of(0)`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_last_of(int c, size_type pos = npos) const {
    return c == 0 ? npos : find_last_of(static_cast<char>(c), pos);
  }
  /**
   * @brief Find the last character at or before `pos` that does not belong to `chars`.
   * @param chars Null-terminated character set.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_last_not_of(const char* chars, size_type pos = npos) const {
    if (!chars || size_ == 0u) return npos;
    unsigned i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (!char_in_set(ptr_[i], chars)) return i;
      if (i == 0u) break;
      --i;
    }
    return npos;
  }
  /**
   * @brief Find the last character at or before `pos` that is not `c`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_last_not_of(char c, size_type pos = npos) const {
    if (size_ == 0u) return npos;
    unsigned i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (ptr_[i] != c) return i;
      if (i == 0u) break;
      --i;
    }
    return npos;
  }
  /**
   * @brief Preserve C++03 null-pointer-literal calls such as `find_last_not_of(0)`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_last_not_of(int c, size_type pos = npos) const {
    return c == 0 ? npos : find_last_not_of(static_cast<char>(c), pos);
  }
  /**
   * @brief Lexicographically compare the view to a null-terminated string.
   * @param s String or set instance.
   * @return Result described by the function brief.
   */
  int compare(const char* s) const {
    size_type sl = cstrlen(s);
    size_type n = size_ < sl ? size_ : sl;
    int r = cmemcmp(ptr_, s, n);
    if (r != 0) return r;
    return size_ == sl ? 0 : (size_ < sl ? -1 : 1);
  }
  /**
   * @brief Return true when the view begins with the supplied null-terminated prefix.
   * @param p Erased storage pointer.
   * @return True when the view begins with the supplied null-terminated prefix.
   */
  bool starts_with(const char* p) const {
    size_type n = cstrlen(p);
    return n <= size_ && cmemcmp(ptr_, p, n) == 0;
  }
  /**
   * @brief Return true when the view ends with the supplied null-terminated suffix.
   * @param p Erased storage pointer.
   * @return True when the view ends with the supplied null-terminated suffix.
   */
  bool ends_with(const char* p) const {
    size_type n = cstrlen(p);
    return n <= size_ && cmemcmp(ptr_ + size_ - n, p, n) == 0;
  }
  /**
   * @brief Drop up to `n` characters from the front of the view.
   * @param n Requested count or size.
   */
  void remove_prefix(size_type n) {
    if (n > size_) n = size_;
    ptr_ += n;
    size_ -= n;
  }
  /**
   * @brief Drop up to `n` characters from the back of the view.
   * @param n Requested count or size.
   */
  void remove_suffix(size_type n) {
    if (n > size_) n = size_;
    size_ -= n;
  }

private:
  /**
   * @brief Return true when `c` appears in the null-terminated character set.
   * @param c Character value.
   * @param chars Null-terminated character set.
   * @return True when `c` appears in the null-terminated character set.
   */
  static bool char_in_set(char c, const char* chars) {
    for (size_type i = 0; chars[i]; ++i) if (chars[i] == c) return true;
    return false;
  }
  /** @brief Pointer to the first viewed character. */
  const char* ptr_;
  /** @brief Number of characters visible through the view. */
  size_type size_;
};

} // namespace sstl

#endif

