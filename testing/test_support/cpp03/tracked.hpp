/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - Tracked is deliberately small and noisy: each special member updates counters so lifetime bugs become ordinary assertions.
 *   - It is a single-translation-unit helper; move static definitions to one .cpp if reused in multi-TU binaries.
 *   - Do not make it allocate or throw; that would obscure the SSTL lifetime contract being tested.
 */
#ifndef SSTL_TEST_SUPPORT_CPP03_TRACKED_HPP
#define SSTL_TEST_SUPPORT_CPP03_TRACKED_HPP

namespace sstl_test {

struct tracked {
  static int default_ctor;
  static int copy_ctor;
  static int dtor;
  static int assign;
  static int live;

  int value;

  tracked() : value(0) {
    ++default_ctor;
    ++live;
  }

  explicit tracked(int v) : value(v) {
    ++default_ctor;
    ++live;
  }

  tracked(const tracked& other) : value(other.value) {
    ++copy_ctor;
    ++live;
  }

  ~tracked() {
    --live;
    ++dtor;
  }

  tracked& operator=(const tracked& other) {
    value = other.value;
    ++assign;
    return *this;
  }

  bool operator==(const tracked& other) const { return value == other.value; }
  bool operator<(const tracked& other) const { return value < other.value; }

  static void reset() {
    default_ctor = 0;
    copy_ctor = 0;
    dtor = 0;
    assign = 0;
    live = 0;
  }
};

inline int& tracked_default_ctor_ref() { static int v = 0; return v; }

} // namespace sstl_test

int sstl_test::tracked::default_ctor = 0;
int sstl_test::tracked::copy_ctor = 0;
int sstl_test::tracked::dtor = 0;
int sstl_test::tracked::assign = 0;
int sstl_test::tracked::live = 0;

#endif
