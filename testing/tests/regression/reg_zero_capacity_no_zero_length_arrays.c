/* test_id: regression.zero_capacity.no_zero_length_arrays */
/* api_ids: [c.vector.push_back, c.string.push_back] */
/* req_ids: [REQ-01, REQ-06] */
/* patterns: [STATIC-LAYOUT, CAPACITY-BOUNDARY] */
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - It focuses on fixed-capacity behavior: size must never exceed capacity, failed mutations must leave observable state unchanged, and zero-capacity types must be real public types.
 *   - The boundary values exercise the SSTL capacity contract: 0, 1, 2, 16, and where practical 1024.
 *   - Failed insertions are expected behavior under SSTL_RETURN, not test setup failures.
 */
#include <sstl/c/sstl_vector.h>
#include <sstl/c/sstl_string.h>

SSTL_VECTOR_DECLARE(reg_zero_vec, int, 0)
SSTL_VECTOR_DEFINE(reg_zero_vec, int, 0)
SSTL_STRING_DECLARE(reg_zero_string, 0)
SSTL_STRING_DEFINE(reg_zero_string, 0)

typedef char reg_zero_vec_must_have_positive_size[(sizeof(reg_zero_vec) > 0u) ? 1 : -1];
typedef char reg_zero_string_must_have_positive_size[(sizeof(reg_zero_string) > 0u) ? 1 : -1];

int main(void) {
  reg_zero_vec v;
  reg_zero_string s;
  reg_zero_vec_init(&v);
  reg_zero_string_init(&s);
  return (reg_zero_vec_capacity(&v) == 0u && reg_zero_string_capacity(&s) == 0u) ? 0 : 1;
}
