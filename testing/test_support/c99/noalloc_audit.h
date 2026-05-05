/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This helper is a heap-allocation tripwire. In the failing path it aborts immediately; in the passing path the guard proves no allocation hook was reached.
 *   - It is designed for test executables, not as production instrumentation.
 *   - Platform-specific wrappers should be added here as new toolchains are introduced.
 */
#ifndef SSTL_TEST_SUPPORT_C99_NOALLOC_AUDIT_H
#define SSTL_TEST_SUPPORT_C99_NOALLOC_AUDIT_H

#include <stddef.h>
#include "test_harness.h"

static unsigned sstl_c_noalloc_count = 0u;
extern unsigned sstl_c_noalloc_link_count;

static void sstl_c_noalloc_begin(void) {
  sstl_c_noalloc_count = 0u;
  sstl_c_noalloc_link_count = 0u;
}

static void sstl_c_noalloc_end(void) {
  SSTL_C_EQ(sstl_c_noalloc_count, 0u);
  SSTL_C_EQ(sstl_c_noalloc_link_count, 0u);
}

#endif
