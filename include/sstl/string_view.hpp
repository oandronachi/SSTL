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
  typedef const char* const_iterator;
  /** @brief Iterator alias matching the immutable `const_iterator` type. */
  typedef const_iterator iterator;
  /** @brief Const reverse iterator over viewed characters. */
  typedef reverse_pointer_iterator<const char> const_reverse_iterator;
  /** @brief Reverse iterator alias matching the immutable const reverse iterator. */
  typedef const_reverse_iterator reverse_iterator;

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
   * @brief Return the number of viewed characters.
   * @return The number of viewed characters.
   */
  size_type length() const { return size_; }
  /**
   * @brief Return the largest representable view size.
   * @return The largest representable view size.
   */
  size_type max_size() const { return npos; }
  /**
   * @brief Return the number of bytes covered by the viewed characters.
   * @return The number of bytes covered by the viewed characters.
   */
  size_type size_bytes() const { return size_; }
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
  char operator[](size_type i) const {
    if (i >= size_) return fail_value<char>("string_view::operator[]"); // LCOV_EXCL_BR_LINE
    return ptr_[i];
  }
  /**
   * @brief Access character `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const char& at(size_type i) const {
    if (i >= size_) return fail_reference<const char>("string_view::at"); // LCOV_EXCL_BR_LINE
    return ptr_[i];
  }
  /** @brief Return a pointer to character `i`, or null when out of range. */
  const char* try_at(size_type i) const { return (i < size_ && ptr_) ? ptr_ + i : 0; }
  /**
   * @brief Access the first viewed character, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  const char& front() const {
    if (empty()) return fail_reference<const char>("string_view::front"); // LCOV_EXCL_BR_LINE
    return ptr_[0];
  }
  /** @brief Return a pointer to the first viewed character, or null when empty. */
  const char* try_front() const { return (!empty() && ptr_) ? ptr_ : 0; }
  /**
   * @brief Access the final viewed character, applying the active error policy when empty.
   * @return Result described by the function brief.
   */
  const char& back() const {
    if (empty()) return fail_reference<const char>("string_view::back"); // LCOV_EXCL_BR_LINE
    return ptr_[size_ - 1u];
  }
  /** @brief Return a pointer to the final viewed character, or null when empty. */
  const char* try_back() const { return (!empty() && ptr_) ? ptr_ + size_ - 1u : 0; }
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
  const_iterator end() const { return ptr_ ? const_iterator(ptr_ + size_) : const_iterator(ptr_); }
  /**
   * @brief Return a const iterator one past the final viewed character.
   * @return A const iterator one past the final viewed character.
   */
  const_iterator cend() const { return ptr_ ? const_iterator(ptr_ + size_) : const_iterator(ptr_); }
  /**
   * @brief Return a reverse iterator to the final viewed character.
   * @return A reverse iterator to the final viewed character.
   */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the final viewed character.
   * @return A const reverse iterator to the final viewed character.
   */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a reverse iterator one before the first viewed character.
   * @return A reverse iterator one before the first viewed character.
   */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first viewed character.
   * @return A const reverse iterator one before the first viewed character.
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

  /**
   * @brief Copy up to `count` viewed characters from `pos` into `dest`.
   * @param dest Destination buffer supplied by the caller.
   * @param count Maximum number of characters to copy.
   * @param pos First viewed character to copy.
   * @return Number of characters copied.
   */
  size_type copy(char* dest, size_type count, size_type pos = 0) const {
    if (!dest || pos > size_) {
      handle_error("string_view::copy");
      return 0u;
    }
    size_type n = count < size_ - pos ? count : size_ - pos;
    for (size_type i = 0u; i != n; ++i) dest[i] = ptr_[pos + i];
    return n;
  }

  /**
   * @brief Return a subview clamped to the viewed range.
   * @param pos First viewed character.
   * @param count Maximum number of characters in the subview.
   * @return A string_view over the requested subrange.
   */
  string_view substr(size_type pos = 0, size_type count = npos) const {
    if (pos > size_) {
      handle_error("string_view::substr");
      return string_view();
    }
    size_type n = count == npos || count > size_ - pos ? size_ - pos : count;
    return string_view(ptr_ ? ptr_ + pos : "", n);
  }

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
    if (nl > size_ - pos) return npos;
    for (size_type i = pos; i <= size_ - nl; ++i) {
      if (cmemcmp(ptr_ + i, needle, nl) == 0) return i;
    }
    return npos;
  }
  /**
   * @brief Find an explicit-length character range at or after `pos`.
   * @param needle Pointer to the first search character.
   * @param pos First position considered.
   * @param n Number of search characters.
   * @return Match position, or `npos` when absent.
   */
  size_type find(const char* needle, size_type pos, size_type n) const {
    if (!needle || pos > size_) return npos;
    if (n == 0u) return pos;
    if (n > size_ - pos) return npos;
    for (size_type i = pos; i <= size_ - n; ++i) {
      if (cmemcmp(ptr_ + i, needle, n) == 0) return i;
    }
    return npos;
  }
  /**
   * @brief Find another string_view at or after `pos`.
   * @param needle View to search for.
   * @param pos First position considered.
   * @return Match position, or `npos` when absent.
   */
  size_type find(string_view needle, size_type pos = 0) const { return find(needle.data(), pos, needle.size()); }
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
   * @brief Find the last occurrence of an explicit-length character range.
   * @param needle Pointer to the first search character.
   * @param pos Last position considered for the match start.
   * @param n Number of search characters.
   * @return Match position, or `npos` when absent.
   */
  size_type rfind(const char* needle, size_type pos, size_type n) const {
    if (!needle) return npos;
    if (n == 0u) return pos == npos || pos > size_ ? size_ : pos;
    if (n > size_) return npos;
    size_type start = size_ - n;
    if (pos != npos && pos < start) start = pos;
    for (size_type i = start + 1u; i != 0u; --i) {
      const size_type candidate = i - 1u;
      if (cmemcmp(ptr_ + candidate, needle, n) == 0) return candidate;
    }
    return npos;
  }
  /**
   * @brief Find the last occurrence of another string_view.
   * @param needle View to search for.
   * @param pos Last position considered for the match start.
   * @return Match position, or `npos` when absent.
   */
  size_type rfind(string_view needle, size_type pos = npos) const { return rfind(needle.data(), pos, needle.size()); }
  /**
   * @brief Find the last occurrence of character `c`.
   * @param c Character value.
   * @param pos Last position considered.
   * @return Match position, or `npos` when absent.
   */
  size_type rfind(char c, size_type pos = npos) const {
    if (size_ == 0u) return npos;
    size_type i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (ptr_[i] == c) return i;
      if (i == 0u) break;
      --i;
    }
    return npos;
  }
  /**
   * @brief Preserve C++03 null-pointer-literal calls such as `rfind(0)`.
   * @param c Character value.
   * @param pos Last position considered.
   * @return Match position, or `npos` when absent.
   */
  size_type rfind(int c, size_type pos = npos) const {
    return c == 0 ? npos : rfind(static_cast<char>(c), pos);
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
   * @brief Lexicographically compare this view with another view.
   * @param other Other view to compare.
   * @return Negative, zero, or positive using string ordering.
   */
  int compare(string_view other) const {
    size_type n = size_ < other.size_ ? size_ : other.size_;
    int r = cmemcmp(ptr_, other.ptr_, n);
    if (r != 0) return r;
    return size_ == other.size_ ? 0 : (size_ < other.size_ ? -1 : 1);
  }
  /**
   * @brief Compare a subview with another view.
   * @param pos First character of the subview.
   * @param count Maximum subview length.
   * @param other Other view to compare.
   * @return Negative, zero, or positive using string ordering.
   */
  int compare(size_type pos, size_type count, string_view other) const { return substr(pos, count).compare(other); }
  /**
   * @brief Compare a subview with a null-terminated string.
   * @param pos First character of the subview.
   * @param count Maximum subview length.
   * @param s Null-terminated string to compare.
   * @return Negative, zero, or positive using string ordering.
   */
  int compare(size_type pos, size_type count, const char* s) const { return substr(pos, count).compare(s); }
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
   * @brief Return true when the view begins with another view.
   * @param v Prefix view.
   * @return True when the view begins with `v`.
   */
  bool starts_with(string_view v) const { return v.size_ <= size_ && cmemcmp(ptr_, v.ptr_, v.size_) == 0; }
  /**
   * @brief Return true when the view begins with `c`.
   * @param c Character prefix.
   * @return True when the first character is `c`.
   */
  bool starts_with(char c) const { return !empty() && ptr_[0] == c; }
  /**
   * @brief Return true when the view ends with the supplied null-terminated suffix.
   * @param p Erased storage pointer.
   * @return True when the view ends with the supplied null-terminated suffix.
   */
  bool ends_with(const char* p) const {
    size_type n = cstrlen(p);
    if (!ptr_) return n == 0u;
    return n <= size_ && cmemcmp(ptr_ + size_ - n, p, n) == 0;
  }
  /**
   * @brief Return true when the view ends with another view.
   * @param v Suffix view.
   * @return True when the view ends with `v`.
   */
  bool ends_with(string_view v) const {
    if (!ptr_) return v.empty();
    return v.size_ <= size_ && cmemcmp(ptr_ + size_ - v.size_, v.ptr_, v.size_) == 0;
  }
  /**
   * @brief Return true when the view ends with `c`.
   * @param c Character suffix.
   * @return True when the final character is `c`.
   */
  bool ends_with(char c) const { return !empty() && ptr_[size_ - 1u] == c; }
  /**
   * @brief Drop up to `n` characters from the front of the view.
   * @param n Requested count or size.
   */
  void remove_prefix(size_type n) {
    if (n > size_) n = size_;
    if (ptr_) ptr_ += n;
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

/** @brief Compare two string views for equality. */
inline bool operator==(string_view lhs, string_view rhs) { return lhs.compare(rhs) == 0; }
/** @brief Compare two string views for inequality. */
inline bool operator!=(string_view lhs, string_view rhs) { return !(lhs == rhs); }
/** @brief Lexicographically compare two string views. */
inline bool operator<(string_view lhs, string_view rhs) { return lhs.compare(rhs) < 0; }
/** @brief Return true when `lhs` is not greater than `rhs`. */
inline bool operator<=(string_view lhs, string_view rhs) { return lhs.compare(rhs) <= 0; }
/** @brief Return true when `lhs` is greater than `rhs`. */
inline bool operator>(string_view lhs, string_view rhs) { return lhs.compare(rhs) > 0; }
/** @brief Return true when `lhs` is not less than `rhs`. */
inline bool operator>=(string_view lhs, string_view rhs) { return lhs.compare(rhs) >= 0; }

/** @brief Compare a string view with a null-terminated string for equality. */
inline bool operator==(string_view lhs, const char* rhs) { return lhs.compare(rhs) == 0; }
/** @brief Compare a string view with a null-terminated string for inequality. */
inline bool operator!=(string_view lhs, const char* rhs) { return !(lhs == rhs); }
/** @brief Compare a null-terminated string with a string view for equality. */
inline bool operator==(const char* lhs, string_view rhs) { return rhs.compare(lhs) == 0; }
/** @brief Compare a null-terminated string with a string view for inequality. */
inline bool operator!=(const char* lhs, string_view rhs) { return !(lhs == rhs); }

} // namespace sstl

#endif

