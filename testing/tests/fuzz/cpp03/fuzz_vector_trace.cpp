// test_id: fuzz.cpp03.vector.byte_trace
// api_ids: [cpp.vector.erase, cpp.vector.push_back]
// req_ids: [REQ-02, REQ-03, REQ-05]
// patterns: [FUZZ, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - The function is a libFuzzer entry point, but it also has a deterministic seed-replay main so ordinary CTest runs exercise the same parser surface.
 *   - Fuzz targets must be deterministic and tolerate arbitrary byte input without leaking state between calls.
 *   - The seed replay is not a substitute for the libFuzzer lane; it is a cheap smoke path.
 */
#include <cstddef>
#include <sstl/vector.hpp>

extern "C" int LLVMFuzzerTestOneInput(const unsigned char* data, std::size_t size) {
  sstl::vector<int, 16> v;
  for (std::size_t i = 0; i != size; ++i) {
    const unsigned char b = data[i];
    switch (b & 3u) {
      case 0u:
        (void)v.push_back(static_cast<int>(b));
        break;
      case 1u:
        if (!v.empty()) v.pop_back();
        break;
      case 2u:
        if (!v.empty()) v.erase(v.begin() + (b % v.size()));
        break;
      default:
        if (!v.empty()) (void)v.at(b % v.size());
        break;
    }
    if (v.size() > v.capacity()) return 1;
  }
  return 0;
}

#ifndef SSTL_FUZZING
int main() {
  const unsigned char seed[] = {0, 1, 2, 3, 255, 17, 99};
  return LLVMFuzzerTestOneInput(seed, sizeof(seed));
}
#endif
