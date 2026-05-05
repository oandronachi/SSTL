// test_id: cpp03.compile.tracked_public_types
// api_ids: [cpp.public_headers.tracked]
// req_ids: [REQ-01, REQ-05, REQ-10]
// patterns: [STL-COMPILE-PARITY, LIFETIME]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This is primarily a compile/link contract test. Passing means the public headers are self-contained in the intended dialect/profile.
 *   - Runtime behavior is intentionally minimal; the important failure mode is a diagnostic from the compiler or linker.
 *   - Add new public headers here when the implementation grows so dialect drift is caught early.
 */
#include <sstl/array.hpp>
#include <sstl/vector.hpp>
#include <sstl/deque.hpp>
#include <sstl/list.hpp>
#include <sstl/forward_list.hpp>
#include <sstl/optional.hpp>
#include <sstl/variant.hpp>
#include "tracked.hpp"

int main() {
  sstl::array<sstl_test::tracked, 2> a;
  sstl::vector<sstl_test::tracked, 2> v;
  sstl::deque<sstl_test::tracked, 2> d;
  sstl::list<sstl_test::tracked, 2> l;
  sstl::forward_list<sstl_test::tracked, 2> fl;
  sstl::optional<sstl_test::tracked> opt;
  sstl::variant2<sstl_test::tracked, int> var;
  (void)a; (void)v; (void)d; (void)l; (void)fl; (void)opt; (void)var;
  return 0;
}
