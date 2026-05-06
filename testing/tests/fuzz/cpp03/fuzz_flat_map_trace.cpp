// test_id: fuzz.cpp03.flat_map.byte_trace
// api_ids: [cpp.flat_map.contract, cpp.flat_map.erase, cpp.flat_map.bounds]
// req_ids: [REQ-02, REQ-03, REQ-05]
// patterns: [FUZZ, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This trace exercises sorted insertion, lookup, erase, and bounds APIs under arbitrary byte input.
 *   - Inserts and operator[] are capacity-guarded so the generic PANIC lane remains a valid smoke replay.
 *   - The ordering check verifies the flat-map invariant after every mutation.
 */
#include <cstddef>
#include <sstl/flat_map.hpp>
#include <sstl/utility.hpp>

extern "C" int LLVMFuzzerTestOneInput(const unsigned char* data, std::size_t size) {
  sstl::flat_map<int, int, 16> items;
  for (std::size_t i = 0; i != size; ++i) {
    const unsigned char b = data[i];
    const int key = static_cast<int>(b % 23u) - 11;
    const int value = static_cast<int>((b * 17u) & 0xffu);

    switch (b & 7u) {
      case 0u:
        if (!items.full() || items.find(key) != items.end()) {
          (void)items.insert(sstl::make_pair(key, value));
        }
        break;
      case 1u:
        if (!items.full() || items.find(key) != items.end()) {
          items[key] = value;
        }
        break;
      case 2u:
        (void)items.erase(key);
        break;
      case 3u:
        (void)items.find(key);
        break;
      case 4u:
        (void)items.lower_bound(key);
        (void)items.upper_bound(key);
        break;
      case 5u:
        (void)items.equal_range(key);
        break;
      case 6u:
        if (!items.empty()) {
          sstl::flat_map<int, int, 16>::iterator it = items.begin() + static_cast<sstl::flat_map<int, int, 16>::iterator::difference_type>(b % items.size());
          (void)it->second;
        }
        break;
      default:
        (void)items.try_at(key);
        break;
    }

    if (items.size() > items.max_size()) return 1;
    if (!items.empty()) {
      sstl::flat_map<int, int, 16>::const_iterator prev = items.begin();
      sstl::flat_map<int, int, 16>::const_iterator cur = prev;
      ++cur;
      for (; cur != items.end(); ++cur, ++prev) {
        if (!(prev->first < cur->first)) return 1;
      }
    }
  }
  return 0;
}

#ifndef SSTL_FUZZING
int main() {
  const unsigned char seed[] = {8, 1, 2, 3, 13, 21, 34, 55, 89, 144, 233};
  return LLVMFuzzerTestOneInput(seed, sizeof(seed));
}
#endif
