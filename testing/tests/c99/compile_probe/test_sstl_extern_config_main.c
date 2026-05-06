int sstl_extern_config_a(void);
int sstl_extern_config_b(void);

int main(void) {
  return (sstl_extern_config_a() < 0 && sstl_extern_config_b() == 0) ? 0 : 1;
}
