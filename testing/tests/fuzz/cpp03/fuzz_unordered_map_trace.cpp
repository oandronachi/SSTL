// test_id: fuzz.cpp03.unordered_map.byte_trace
// api_ids: [cpp.unordered_map.contract, cpp.unordered_map.erase, cpp.unordered_map.subscript]
// req_ids: [REQ-02, REQ-03, REQ-05]
// patterns: [FUZZ, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This trace covers hash-table insertion, lookup, erase, bucket observers, and iterator traversal.
 *   - Capacity-affecting operations are guarded to avoid intentional contract failures in non-RETURN lanes.
 *   - The size/count invariant is cheap but catches duplicate-key or stale-node regressions.
 */
#include <cstddef>
#include <sstl/unordered_map.hpp>
#include <sstl/utility.hpp>

extern "C" int LLVMFuzzerTestOneInput(const unsigned char* data, std::size_t size) {
  sstl::unordered_map<int, int, 16, 7> items;
  for (std::size_t i = 0; i != size; ++i) {
    const unsigned char b = data[i];
    const int key = static_cast<int>(b % 19u) - 9;
    const int value = static_cast<int>((b * 29u) & 0xffu);

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
        (void)items.equal_range(key);
        break;
      case 5u:
        if (items.bucket_count() != 0u) {
          (void)items.bucket(static_cast<int>(b));
          (void)items.bucket_size(static_cast<sstl::unordered_map<int, int, 16, 7>::size_type>(b % items.bucket_count()));
        }
        break;
      case 6u:
        (void)items.load_factor();
        (void)items.max_load_factor();
        break;
      default:
        (void)items.try_at(key);
        break;
    }

    if (items.size() > items.max_size()) return 1;
    sstl::unordered_map<int, int, 16, 7>::size_type iterated = 0u;
    for (sstl::unordered_map<int, int, 16, 7>::const_iterator it = items.begin(); it != items.end(); ++it) {
      if (items.count(it->first) != 1u) return 1;
      ++iterated;
    }
    if (iterated != items.size()) return 1;
  }
  return 0;
}

#ifndef SSTL_FUZZING
int main() {
  const unsigned char seed[] = {0, 5, 10, 15, 20, 25, 30, 60, 120, 240};
  return LLVMFuzzerTestOneInput(seed, sizeof(seed));
}
#endif
