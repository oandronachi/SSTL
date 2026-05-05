// test_id: freestanding.cpp03.public_probe
// api_ids: [cpp.freestanding.compile]
// req_ids: [REQ-01, REQ-02]
// patterns: [FREESTANDING-COMPILE]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This is primarily a compile/link contract test. Passing means the public headers are self-contained in the intended dialect/profile.
 *   - Runtime behavior is intentionally minimal; the important failure mode is a diagnostic from the compiler or linker.
 *   - Add new public headers here when the implementation grows so dialect drift is caught early.
 */
#include <sstl/vector.hpp>

extern "C" void _start() {
  sstl::vector<int, 2> v;
  (void)v.push_back(1);
  for (;;) {}
}
