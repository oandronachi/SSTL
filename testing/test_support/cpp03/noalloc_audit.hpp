/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This helper is a heap-allocation tripwire. In the failing path it exits immediately; in the passing path the guard proves no allocation hook was reached.
 *   - It is designed for test executables, not as production instrumentation.
 *   - Platform-specific wrappers should be added here as new toolchains are introduced.
 */
#ifndef SSTL_TEST_SUPPORT_CPP03_NOALLOC_AUDIT_HPP
#define SSTL_TEST_SUPPORT_CPP03_NOALLOC_AUDIT_HPP

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <new>
#include "test_harness.hpp"

namespace sstl_test {

struct noalloc_counter {
  static unsigned& count() {
    static unsigned c = 0u;
    return c;
  }
};

struct noalloc_guard {
  unsigned start;
  noalloc_guard() : start(noalloc_counter::count()) {}
  ~noalloc_guard() {
    SSTL_TEST_EQ(noalloc_counter::count(), start);
  }
};

} // namespace sstl_test

/*
  These global allocation operators are intentional tripwires. If SSTL calls
  allocating new, the process exits at the call site. The guard's count check
  verifies the non-failing path; it is not meant to recover after allocation.
*/
#if !defined(SSTL_TEST_DISABLE_CPP_NOALLOC_AUDIT)
void* operator new(std::size_t) throw(std::bad_alloc) {
  ++::sstl_test::noalloc_counter::count();
  std::fprintf(stderr, "unexpected heap allocation through operator new\n");
  std::exit(100);
}

void* operator new[](std::size_t) throw(std::bad_alloc) {
  ++::sstl_test::noalloc_counter::count();
  std::fprintf(stderr, "unexpected heap allocation through operator new[]\n");
  std::exit(100);
}

void operator delete(void*) throw() {}
void operator delete[](void*) throw() {}
#endif

#endif
