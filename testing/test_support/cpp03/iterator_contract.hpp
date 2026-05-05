/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This header defines the contract between invalidation tests and implementation debug iterators.
 *   - It avoids undefined behavior by asking the implementation whether an iterator is still valid instead of using the iterator after invalidation.
 *   - If an implementation cannot provide the hook, keep this test disabled rather than weakening it to a no-op.
 */
#ifndef SSTL_TEST_SUPPORT_CPP03_ITERATOR_CONTRACT_HPP
#define SSTL_TEST_SUPPORT_CPP03_ITERATOR_CONTRACT_HPP

#include "test_harness.hpp"

/*
  Iterator invalidation cannot be verified portably by dereferencing an
  invalidated iterator; that would test via undefined behavior. SSTL
  implementations should expose one of these test-only hooks in debug builds:

    - define SSTL_TEST_ITERATOR_IS_VALID(container, iterator)
    - or provide iterator.is_valid_for(container)

  Release builds may omit these hooks, but the ITER-INVALIDATION lane must be
  run with them enabled to satisfy the §7.1 pattern.
*/
#ifndef SSTL_TEST_ITERATOR_IS_VALID
# define SSTL_TEST_ITERATOR_IS_VALID(container, iterator) ((iterator).is_valid_for(container))
#endif

#define SSTL_TEST_EXPECT_VALID(container, iterator) \
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID((container), (iterator)))

#define SSTL_TEST_EXPECT_INVALIDATED(container, iterator) \
  SSTL_TEST_ASSERT(!SSTL_TEST_ITERATOR_IS_VALID((container), (iterator)))

#endif
