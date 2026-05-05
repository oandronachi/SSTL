// test_id: cpp03.policy.default_debug_custom_panic_hook
// api_ids: [cpp.policy.default, cpp.policy.panic, cpp.vector.at]
// req_ids: [REQ-04]
// patterns: [POLICY-MATRIX, EDGE-BRANCH-COVERAGE]
/*
 * Notes:
 *   - This translation unit is intentionally compiled without an explicit
 *     SSTL_ON_ERROR definition from CMake.
 *   - It selects the debug default policy by ensuring NDEBUG is absent before
 *     any SSTL header is included.
 *   - It disables the default panic hook and installs a local longjmp hook.
 *     That keeps the panic-dispatch branch observable in coverage without
 *     allowing the intentionally loud default trap to terminate the test
 *     process or open platform abort dialogs.
 */
#ifdef NDEBUG
# undef NDEBUG
#endif

#define SSTL_NO_DEFAULT_PANIC 1

#include <setjmp.h>

#include <sstl/vector.hpp>

#include "test_harness.hpp"

#if SSTL_ON_ERROR != SSTL_PANIC
# error Debug default policy must be SSTL_PANIC when SSTL_ON_ERROR is unset.
#endif

#ifdef SSTL_PANIC_HOOK_DEFINED
# error Disabling the default panic hook must leave hook ownership to the application.
#endif

static jmp_buf g_panic_env;
static int g_panic_count = 0;

extern "C" void sstl_panic(const char*) {
  ++g_panic_count;
  longjmp(g_panic_env, 1);
}

static void default_debug_panic_dispatch_reaches_application_hook() {
  sstl::vector<int, 1> v;
  g_panic_count = 0;
  if (setjmp(g_panic_env) == 0) {
    (void)v.at(0);
    SSTL_TEST_ASSERT(0 && "default debug policy must route invalid at() through sstl_panic");
  }
  SSTL_TEST_EQ(g_panic_count, 1);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"default_debug_panic_dispatch_reaches_application_hook", default_debug_panic_dispatch_reaches_application_hook}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
