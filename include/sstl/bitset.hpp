/**
 * @file bitset.hpp
 * @brief C++03 SSTL public header with static, allocation-free API surface.
 *
 * The declarations in this file are part of the local SSTL contract. They are
 * documented for Doxygen consumers and for maintainers reading the headers
 * directly. Keep behavior aligned with the SSTL public contract and tests,
 * and avoid introducing hosted STL dependencies into implementation
 * headers.
 */
#ifndef SSTL_BITSET_HPP
/** @def SSTL_BITSET_HPP
 * @brief Include guard for bitset.hpp.
 */
#define SSTL_BITSET_HPP

#include "config.hpp"
#include "string.hpp"

namespace sstl {

/** @brief Fixed-size bit collection backed by inline word storage. */
template <size_t N>
class bitset {
public:
  /** @brief Unsigned size and index type used by the bitset. */
  typedef size_t size_type;

  /**
   * @brief Mutable single-bit proxy returned by non-const `operator[]`.
   *
   * C++ cannot return a real `bool&` for a packed bit. This lightweight proxy
   * stores the owning bitset and bit position, then forwards assignment,
   * conversion, and flipping to the parent bitset without allocating storage.
   */
  class reference {
  public:
    /**
     * @brief Assign a boolean value to the referenced bit.
     * @param value New bit value.
     * @return This proxy so chained assignments remain possible.
     */
    reference& operator=(bool value) { owner_->set(pos_, value); return *this; }
    /**
     * @brief Copy the value from another bit reference.
     * @param other Other bit reference being read.
     * @return This proxy after assignment.
     */
    reference& operator=(const reference& other) { return *this = static_cast<bool>(other); }
    /**
     * @brief Convert the referenced bit to a boolean value.
     * @return True when the referenced bit is set.
     */
    operator bool() const { return owner_->test(pos_); }
    /**
     * @brief Return the logical inverse of the referenced bit.
     * @return True when the referenced bit is clear.
     */
    bool operator~() const { return !owner_->test(pos_); }
    /**
     * @brief Toggle the referenced bit.
     * @return This proxy after flipping the bit.
     */
    reference& flip() { owner_->flip(pos_); return *this; }

  private:
    /** @brief Owning bitset whose packed storage contains the referenced bit. */
    bitset* owner_;
    /** @brief Zero-based bit position inside the owning bitset. */
    size_type pos_;

    /**
     * @brief Construct a proxy for one mutable bit.
     * @param owner Owning bitset.
     * @param pos Zero-based bit position.
     */
    reference(bitset& owner, size_type pos) : owner_(&owner), pos_(pos) {}

    /** @brief Allow `bitset` to create valid proxies. */
    friend class bitset;
  };

  /** @brief Construct a bitset with all bits cleared. */
  bitset() { reset(); }
  /**
   * @brief Construct a bitset from the low bits of an unsigned long value.
   * @param value Source integer whose bit zero maps to bit position zero.
   */
  explicit bitset(unsigned long value) { assign_ulong(value); }
  /**
   * @brief Construct from a null-terminated high-to-low bit string.
   * @param text String containing `0` and `1` characters.
   */
  explicit bitset(const char* text) { assign_text(text, cstrlen(text)); }
  /**
   * @brief Construct from an SSTL string containing high-to-low bit text.
   * @tparam M Source string capacity.
   * @param text String containing `0` and `1` characters.
   */
  template <size_t M>
  explicit bitset(const string<M>& text) { assign_text(text.c_str(), text.size()); }

  /** @brief Clear every bit in the set. */
  void reset() { for (size_type i = 0; i != words; ++i) data_[i] = 0u; }
  /**
   * @brief Clear the bit at `pos` and return this bitset.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  bitset& reset(size_type pos) { data_[pos / bits] &= ~(word_type(1) << (pos % bits)); return *this; }
  /**
   * @brief Set every public bit and return this bitset.
   * @return This bitset after all public bits are set.
   */
  bitset& set() { for (size_type i = 0; i != words; ++i) data_[i] = ~word_type(0); trim(); return *this; }
  /**
   * @brief Set the bit at `pos` and return this bitset.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  bitset& set(size_type pos) { data_[pos / bits] |= (word_type(1) << (pos % bits)); return *this; }
  /**
   * @brief Set or clear one bit according to `value`.
   * @param pos Zero-based logical position.
   * @param value New bit value.
   * @return This bitset after the requested bit is updated.
   */
  bitset& set(size_type pos, bool value) { return value ? set(pos) : reset(pos); }
  /**
   * @brief Toggle the bit at `pos` and return this bitset.
   * @param pos Zero-based logical position.
   * @return Result described by the function brief.
   */
  bitset& flip(size_type pos) { data_[pos / bits] ^= (word_type(1) << (pos % bits)); return *this; }
  /**
   * @brief Toggle every public bit and mask off unused storage bits.
   * @return Result described by the function brief.
   */
  bitset& flip() { for (size_type i = 0; i != words; ++i) data_[i] = ~data_[i]; trim(); return *this; }
  /**
   * @brief Return true when bit `pos` is set.
   * @param pos Zero-based logical position.
   * @return True when bit `pos` is set.
   */
  bool test(size_type pos) const { return (data_[pos / bits] & (word_type(1) << (pos % bits))) != 0u; }
  /**
   * @brief Return true when bit `pos` is set.
   * @param pos Zero-based logical position.
   * @return True when bit `pos` is set.
   */
  bool operator[](size_type pos) const { return test(pos); }
  /**
   * @brief Return a mutable proxy for bit `pos`.
   * @param pos Zero-based logical position.
   * @return Proxy that can read, assign, or flip the packed bit.
   */
  reference operator[](size_type pos) { return reference(*this, pos); }
  /**
   * @brief Return the number of public bits.
   * @return The number of public bits.
   */
  size_type size() const { return N; }
  /**
   * @brief Count the number of bits currently set.
   * @return Result described by the function brief.
   */
  size_type count() const {
    size_type n = 0;
    for (size_type i = 0; i != N; ++i) if (test(i)) ++n;
    return n;
  }
  /**
   * @brief Report whether at least one bit is set.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool any() const { return count() != 0u; }
  /**
   * @brief Report whether no bits are set.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool none() const { return !any(); }
  /**
   * @brief Report whether every public bit is set.
   * @return `true` when the documented condition holds; otherwise `false`.
   */
  bool all() const { return count() == N; }
  /**
   * @brief Return the low storage word and report overflow when higher bits are set.
   * @return The low storage word and report overflow when higher bits are set.
   */
  unsigned long to_ulong() const {
    for (size_type i = bits; i != N; ++i) {
      if (test(i)) {
        handle_error("bitset::to_ulong overflow");
        break;
      }
    }
    return words == 0 ? 0ul : data_[0];
  }
  /**
   * @brief Return an allocation-free string containing high-to-low bit text.
   * @return An allocation-free string containing high-to-low bit text.
   */
  string<N> to_string() const {
    string<N> out;
    for (size_type i = 0; i != N; ++i) out.push_back(test(N - 1u - i) ? '1' : '0');
    return out;
  }
  /**
   * @brief Return a copy with every public bit inverted.
   * @return A copy with every public bit inverted.
   */
  bitset operator~() const { bitset out(*this); out.flip(); return out; }
  /**
   * @brief Compare two bitsets for identical public bit values.
   * @param other Other bitset participating in the comparison.
   * @return True when every public bit is equal.
   */
  bool operator==(const bitset& other) const {
    for (size_type i = 0; i != words; ++i) {
      if (data_[i] != other.data_[i]) return false;
    }
    return true;
  }
  /**
   * @brief Compare two bitsets for any differing public bit value.
   * @param other Other bitset participating in the comparison.
   * @return True when at least one public bit differs.
   */
  bool operator!=(const bitset& other) const { return !(*this == other); }
  /**
   * @brief Apply bitwise AND with another bitset.
   * @param other Other object participating in the operation.
   * @return Result described by the function brief.
   */
  bitset& operator&=(const bitset& other) { for (size_type i = 0; i != words; ++i) data_[i] &= other.data_[i]; return *this; }
  /**
   * @brief Apply bitwise OR with another bitset.
   * @param other Other object participating in the operation.
   * @return Result described by the function brief.
   */
  bitset& operator|=(const bitset& other) { for (size_type i = 0; i != words; ++i) data_[i] |= other.data_[i]; trim(); return *this; }
  /**
   * @brief Apply bitwise XOR with another bitset.
   * @param other Other object participating in the operation.
   * @return Result described by the function brief.
   */
  bitset& operator^=(const bitset& other) { for (size_type i = 0; i != words; ++i) data_[i] ^= other.data_[i]; trim(); return *this; }
  /**
   * @brief Shift bits left by `shift`, clearing vacated and overflow positions.
   * @param shift Caller-supplied argument used by this operation.
   * @return Result described by the function brief.
   */
  bitset& operator<<=(size_type shift) {
    if (shift >= N) {
      reset();
      return *this;
    }
    bitset old(*this);
    reset();
    for (size_type i = N; i != 0; --i) {
      size_type pos = i - 1;
      if (pos >= shift && old.test(pos - shift)) set(pos);
    }
    return *this;
  }
  /**
   * @brief Shift bits right by `shift`, clearing vacated and overflow positions.
   * @param shift Caller-supplied argument used by this operation.
   * @return Result described by the function brief.
   */
  bitset& operator>>=(size_type shift) {
    if (shift >= N) {
      reset();
      return *this;
    }
    bitset old(*this);
    reset();
    for (size_type pos = 0; pos != N; ++pos) {
      if (pos + shift < N && old.test(pos + shift)) set(pos);
    }
    return *this;
  }
  /**
   * @brief Return a shifted-left copy without modifying this bitset.
   * @param shift Number of bit positions to shift.
   * @return Shifted copy with vacated and overflow positions cleared.
   */
  bitset operator<<(size_type shift) const { bitset out(*this); out <<= shift; return out; }
  /**
   * @brief Return a shifted-right copy without modifying this bitset.
   * @param shift Number of bit positions to shift.
   * @return Shifted copy with vacated and overflow positions cleared.
   */
  bitset operator>>(size_type shift) const { bitset out(*this); out >>= shift; return out; }
private:
  /** @brief Unsigned storage word deliberately shared with the C bitset macro family. */
  typedef unsigned long word_type;
  /**
   * @brief Storage geometry constants: bits per word and public word count.
   * @return Result described by the function brief.
   */
  enum { bits = sizeof(word_type) * 8, words = (N + bits - 1) / bits };
  /** @brief Inline packed-word storage; keeps one dummy word when `N == 0`. */
  word_type data_[words == 0 ? 1 : words];
  /** @brief Clear unused high bits in the final word after whole-bitset operations. */
  void trim() {
    if (N != 0 && (N % bits) != 0) data_[words - 1] &= ((word_type(1) << (N % bits)) - 1u);
  }
  /**
   * @brief Replace all bits from the low positions of an integer value.
   * @param value Source integer value.
   */
  void assign_ulong(unsigned long value) {
    reset();
    for (size_type pos = 0; pos != N && pos != bits; ++pos) {
      if ((value & (word_type(1) << pos)) != 0u) set(pos);
    }
  }
  /**
   * @brief Replace all bits from high-to-low character text.
   * @param text Source character buffer.
   * @param len Number of characters to read.
   */
  void assign_text(const char* text, size_type len) {
    reset();
    if (!text) return;
    size_type usable = len < N ? len : N;
    for (size_type i = 0; i != usable; ++i) {
      char c = text[len - 1u - i];
      if (c == '1') set(i);
    }
  }
};

/**
 * @brief Return the bitwise AND of two bitsets.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return The bitwise and of two bitsets.
 */
template <size_t N>
inline bitset<N> operator&(bitset<N> a, const bitset<N>& b) { a &= b; return a; }
/**
 * @brief Return the bitwise OR of two bitsets.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return The bitwise or of two bitsets.
 */
template <size_t N>
inline bitset<N> operator|(bitset<N> a, const bitset<N>& b) { a |= b; return a; }
/**
 * @brief Return the bitwise XOR of two bitsets.
 * @param a First operand or first range start.
 * @param b Second operand or second range start.
 * @return The bitwise xor of two bitsets.
 */
template <size_t N>
inline bitset<N> operator^(bitset<N> a, const bitset<N>& b) { a ^= b; return a; }

} // namespace sstl

#endif

