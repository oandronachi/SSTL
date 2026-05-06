#define SSTL_EXTERN
#include <sstl/c/sstl_config.h>

int sstl_extern_config_a(void) {
  int x = 1;
  int y = 2;
  return sstl_cmp_int_v(&x, &y);
}
