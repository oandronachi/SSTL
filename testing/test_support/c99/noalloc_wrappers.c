/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This helper is a heap-allocation tripwire. In the failing path it aborts immediately; in the passing path the guard proves no allocation hook was reached.
 *   - It is designed for test executables, not as production instrumentation.
 *   - Platform-specific wrappers should be added here as new toolchains are introduced.
 */
#include <stddef.h>

unsigned sstl_c_noalloc_link_count = 0u;

void* __wrap_malloc(size_t n) {
  (void)n;
  ++sstl_c_noalloc_link_count;
  return 0;
}

void* __wrap_calloc(size_t n, size_t s) {
  (void)n;
  (void)s;
  ++sstl_c_noalloc_link_count;
  return 0;
}

void* __wrap_realloc(void* p, size_t n) {
  (void)p;
  (void)n;
  ++sstl_c_noalloc_link_count;
  return 0;
}

void* __wrap_aligned_alloc(size_t a, size_t n) {
  (void)a;
  (void)n;
  ++sstl_c_noalloc_link_count;
  return 0;
}

int __wrap_posix_memalign(void** p, size_t a, size_t n) {
  (void)a;
  (void)n;
  if (p) *p = 0;
  ++sstl_c_noalloc_link_count;
  return -1;
}

void* __wrap_memalign(size_t a, size_t n) {
  (void)a;
  (void)n;
  ++sstl_c_noalloc_link_count;
  return 0;
}

void __wrap_free(void* p) {
  (void)p;
  ++sstl_c_noalloc_link_count;
}
