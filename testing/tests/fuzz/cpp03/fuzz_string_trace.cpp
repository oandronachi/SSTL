// test_id: fuzz.cpp03.string.byte_trace
// api_ids: [cpp.string.push_back, cpp.string.pop_back, cpp.string.append_n, cpp.string.insert, cpp.string.erase, cpp.string.resize, cpp.string.find, cpp.string.compare]
// req_ids: [REQ-02, REQ-03, REQ-05]
// patterns: [FUZZ, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This is a deterministic byte-trace replay for CTest and a libFuzzer entry point when SSTL_FUZZING is defined.
 *   - Operations use try_* or prevalidated positions so PANIC and UB policy lanes do not intentionally trip contracts.
 *   - The invariant check keeps the fixed-capacity string's public null terminator under fuzz pressure.
 */
#include <cstddef>
#include <sstl/string.hpp>

extern "C" int LLVMFuzzerTestOneInput(const unsigned char* data, std::size_t size) {
  sstl::string<16> text;
  for (std::size_t i = 0; i != size; ++i) {
    const unsigned char b = data[i];
    const char ch = static_cast<char>('a' + (b % 26u));
    char fragment[4];
    fragment[0] = ch;
    fragment[1] = static_cast<char>('A' + ((b >> 1u) % 26u));
    fragment[2] = static_cast<char>('0' + ((b >> 2u) % 10u));
    fragment[3] = '\0';

    switch (b & 7u) {
      case 0u:
        (void)text.try_push_back(ch);
        break;
      case 1u:
        (void)text.try_pop_back();
        break;
      case 2u:
        (void)text.try_append(fragment, static_cast<sstl::string<16>::size_type>(b % 4u));
        break;
      case 3u:
        (void)text.try_insert(static_cast<sstl::string<16>::size_type>(b % (text.size() + 1u)), ch);
        break;
      case 4u:
        if (!text.empty()) {
          const sstl::string<16>::size_type pos = static_cast<sstl::string<16>::size_type>(b % text.size());
          text.erase(pos, static_cast<sstl::string<16>::size_type>((b >> 3u) % 4u));
        }
        break;
      case 5u:
        (void)text.try_resize(static_cast<sstl::string<16>::size_type>(b % (text.max_size() + 1u)), ch);
        break;
      case 6u:
        (void)text.find(fragment);
        break;
      default:
        (void)text.compare(fragment);
        break;
    }

    if (text.size() > text.max_size()) return 1;
    if (text.c_str()[text.size()] != '\0') return 1;
  }
  return 0;
}

#ifndef SSTL_FUZZING
int main() {
  const unsigned char seed[] = {0, 2, 3, 4, 7, 15, 31, 63, 127, 255};
  return LLVMFuzzerTestOneInput(seed, sizeof(seed));
}
#endif
