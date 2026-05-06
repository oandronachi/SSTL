#define SSTL_EXTERN
#include <sstl/c/sstl_config.h>

int sstl_extern_config_b(void) {
  unsigned x = 3u;
  unsigned y = 3u;
  return sstl_eq_uint_v(&x, &y) ? 0 : 1;
}
