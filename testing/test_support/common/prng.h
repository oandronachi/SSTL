/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - The PRNG is deterministic by design. Reproducibility is more important than statistical quality for these tests.
 *   - Changing constants or seeds changes replay semantics and should be treated like changing test data.
 *   - Use it only for tests where the seed is recorded in metadata or the source itself.
 */
#ifndef SSTL_TEST_SUPPORT_COMMON_PRNG_H
#define SSTL_TEST_SUPPORT_COMMON_PRNG_H

typedef struct sstl_c_prng {
  unsigned state;
} sstl_c_prng;

static unsigned sstl_c_prng_next(sstl_c_prng* p) {
  p->state = p->state * 1664525u + 1013904223u;
  return p->state;
}

static int sstl_c_prng_bounded(sstl_c_prng* p, int n) {
  return (int)(sstl_c_prng_next(p) % (unsigned)n);
}

#endif
