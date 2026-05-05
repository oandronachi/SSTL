/* test_id: c99.vector.extended_ops */
/* api_ids: [c.vector.resize, c.vector.insert, c.vector.erase] */
/* req_ids: [REQ-03, REQ-05, REQ-06, REQ-07] */
/* patterns: [AUDIT-NOALLOC, CAPACITY-BOUNDARY, C-API-PARITY] */
/*
 * Notes:
 *   - The C vector resize takes an explicit fill value because C has no generic default constructor.
 *   - Insert and erase use indexes rather than iterators, matching the generated C macro style.
 *   - Every failed capacity operation must leave size and existing values unchanged.
 */
#include <sstl/c/sstl_vector.h>
#include "test_harness.h"
#include "noalloc_audit.h"

SSTL_VECTOR_DECLARE(c_ext_vec, int, 3)
SSTL_VECTOR_DEFINE(c_ext_vec, int, 3)

static void resize_insert_and_erase_are_capacity_checked(void) {
  c_ext_vec v;
  int out = 0;
  sstl_c_noalloc_begin();
  c_ext_vec_init(&v);
  SSTL_C_ASSERT(c_ext_vec_resize(&v, 2u, 7));
  SSTL_C_EQ(c_ext_vec_size(&v), 2u);
  SSTL_C_EQ(v.data[0], 7);
  SSTL_C_ASSERT(c_ext_vec_insert(&v, 1u, 9));
  SSTL_C_EQ(v.data[1], 9);
  SSTL_C_ASSERT(!c_ext_vec_insert(&v, 0u, 1));
  SSTL_C_ASSERT(c_ext_vec_erase(&v, 1u, &out));
  SSTL_C_EQ(out, 9);
  SSTL_C_EQ(c_ext_vec_size(&v), 2u);
  SSTL_C_ASSERT(!c_ext_vec_resize(&v, 4u, 0));
  SSTL_C_EQ(c_ext_vec_size(&v), 2u);
  sstl_c_noalloc_end();
}

int main(void) {
  const sstl_c_test_case tests[] = {
    {"resize_insert_and_erase_are_capacity_checked", resize_insert_and_erase_are_capacity_checked}
  };
  return sstl_c_run_all(tests, (int)(sizeof(tests) / sizeof(tests[0])));
}
