/**
 * @file string.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_STRING_HPP
/** @def SSTL_STRING_HPP
 * @brief Include guard for string.hpp.
 */
#define SSTL_STRING_HPP

#include "config.hpp"
#include "iterator.hpp"

namespace sstl {

/** @brief Sentinel returned by string search operations when no match exists. */
static const size_t npos = static_cast<size_t>(-1);

/**
 * @brief Compute the length of a nullable null-terminated string without using `<cstring>`.
 * @param s String or set instance.
 * @return Result described by the function brief.
 */
inline size_t cstrlen(const char* s) {
  size_t n = 0;
  while (s && s[n]) ++n;
  return n;
}

/**
 * @brief Compare two raw character ranges byte-by-byte without using `<cstring>`.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @param n Requested count or size.
 * @return Result described by the function brief.
 */
inline int cmemcmp(const char* a, const char* b, size_t n) {
  for (size_t i = 0; i != n; ++i) {
    unsigned char ac = static_cast<unsigned char>(a[i]);
    unsigned char bc = static_cast<unsigned char>(b[i]);
    if (ac != bc) return ac < bc ? -1 : 1;
  }
  return 0;
}

/**
 * @brief Compare two null-terminated strings without using `<cstring>`.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return Result described by the function brief.
 */
inline int cstrcmp(const char* a, const char* b) {
  size_t i = 0;
  while (a[i] && b[i] && a[i] == b[i]) ++i;
  unsigned char ac = static_cast<unsigned char>(a[i]);
  unsigned char bc = static_cast<unsigned char>(b[i]);
  if (ac == bc) return 0;
  return ac < bc ? -1 : 1;
}

/**
 * @brief Fixed-capacity, null-terminated string.
 * @tparam N Maximum number of non-null characters.
 *
 * The object always maintains a trailing `'\0'`, even after failed append
 * operations. Capacity excludes the terminator, so `string<8>` can hold eight
 * user-visible characters plus the internal null byte.
 */
template <size_t N>
class string {
public:
  /** @brief Character type stored by the fixed-capacity string. */
  typedef char value_type;
  /** @brief Unsigned size and index type used by the string. */
  typedef size_t size_type;
  /** @brief Mutable contiguous iterator over the non-null character range. */
  typedef contiguous_iterator<char> iterator;
  /** @brief Const contiguous iterator over the non-null character range. */
  typedef contiguous_iterator<const char> const_iterator;
  /** @brief Mutable reverse iterator over non-null characters. */
  typedef reverse_pointer_iterator<char> reverse_iterator;
  /** @brief Const reverse iterator over non-null characters. */
  typedef reverse_pointer_iterator<const char> const_reverse_iterator;

  /** @brief Construct an empty null-terminated string. */
  string() : size_(0) { data_[0] = '\0'; }
  /**
   * @brief Construct from a null-terminated source string, truncation-free when it fits.
   * @param s String or set instance.
   */
  string(const char* s) : size_(0) { data_[0] = '\0'; append(s); }

  /**
   * @brief Return the number of non-null characters.
   * @return The number of non-null characters.
   */
  size_type size() const { return size_; }
  /**
   * @brief Return the number of non-null characters.
   * @return The number of non-null characters.
   */
  size_type length() const { return size_; }
  /**
   * @brief Return the fixed non-null character capacity without requiring an object.
   * @return The fixed non-null character capacity without requiring an object.
   */
  static size_type capacity() { return N; }
  /**
   * @brief Return the fixed non-null character capacity.
   * @return The fixed non-null character capacity.
   */
  size_type max_size() const { return N; }
  /**
   * @brief Report whether the string has no non-null characters.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool empty() const { return size_ == 0; }
  /**
   * @brief Report whether all non-null character capacity is used.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool full() const { return size_ == N; }

  /**
   * @brief Return mutable access to the null-terminated character buffer.
   * @return Mutable access to the null-terminated character buffer.
   */
  char* data() { return data_; }
  /**
   * @brief Return const access to the null-terminated character buffer.
   * @return Const access to the null-terminated character buffer.
   */
  const char* data() const { return data_; }
  /**
   * @brief Return a const null-terminated C-string pointer.
   * @return A const null-terminated c-string pointer.
   */
  const char* c_str() const { return data_; }

  /**
   * @brief Return a mutable iterator to the first character.
   * @return A mutable iterator to the first character.
   */
  iterator begin() { return iterator(data_); }
  /**
   * @brief Return a const iterator to the first character.
   * @return A const iterator to the first character.
   */
  const_iterator begin() const { return const_iterator(data_); }
  /**
   * @brief Return a const iterator to the first character.
   * @return A const iterator to the first character.
   */
  const_iterator cbegin() const { return const_iterator(data_); }
  /**
   * @brief Return a mutable iterator one past the last non-null character.
   * @return A mutable iterator one past the last non-null character.
   */
  iterator end() { return iterator(data_ + size_); }
  /**
   * @brief Return a const iterator one past the last non-null character.
   * @return A const iterator one past the last non-null character.
   */
  const_iterator end() const { return const_iterator(data_ + size_); }
  /**
   * @brief Return a const iterator one past the last non-null character.
   * @return A const iterator one past the last non-null character.
   */
  const_iterator cend() const { return const_iterator(data_ + size_); }
  /**
   * @brief Return a mutable reverse iterator to the last non-null character.
   * @return A mutable reverse iterator to the last non-null character.
   */
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the last non-null character.
   * @return A const reverse iterator to the last non-null character.
   */
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a const reverse iterator to the last non-null character.
   * @return A const reverse iterator to the last non-null character.
   */
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  /**
   * @brief Return a mutable reverse iterator one before the first non-null character.
   * @return A mutable reverse iterator one before the first non-null character.
   */
  reverse_iterator rend() { return reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first non-null character.
   * @return A const reverse iterator one before the first non-null character.
   */
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  /**
   * @brief Return a const reverse iterator one before the first non-null character.
   * @return A const reverse iterator one before the first non-null character.
   */
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

  /**
   * @brief Validate that a mutable iterator lies in the non-null character range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(iterator it) const { return it.base() >= data_ && it.base() <= data_ + size_; } // LCOV_EXCL_BR_LINE

  /**
   * @brief Validate that a const iterator lies in the non-null character range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(const_iterator it) const { return it.base() >= data_ && it.base() <= data_ + size_; } // LCOV_EXCL_BR_LINE

  /**
   * @brief Validate that a mutable raw pointer lies in the non-null character range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(char* it) const { return it >= data_ && it <= data_ + size_; } // LCOV_EXCL_BR_LINE

  /**
   * @brief Validate that a const raw pointer lies in the non-null character range or end.
   * @param it Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool is_valid_iterator(const char* it) const { return it >= data_ && it <= data_ + size_; } // LCOV_EXCL_BR_LINE

  /**
   * @brief Access character `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  char& operator[](size_type i) { return data_[i]; }
  /**
   * @brief Const access character `i` without bounds checking.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const char& operator[](size_type i) const { return data_[i]; }

  /**
   * @brief Access character `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  char& at(size_type i) {
    if (i >= size_) handle_error("string::at");
    return data_[i];
  }
  /**
   * @brief Const access character `i`, applying the active error policy when out of range.
   * @param i Zero-based logical index.
   * @return Result described by the function brief.
   */
  const char& at(size_type i) const {
    if (i >= size_) handle_error("string::at");
    return data_[i];
  }
  /**
   * @brief Return a mutable pointer to character `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return A mutable pointer to character `i`, or null when out of range.
   */
  char* try_at(size_type i) { return i < size_ ? data_ + i : 0; }
  /**
   * @brief Return a const pointer to character `i`, or null when out of range.
   * @param i Zero-based logical index.
   * @return A const pointer to character `i`, or null when out of range.
   */
  const char* try_at(size_type i) const { return i < size_ ? data_ + i : 0; }

  /**
   * @brief Access the first character through `at(0)`.
   * @return Result described by the function brief.
   */
  char& front() { return at(0); }
  /**
   * @brief Const access the first character through `at(0)`.
   * @return Result described by the function brief.
   */
  const char& front() const { return at(0); }
  /**
   * @brief Access the last character through `at(size() - 1)`.
   * @return Result described by the function brief.
   */
  char& back() { return at(size_ - 1); }
  /**
   * @brief Const access the last character through `at(size() - 1)`.
   * @return Result described by the function brief.
   */
  const char& back() const { return at(size_ - 1); }
  /**
   * @brief Return a pointer to the first character, or null when empty.
   * @return A pointer to the first character, or null when empty.
   */
  char* try_front() { return empty() ? 0 : data_; }
  /**
   * @brief Return a const pointer to the first character, or null when empty.
   * @return A const pointer to the first character, or null when empty.
   */
  const char* try_front() const { return empty() ? 0 : data_; }
  /**
   * @brief Return a pointer to the last character, or null when empty.
   * @return A pointer to the last character, or null when empty.
   */
  char* try_back() { return empty() ? 0 : data_ + size_ - 1u; }
  /**
   * @brief Return a const pointer to the last character, or null when empty.
   * @return A const pointer to the last character, or null when empty.
   */
  const char* try_back() const { return empty() ? 0 : data_ + size_ - 1u; }

  /** @brief Reset to the empty string while preserving null termination. */
  void clear() { size_ = 0; data_[0] = '\0'; }

  /**
   * @brief Append one character while preserving null termination.
   * @param c Character to append.
   * @return `true` when appended; `false` when the string is already full.
   */
  bool push_back(char c) {
    if (full()) {
      handle_error("string::push_back full");
      return false;
    }
    data_[size_++] = c;
    data_[size_] = '\0';
    return true;
  }

  /**
   * @brief Remove the last character while preserving null termination.
   * @return `true` when a character was removed; `false` when the string was empty.
   */
  bool pop_back() {
    if (empty()) {
      handle_error("string::pop_back empty");
      return false;
    }
    --size_;
    data_[size_] = '\0';
    return true;
  }

  /**
   * @brief Try to append one character without invoking the active error policy.
   * @param c Character to append.
   * @return `true` when capacity accepted the character; `false` when full.
   */
  bool try_push_back(char c) {
    if (full()) return false;
    return push_back(c);
  }

  /**
   * @brief Change the logical character count using `char()` for any new slots.
   * @param n Requested number of non-null character slots.
   * @return `true` when the requested size fits in fixed capacity.
   */
  bool resize(size_type n) { return resize(n, char()); }

  /**
   * @brief Change the logical character count and fill newly exposed slots.
   * @param n Requested number of non-null character slots.
   * @param c Character copied into positions created by growth.
   * @return `true` when the requested size fits in fixed capacity.
   */
  bool resize(size_type n, char c) {
    if (n > N) {
      handle_error("string::resize full");
      return false;
    }
    while (size_ < n) data_[size_++] = c;
    size_ = n;
    data_[size_] = '\0';
    return true;
  }

  /**
   * @brief Try to resize using `char()` for growth without policy reporting.
   * @param n Requested count or size.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_resize(size_type n) { return try_resize(n, char()); }

  /**
   * @brief Try to resize and fill growth slots without invoking the active error policy.
   * @param n Requested count or size.
   * @param c Character value.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_resize(size_type n, char c) {
    if (n > N) return false;
    return resize(n, c);
  }

  /**
   * @brief Exchange contents with another string of the same fixed capacity.
   * @param other String whose current text and size are exchanged with this one.
   */
  void swap(string& other) {
    const size_type limit = size_ > other.size_ ? size_ : other.size_;
    for (size_type i = 0; i <= limit; ++i) {
      char tmp = data_[i];
      data_[i] = other.data_[i];
      other.data_[i] = tmp;
    }
    const size_type old_size = size_;
    size_ = other.size_;
    other.size_ = old_size;
  }

  /**
   * @brief Append a null-terminated C string until exhausted or full.
   * @param s Source string; must be non-null.
   * @return `true` when the whole source was appended; `false` if capacity stopped the copy.
   */
  bool append(const char* s) {
    return append(s, cstrlen(s));
  }

  /**
   * @brief Try to append a null-terminated C string without invoking the active error policy.
   * @param s String or set instance.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_append(const char* s) { return try_append(s, cstrlen(s)); }

  /**
   * @brief Append exactly `n` characters from a raw buffer.
   * @param s Source buffer; must point to at least `n` characters.
   * @param n Number of characters to append.
   * @return `true` when all characters fit; `false` when capacity is insufficient.
   *
   * The capacity check is performed before mutation so failed appends leave the
   * string text and terminator unchanged.
   */
  bool append(const char* s, size_type n) {
    if (n > N - size_) {
      handle_error("string::append full");
      return false;
    }
    for (size_type i = 0; i != n; ++i) {
      data_[size_++] = s[i];
    }
    data_[size_] = '\0';
    return true;
  }

  /**
   * @brief Try to append exactly `n` characters without invoking the active error policy.
   * @param s String or set instance.
   * @param n Requested count or size.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_append(const char* s, size_type n) {
    if (n > N - size_) return false;
    return append(s, n);
  }

  /**
   * @brief Replace the current contents with a null-terminated C string.
   * @param s String or set instance.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool assign(const char* s) { return assign(s, cstrlen(s)); }

  /**
   * @brief Try to replace the current contents with a null-terminated C string quietly.
   * @param s String or set instance.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_assign(const char* s) { return try_assign(s, cstrlen(s)); }

  /**
   * @brief Replace the current contents with the first `n` characters of `s`.
   * @param s String or set instance.
   * @param n Requested count or size.
   * @return Result described by the function brief.
   */
  bool assign(const char* s, size_type n) {
    if (n > N) {
      handle_error("string::assign full");
      return false;
    }
    clear();
    return append(s, n);
  }

  /**
   * @brief Try to replace the current contents with `n` characters quietly.
   * @param s String or set instance.
   * @param n Requested count or size.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_assign(const char* s, size_type n) {
    if (n > N) return false;
    return assign(s, n);
  }

  /**
   * @brief Insert a character before `pos`.
   * @param pos Zero-based logical position.
   * @param c Character value.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool insert(size_type pos, char c) {
    if (pos > size_ || full()) {
      handle_error("string::insert");
      return false;
    }
    for (size_type i = size_; i != pos; --i) data_[i] = data_[i - 1u];
    data_[pos] = c;
    ++size_;
    data_[size_] = '\0';
    return true;
  }

  /**
   * @brief Try to insert one character before `pos` without policy reporting.
   * @param pos Zero-based logical position.
   * @param c Character value.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_insert(size_type pos, char c) {
    if (pos > size_ || full()) return false;
    return insert(pos, c);
  }

  /**
   * @brief Insert a null-terminated string before `pos`.
   * @param pos Zero-based logical position.
   * @param s String or set instance.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool insert(size_type pos, const char* s) {
    const size_type n = cstrlen(s);
    if (n == 0u) return pos <= size_;
    if (pos > size_ || n > N - size_) {
      handle_error("string::insert");
      return false;
    }
    for (size_type i = size_; i != pos; --i) data_[i + n - 1u] = data_[i - 1u];
    for (size_type i = 0; i != n; ++i) data_[pos + i] = s[i];
    size_ += n;
    data_[size_] = '\0';
    return true;
  }

  /**
   * @brief Try to insert a null-terminated string before `pos` without policy reporting.
   * @param pos Zero-based logical position.
   * @param s String or set instance.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_insert(size_type pos, const char* s) {
    const size_type n = cstrlen(s);
    if (n == 0u) return pos <= size_;
    if (pos > size_ || n > N - size_) return false;
    return insert(pos, s);
  }

  /**
   * @brief Erase up to `count` characters starting at `pos`.
   * @param pos Zero-based logical position.
   * @param count Requested element or character count.
   * @return Result described by the function brief.
   */
  string& erase(size_type pos, size_type count) {
    if (pos > size_) {
      handle_error("string::erase");
      return *this;
    }
    if (count > size_ - pos) count = size_ - pos;
    for (size_type i = pos + count; i <= size_; ++i) data_[i - count] = data_[i];
    size_ -= count;
    return *this;
  }

  /**
   * @brief Replace a substring with a null-terminated source string.
   * @param pos Zero-based logical position.
   * @param count Requested element or character count.
   * @param s String or set instance.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool replace(size_type pos, size_type count, const char* s) {
    const size_type n = cstrlen(s);
    if (pos > size_) {
      handle_error("string::replace");
      return false;
    }
    if (count > size_ - pos) count = size_ - pos;
    if (n > N - (size_ - count)) {
      handle_error("string::replace full");
      return false;
    }
    if (n < count) {
      for (size_type i = pos + count; i <= size_; ++i) data_[i - (count - n)] = data_[i];
    } else if (n > count) {
      for (size_type i = size_ + 1u; i != pos + count; --i) data_[i + (n - count) - 1u] = data_[i - 1u];
    }
    for (size_type i = 0; i != n; ++i) data_[pos + i] = s[i];
    size_ = size_ - count + n;
    data_[size_] = '\0';
    return true;
  }

  /**
   * @brief Try to replace a substring without invoking the active error policy.
   * @param pos Zero-based logical position.
   * @param count Requested element or character count.
   * @param s String or set instance.
   * @return `true` on success; otherwise `false` without invoking the panic policy.
   */
  bool try_replace(size_type pos, size_type count, const char* s) {
    const size_type n = cstrlen(s);
    if (pos > size_) return false;
    if (count > size_ - pos) count = size_ - pos;
    if (n > N - (size_ - count)) return false;
    return replace(pos, count, s);
  }

  /**
   * @brief Append a null-terminated string and return this string.
   * @param s String or set instance.
   * @return Result described by the function brief.
   */
  string& operator+=(const char* s) { append(s); return *this; }
  /**
   * @brief Append another fixed-capacity string and return this string.
   * @param s String or set instance.
   * @return Result described by the function brief.
   */
  template <size_t M>
  string& operator+=(const string<M>& s) { append(s.c_str(), s.size()); return *this; }
  /**
   * @brief Append one character and return this string.
   * @param c Character value.
   * @return Result described by the function brief.
   */
  string& operator+=(char c) { push_back(c); return *this; }

  /**
   * @brief Lexicographically compare this string with a C string.
   * @param s Null-terminated string to compare against.
   * @return Negative, zero, or positive using `strcmp`-style ordering.
   */
  int compare(const char* s) const { return cstrcmp(data_, s); }

  /**
   * @brief Lexicographically compare this string with another fixed-capacity string.
   * @tparam M Capacity of the other string.
   * @param s String whose current text is compared against this string.
   * @return Negative, zero, or positive using `strcmp`-style ordering.
   */
  template <size_t M>
  int compare(const string<M>& s) const { return compare(s.c_str()); }

  /**
   * @brief Find the first occurrence of a C-string needle at or after `pos`.
   * @param needle Null-terminated substring to search for.
   * @param pos First position considered.
   * @return Match position, or `sstl::npos` when absent.
   */
  size_type find(const char* needle, size_type pos = 0) const {
    if (!needle || pos > size_) return npos;
    size_type nl = cstrlen(needle);
    if (nl == 0) return pos;
    for (size_type i = pos; i + nl <= size_; ++i) {
      if (cmemcmp(data_ + i, needle, nl) == 0) return i;
    }
    return npos;
  }

  /**
   * @brief Find the first character at or after `pos` that belongs to `chars`.
   * @param chars Null-terminated character set.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_first_of(const char* chars, size_type pos = 0) const {
    if (!chars || pos >= size_) return npos;
    for (size_type i = pos; i != size_; ++i) if (char_in_set(data_[i], chars)) return i;
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
    for (size_type i = pos; i != size_; ++i) {
      if (data_[i] == c) return i;
    }
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
   * @brief Find the first occurrence of character `c` at or after `pos`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_first_of(char c, size_type pos = 0) const {
    if (pos >= size_) return npos;
    for (size_type i = pos; i != size_; ++i) if (data_[i] == c) return i;
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
    for (size_type i = pos; i != size_; ++i) if (!char_in_set(data_[i], chars)) return i;
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
    for (size_type i = pos; i != size_; ++i) if (data_[i] != c) return i;
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
   * @brief Find the last character at or before `pos` that belongs to `chars`.
   * @param chars Null-terminated character set.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type find_last_of(const char* chars, size_type pos = npos) const {
    if (!chars || size_ == 0u) return npos;
    size_type i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (char_in_set(data_[i], chars)) return i;
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
    size_type i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (data_[i] == c) return i;
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
    size_type i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (!char_in_set(data_[i], chars)) return i;
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
    size_type i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (data_[i] != c) return i;
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
   * @brief Find the last occurrence of a C-string needle at or before `pos`.
   * @param needle Null-terminated substring to search for.
   * @param pos Last position considered for the match start.
   * @return Match position, or `sstl::npos` when absent.
   */
  size_type rfind(const char* needle, size_type pos = npos) const {
    if (!needle) return npos;
    size_type nl = cstrlen(needle);
    if (nl == 0) return pos == npos || pos > size_ ? size_ : pos;
    if (nl > size_) return npos;
    size_type start = size_ - nl;
    if (pos != npos && pos < start) start = pos;
    for (size_type i = start + 1u; i != 0u; --i) {
      size_type current = i - 1u;
      if (cmemcmp(data_ + current, needle, nl) == 0) return current;
    }
    return npos;
  }

  /**
   * @brief Find the last occurrence of character `c` at or before `pos`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type rfind(char c, size_type pos = npos) const {
    if (size_ == 0u) return npos;
    size_type i = pos == npos || pos >= size_ ? size_ - 1u : pos;
    for (;;) {
      if (data_[i] == c) return i;
      if (i == 0u) break;
      --i;
    }
    return npos;
  }

  /**
   * @brief Preserve C++03 null-pointer-literal calls such as `rfind(0)`.
   * @param c Character value.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  size_type rfind(int c, size_type pos = npos) const {
    return c == 0 ? npos : rfind(static_cast<char>(c), pos);
  }

  /**
   * @brief Return a fixed-capacity copy of a substring.
   * @param pos First source position.
   * @param count Maximum number of characters to copy.
   * @return A new `string<N>` containing up to `count` characters.
   */
  string substr(size_type pos, size_type count) const {
    string out;
    for (size_type i = 0; i != count && pos + i < size_; ++i) out.push_back(data_[pos + i]);
    return out;
  }

  /**
   * @brief Compare this string with a C string for equality.
   * @param rhs Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool operator==(const char* rhs) const { return compare(rhs) == 0; }
  /**
   * @brief Compare this string with a C string for inequality.
   * @param rhs Caller-supplied argument used by this operation.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool operator!=(const char* rhs) const { return compare(rhs) != 0; }
  /**
   * @brief Return true when this string is lexicographically less than a C string.
   * @param rhs Caller-supplied argument used by this operation.
   * @return True when this string is lexicographically less than a c string.
   */
  bool operator<(const char* rhs) const { return compare(rhs) < 0; }
  /**
   * @brief Return true when this string is not lexicographically greater than a C string.
   * @param rhs Caller-supplied argument used by this operation.
   * @return True when this string is not lexicographically greater than a c string.
   */
  bool operator<=(const char* rhs) const { return compare(rhs) <= 0; }
  /**
   * @brief Return true when this string is lexicographically greater than a C string.
   * @param rhs Caller-supplied argument used by this operation.
   * @return True when this string is lexicographically greater than a c string.
   */
  bool operator>(const char* rhs) const { return compare(rhs) > 0; }
  /**
   * @brief Return true when this string is not lexicographically less than a C string.
   * @param rhs Caller-supplied argument used by this operation.
   * @return True when this string is not lexicographically less than a c string.
   */
  bool operator>=(const char* rhs) const { return compare(rhs) >= 0; }

  /**
   * @brief Compare this string with another fixed-capacity string for equality.
   * @tparam M Capacity of the other string.
   * @param rhs String whose current text is compared against this string.
   * @return True when both strings contain the same character sequence.
   */
  template <size_t M>
  bool operator==(const string<M>& rhs) const { return compare(rhs) == 0; }

  /**
   * @brief Compare this string with another fixed-capacity string for inequality.
   * @tparam M Capacity of the other string.
   * @param rhs String whose current text is compared against this string.
   * @return True when the strings differ.
   */
  template <size_t M>
  bool operator!=(const string<M>& rhs) const { return compare(rhs) != 0; }

  /**
   * @brief Return true when this string is lexicographically less than another string.
   * @tparam M Capacity of the other string.
   * @param rhs String whose current text is compared against this string.
   * @return True when this string sorts before `rhs`.
   */
  template <size_t M>
  bool operator<(const string<M>& rhs) const { return compare(rhs) < 0; }

  /**
   * @brief Return true when this string is not lexicographically greater than another string.
   * @tparam M Capacity of the other string.
   * @param rhs String whose current text is compared against this string.
   * @return True when this string sorts before or equal to `rhs`.
   */
  template <size_t M>
  bool operator<=(const string<M>& rhs) const { return compare(rhs) <= 0; }

  /**
   * @brief Return true when this string is lexicographically greater than another string.
   * @tparam M Capacity of the other string.
   * @param rhs String whose current text is compared against this string.
   * @return True when this string sorts after `rhs`.
   */
  template <size_t M>
  bool operator>(const string<M>& rhs) const { return compare(rhs) > 0; }

  /**
   * @brief Return true when this string is not lexicographically less than another string.
   * @tparam M Capacity of the other string.
   * @param rhs String whose current text is compared against this string.
   * @return True when this string sorts after or equal to `rhs`.
   */
  template <size_t M>
  bool operator>=(const string<M>& rhs) const { return compare(rhs) >= 0; }

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

  /** @brief Inline character buffer including the required trailing null byte. */
  char data_[N + 1];
  /** @brief Number of non-null characters currently stored in `data_`. */
  size_type size_;
};

/**
 * @brief Exchange two fixed-capacity strings of the same capacity.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 */
template <size_t N>
inline void swap(string<N>& lhs, string<N>& rhs) { lhs.swap(rhs); }

/**
 * @brief Compare a C string with a fixed-capacity string for equality.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <size_t N>
inline bool operator==(const char* lhs, const string<N>& rhs) { return rhs.compare(lhs) == 0; }

/**
 * @brief Compare a C string with a fixed-capacity string for inequality.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return `true` when the documented condition holds; otherwise `false`.
 */
template <size_t N>
inline bool operator!=(const char* lhs, const string<N>& rhs) { return rhs.compare(lhs) != 0; }

/**
 * @brief Return true when a C string is lexicographically less than a fixed-capacity string.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when a c string is lexicographically less than a fixed-capacity string.
 */
template <size_t N>
inline bool operator<(const char* lhs, const string<N>& rhs) { return cstrcmp(lhs, rhs.c_str()) < 0; }

/**
 * @brief Return true when a C string is not lexicographically greater than a fixed-capacity string.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when a c string is not lexicographically greater than a fixed-capacity string.
 */
template <size_t N>
inline bool operator<=(const char* lhs, const string<N>& rhs) { return cstrcmp(lhs, rhs.c_str()) <= 0; }

/**
 * @brief Return true when a C string is lexicographically greater than a fixed-capacity string.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when a c string is lexicographically greater than a fixed-capacity string.
 */
template <size_t N>
inline bool operator>(const char* lhs, const string<N>& rhs) { return cstrcmp(lhs, rhs.c_str()) > 0; }

/**
 * @brief Return true when a C string is not lexicographically less than a fixed-capacity string.
 * @param lhs Caller-supplied argument used by this operation.
 * @param rhs Caller-supplied argument used by this operation.
 * @return True when a c string is not lexicographically less than a fixed-capacity string.
 */
template <size_t N>
inline bool operator>=(const char* lhs, const string<N>& rhs) { return cstrcmp(lhs, rhs.c_str()) >= 0; }

/** @brief FNV-1a hash specialization that considers only the live string characters. */
template <size_t N>
struct hash<string<N> > {
  /**
   * @brief Hash the logical string contents without reading unused inline capacity.
   * @return Result described by the function brief.
   * @param text Caller-supplied argument used by this operation.
   */
  unsigned operator()(const string<N>& text) const {
    unsigned h = 2166136261u;
    for (unsigned i = 0u; i != text.size(); ++i) {
      h ^= static_cast<unsigned char>(text[i]);
      h *= 16777619u;
    }
    return h;
  }
};

} // namespace sstl

#endif

