/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - The PRNG is deterministic by design. Reproducibility is more important than statistical quality for these tests.
 *   - Changing constants or seeds changes replay semantics and should be treated like changing test data.
 *   - Use it only for tests where the seed is recorded in metadata or the source itself.
 */
#ifndef SSTL_TEST_SUPPORT_COMMON_PRNG_HPP
#define SSTL_TEST_SUPPORT_COMMON_PRNG_HPP

namespace sstl_test {

struct prng {
  unsigned state;
  explicit prng(unsigned seed) : state(seed) {}

  unsigned next() {
    state = state * 1664525u + 1013904223u;
    return state;
  }

  int bounded(int n) {
    return static_cast<int>(next() % static_cast<unsigned>(n));
  }
};

} // namespace sstl_test

#endif
