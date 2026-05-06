// test_id: runtime.interface.side_by_side
// api_ids: [c.vector.parity, cpp.vector.parity, c.string.push_back, cpp.string.push_back, c.array.contract, cpp.array.contract, c.span.contract, cpp.span.contract, c.string_view.contract, cpp.string_view.contract, c.deque.contract, cpp.deque.contract, c.queue.contract, cpp.queue.contract, c.stack.contract, cpp.stack.contract, c.priority_queue.contract, cpp.priority_queue.contract, c.list.contract, cpp.list.lifetime, c.forward_list.contract, cpp.forward_list.lifetime, c.map.contract, cpp.map.contract, c.set.contract, cpp.set.contract, c.flat_map.contract, cpp.flat_map.contract, c.flat_set.contract, cpp.flat_set.contract, c.unordered_map.contract, cpp.unordered_map.contract, c.unordered_set.contract, cpp.unordered_set.contract, c.bitset.contract, cpp.bitset.contract, c.optional.contract, cpp.optional.contract, c.variant.contract, cpp.variant.contract, c.function.contract, cpp.function.contract, c.algorithm.contract, cpp.algorithm.contract]
// req_ids: [REQ-RUNTIME-COMPARISON]
// patterns: [RUNTIME-COMPARISON, API-PARITY, MEMORY-FOOTPRINT, CPU-FOOTPRINT]
/*
 * Notes:
 *   - This test intentionally compares public interfaces rather than private implementation details.
 *   - The goal is not to enforce a universal "fastest" winner because host CPU, compiler, optimization level,
 *     STL implementation, allocator, and sanitizer settings can all change timing numbers substantially.
 *   - The goal is to make size and runtime differences visible with a large enough footprint that accidental
 *     heap use, unexpected object bloat, or severe algorithmic drift is easy for a human reader to spot.
 *   - The C SSTL and C++ SSTL lanes use fixed-capacity objects.  The STL lanes reserve equivalent capacity
 *     so the measured storage size is comparable to the fixed-capacity footprints.
 *   - The printed diff percentages are relative to the first row in each comparison group.  Very small
 *     baseline footprints, such as 16-byte string views, can make mathematically correct memory percentages
 *     look extremely large when a heap-backed STL proxy row is compared against them.
 *   - The timing loops include ordinary public API calls only: initialization/clear, push-style appends,
 *     indexed reads, and pop-style removals where that operation is available on the interface.
 */
extern "C" {
#include <sstl/c/sstl_algorithm.h>
#include <sstl/c/sstl_array.h>
#include <sstl/c/sstl_bitset.h>
#include <sstl/c/sstl_deque.h>
#include <sstl/c/sstl_queue.h>
#include <sstl/c/sstl_stack.h>
#include <sstl/c/sstl_priority_queue.h>
#include <sstl/c/sstl_flat_map.h>
#include <sstl/c/sstl_flat_set.h>
#include <sstl/c/sstl_forward_list.h>
#include <sstl/c/sstl_function.h>
#include <sstl/c/sstl_list.h>
#include <sstl/c/sstl_map.h>
#include <sstl/c/sstl_optional.h>
#include <sstl/c/sstl_set.h>
#include <sstl/c/sstl_span.h>
#include <sstl/c/sstl_string.h>
#include <sstl/c/sstl_string_view.h>
#include <sstl/c/sstl_unordered_map.h>
#include <sstl/c/sstl_unordered_set.h>
#include <sstl/c/sstl_variant.h>
#include <sstl/c/sstl_vector.h>
}

#include <sstl/algorithm.hpp>
#include <sstl/array.hpp>
#include <sstl/string.hpp>
#include <sstl/string_view.hpp>
#include <sstl/span.hpp>
#include <sstl/deque.hpp>
#include <sstl/forward_list.hpp>
#include <sstl/list.hpp>
#include <sstl/map.hpp>
#include <sstl/set.hpp>
#include <sstl/flat_map.hpp>
#include <sstl/flat_set.hpp>
#include <sstl/unordered_map.hpp>
#include <sstl/unordered_set.hpp>
#include <sstl/bitset.hpp>
#include <sstl/optional.hpp>
#include <sstl/variant.hpp>
#include <sstl/inplace_function.hpp>
#include <sstl/stack.hpp>
#include <sstl/queue.hpp>
#include <sstl/priority_queue.hpp>
#include <sstl/vector.hpp>

#include "test_harness.hpp"

#include <bitset>
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <deque>
#include <functional>
#include <list>
#include <map>
#include <new>
#include <numeric>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace {

/*
 * The vector payload is deliberately wider than an int.  A wide record makes
 * inline static storage and std::vector heap storage large enough to inspect
 * in a normal test log without needing millions of elements.
 */
struct runtime_record {
  int id;
  unsigned tag;
  char payload[24];
};

enum {
  RUNTIME_ARRAY_CAP = 65536u,
  RUNTIME_VECTOR_CAP = 262144u,
  RUNTIME_VECTOR_CYCLES = 4u,
  RUNTIME_STRING_CAP = 1048576u,
  RUNTIME_STRING_CYCLES = 4u,
  RUNTIME_NODE_CAP = 4096u,
  RUNTIME_NODE_CYCLES = 4u,
  RUNTIME_ASSOC_CAP = 192u,
  RUNTIME_ASSOC_CYCLES = 2u,
  RUNTIME_ASSOC_BUCKETS = 257u,
  RUNTIME_BITSET_BITS = 1048576u,
  RUNTIME_ADAPTOR_CAP = 65536u,
  RUNTIME_ADAPTOR_CYCLES = 4u,
  RUNTIME_UTILITY_CYCLES = 65536u,
  RUNTIME_ALGORITHM_CAP = 32768u,
  RUNTIME_LANE_SECONDS = 15u
};

static int runtime_record_cmp_ptr(const void* a, const void* b);

SSTL_ARRAY_DECLARE(runtime_c_array, runtime_record, RUNTIME_ARRAY_CAP)
SSTL_ARRAY_DEFINE(runtime_c_array, runtime_record, RUNTIME_ARRAY_CAP)

SSTL_VECTOR_DECLARE(runtime_c_vector, runtime_record, RUNTIME_VECTOR_CAP)
SSTL_VECTOR_DEFINE(runtime_c_vector, runtime_record, RUNTIME_VECTOR_CAP)

SSTL_STRING_DECLARE(runtime_c_string, RUNTIME_STRING_CAP)
SSTL_STRING_DEFINE(runtime_c_string, RUNTIME_STRING_CAP)

SSTL_STRING_VIEW_DECLARE(runtime_c_string_view)
SSTL_STRING_VIEW_DEFINE(runtime_c_string_view)

SSTL_SPAN_DECLARE(runtime_c_span, runtime_record)
SSTL_SPAN_DEFINE(runtime_c_span, runtime_record)

SSTL_DEQUE_DECLARE(runtime_c_deque, runtime_record, RUNTIME_VECTOR_CAP)
SSTL_DEQUE_DEFINE(runtime_c_deque, runtime_record, RUNTIME_VECTOR_CAP)

SSTL_QUEUE_DECLARE(runtime_c_queue, runtime_record, RUNTIME_ADAPTOR_CAP)
SSTL_QUEUE_DEFINE(runtime_c_queue, runtime_record, RUNTIME_ADAPTOR_CAP)

SSTL_STACK_DECLARE(runtime_c_stack, runtime_record, RUNTIME_ADAPTOR_CAP)
SSTL_STACK_DEFINE(runtime_c_stack, runtime_record, RUNTIME_ADAPTOR_CAP)

SSTL_PRIORITY_QUEUE_DECLARE(runtime_c_priority_queue, runtime_record, RUNTIME_ADAPTOR_CAP, runtime_record_cmp_ptr)
SSTL_PRIORITY_QUEUE_DEFINE(runtime_c_priority_queue, runtime_record, RUNTIME_ADAPTOR_CAP, runtime_record_cmp_ptr)

SSTL_LIST_DECLARE(runtime_c_list, runtime_record, RUNTIME_NODE_CAP)
SSTL_LIST_DEFINE(runtime_c_list, runtime_record, RUNTIME_NODE_CAP)

SSTL_FORWARD_LIST_DECLARE(runtime_c_forward_list, runtime_record, RUNTIME_NODE_CAP)
SSTL_FORWARD_LIST_DEFINE(runtime_c_forward_list, runtime_record, RUNTIME_NODE_CAP)

SSTL_MAP_DECLARE(runtime_c_map, int, runtime_record, RUNTIME_ASSOC_CAP, sstl_cmp_int_v)
SSTL_MAP_DEFINE(runtime_c_map, int, runtime_record, RUNTIME_ASSOC_CAP, sstl_cmp_int_v)

SSTL_SET_DECLARE(runtime_c_set, int, RUNTIME_ASSOC_CAP, sstl_cmp_int_v)
SSTL_SET_DEFINE(runtime_c_set, int, RUNTIME_ASSOC_CAP, sstl_cmp_int_v)

SSTL_FLAT_MAP_DECLARE(runtime_c_flat_map, int, runtime_record, RUNTIME_ASSOC_CAP, sstl_cmp_int_v)
SSTL_FLAT_MAP_DEFINE(runtime_c_flat_map, int, runtime_record, RUNTIME_ASSOC_CAP, sstl_cmp_int_v)

SSTL_FLAT_SET_DECLARE(runtime_c_flat_set, int, RUNTIME_ASSOC_CAP, sstl_cmp_int_v)
SSTL_FLAT_SET_DEFINE(runtime_c_flat_set, int, RUNTIME_ASSOC_CAP, sstl_cmp_int_v)

SSTL_UNORDERED_MAP_DECLARE(runtime_c_unordered_map, int, runtime_record, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS, sstl_eq_int_v, sstl_hash_int_v)
SSTL_UNORDERED_MAP_DEFINE(runtime_c_unordered_map, int, runtime_record, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS, sstl_eq_int_v, sstl_hash_int_v)

SSTL_UNORDERED_SET_DECLARE(runtime_c_unordered_set, int, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS, sstl_eq_int_v, sstl_hash_int_v)
SSTL_UNORDERED_SET_DEFINE(runtime_c_unordered_set, int, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS, sstl_eq_int_v, sstl_hash_int_v)

SSTL_BITSET_DECLARE(runtime_c_bitset, RUNTIME_BITSET_BITS)
SSTL_BITSET_DEFINE(runtime_c_bitset, RUNTIME_BITSET_BITS)

SSTL_OPTIONAL_DECLARE(runtime_c_optional, runtime_record)
SSTL_OPTIONAL_DEFINE(runtime_c_optional, runtime_record)

SSTL_VARIANT2_DECLARE(runtime_c_variant, runtime_record, int)
SSTL_VARIANT2_DEFINE(runtime_c_variant, runtime_record, int)

SSTL_FUNCTION1_DECLARE(runtime_c_function1, unsigned long, unsigned)
SSTL_FUNCTION1_DEFINE(runtime_c_function1, unsigned long, unsigned)

/*
 * Keep the fixed-capacity SSTL objects in static storage.  Their purpose is to
 * demonstrate object footprint, and placing them on the stack would make the
 * test sensitive to the host's stack limit rather than the container designs.
 */
static runtime_c_array g_c_array;
static sstl::array<runtime_record, RUNTIME_ARRAY_CAP> g_cpp_array;
static runtime_record g_stl_array_proxy[RUNTIME_ARRAY_CAP];
static runtime_c_vector g_c_vector;
static sstl::vector<runtime_record, RUNTIME_VECTOR_CAP> g_cpp_vector;
static runtime_c_string g_c_string;
static sstl::string<RUNTIME_STRING_CAP> g_cpp_string;
static runtime_c_span g_c_span;
static sstl::deque<runtime_record, RUNTIME_VECTOR_CAP> g_cpp_deque;
static runtime_c_deque g_c_deque;
static runtime_c_list g_c_list;
static sstl::list<runtime_record, RUNTIME_NODE_CAP> g_cpp_list;
static runtime_c_forward_list g_c_forward_list;
static sstl::forward_list<runtime_record, RUNTIME_NODE_CAP> g_cpp_forward_list;
static runtime_c_map g_c_map;
static sstl::map<int, runtime_record, RUNTIME_ASSOC_CAP> g_cpp_map;
static runtime_c_set g_c_set;
static sstl::set<int, RUNTIME_ASSOC_CAP> g_cpp_set;
static runtime_c_flat_map g_c_flat_map;
static runtime_c_flat_set g_c_flat_set;
static runtime_c_unordered_map g_c_unordered_map;
static runtime_c_unordered_set g_c_unordered_set;
static runtime_c_bitset g_c_bitset;
static sstl::bitset<RUNTIME_BITSET_BITS> g_cpp_bitset;
static runtime_c_optional g_c_optional;
static runtime_c_variant g_c_variant;
static runtime_c_stack g_c_stack;
static sstl::stack<runtime_record, RUNTIME_ADAPTOR_CAP> g_cpp_stack;
static runtime_c_queue g_c_queue;
static sstl::queue<runtime_record, RUNTIME_ADAPTOR_CAP> g_cpp_queue;
static runtime_c_priority_queue g_c_priority_queue;
static int g_c_algorithm_data[RUNTIME_ALGORITHM_CAP];
static int g_cpp_algorithm_data[RUNTIME_ALGORITHM_CAP];
static int g_stl_algorithm_data[RUNTIME_ALGORITHM_CAP];

struct metric {
  const char* name;
  std::size_t object_bytes;
  std::size_t storage_bytes;
  double seconds;
  unsigned long checksum;
};

struct csv_metric_row {
  const char* group;
  const char* name;
  std::size_t object_bytes;
  std::size_t storage_bytes;
  double seconds;
  unsigned long checksum;
  double cpu_diff_percent;
  double memory_diff_percent;
};

enum { RUNTIME_MAX_CSV_ROWS = 96u };

static std::FILE* g_runtime_csv = 0;
static const char* g_runtime_csv_path = 0;
static csv_metric_row g_runtime_csv_rows[RUNTIME_MAX_CSV_ROWS];
static unsigned g_runtime_csv_row_count = 0u;

/*
 * Counting allocators make STL heap use visible without replacing the STL
 * implementation under test.  Each Tag owns one counter set, so vector/string/
 * deque/list/map/set lanes do not contaminate each other's memory reports.
 */
template <class Tag>
struct allocation_counter {
  static std::size_t current;
  static std::size_t peak;
};

template <class Tag> std::size_t allocation_counter<Tag>::current = 0u;
template <class Tag> std::size_t allocation_counter<Tag>::peak = 0u;

template <class Tag>
static void reset_counter() {
  allocation_counter<Tag>::current = 0u;
  allocation_counter<Tag>::peak = 0u;
}

template <class T, class Tag>
class counting_allocator {
public:
  typedef T value_type;
  typedef T* pointer;
  typedef const T* const_pointer;
  typedef T& reference;
  typedef const T& const_reference;
  typedef std::size_t size_type;
  typedef std::ptrdiff_t difference_type;

  template <class U>
  struct rebind { typedef counting_allocator<U, Tag> other; };

  counting_allocator() {}
  template <class U>
  counting_allocator(const counting_allocator<U, Tag>&) {}

  pointer allocate(size_type n, const void* = 0) {
    const size_type bytes = n * sizeof(T);
    allocation_counter<Tag>::current += bytes;
    if (allocation_counter<Tag>::current > allocation_counter<Tag>::peak) {
      allocation_counter<Tag>::peak = allocation_counter<Tag>::current;
    }
    return static_cast<pointer>(::operator new(bytes));
  }

  void deallocate(pointer p, size_type n) {
    allocation_counter<Tag>::current -= n * sizeof(T);
    ::operator delete(p);
  }

  size_type max_size() const { return static_cast<size_type>(-1) / sizeof(T); }
  void construct(pointer p, const T& value) { new (p) T(value); }
  void destroy(pointer p) { p->~T(); }
};

template <class T, class U, class Tag>
bool operator==(const counting_allocator<T, Tag>&, const counting_allocator<U, Tag>&) { return true; }

template <class T, class U, class Tag>
bool operator!=(const counting_allocator<T, Tag>&, const counting_allocator<U, Tag>&) { return false; }

struct vector_alloc_tag {};
struct string_alloc_tag {};
struct deque_alloc_tag {};
struct list_alloc_tag {};
struct map_alloc_tag {};
struct set_alloc_tag {};
struct stack_alloc_tag {};
struct queue_alloc_tag {};
struct priority_queue_alloc_tag {};
struct flat_map_alloc_tag {};
struct flat_set_alloc_tag {};
struct unordered_map_alloc_tag {};
struct unordered_set_alloc_tag {};
struct algorithm_alloc_tag {};

class lane_guard {
public:
  lane_guard(const char* name, unsigned seconds)
    : name_(name), seconds_(seconds), start_(std::time(0)), last_report_(start_) {
    std::printf("[runtime] begin lane: %s (timeout %u sec)\n", name_, seconds_);
    std::fflush(stdout);
  }

  void checkpoint(const char* phase, unsigned done, unsigned total) {
    const std::time_t now = std::time(0);
    const double elapsed = std::difftime(now, start_);
    if (elapsed > static_cast<double>(seconds_)) {
      std::fprintf(stderr,
                   "[runtime][timeout] lane '%s' exceeded %u sec during %s (%u/%u)\n",
                   name_, seconds_, phase, done, total);
      std::fflush(stderr);
      std::exit(1);
    }
    if (now != last_report_) {
      std::printf("[runtime] progress lane: %s phase=%s %u/%u elapsed=%.0f sec\n",
                  name_, phase, done, total, elapsed);
      std::fflush(stdout);
      last_report_ = now;
    }
  }

  void finish() {
    const double elapsed = std::difftime(std::time(0), start_);
    std::printf("[runtime] end lane: %s elapsed=%.0f sec\n", name_, elapsed);
    std::fflush(stdout);
  }

private:
  const char* name_;
  unsigned seconds_;
  std::time_t start_;
  std::time_t last_report_;
};

static runtime_record make_record(unsigned i) {
  runtime_record r;
  r.id = static_cast<int>(i);
  r.tag = i * 2654435761u;
  for (unsigned j = 0; j != sizeof(r.payload); ++j) {
    r.payload[j] = static_cast<char>('A' + ((i + j) % 26u));
  }
  return r;
}

static unsigned long checksum_record(const runtime_record& r) {
  unsigned long sum = static_cast<unsigned long>(r.id) ^ static_cast<unsigned long>(r.tag);
  sum += static_cast<unsigned long>(static_cast<unsigned char>(r.payload[0]));
  sum += static_cast<unsigned long>(static_cast<unsigned char>(r.payload[sizeof(r.payload) - 1u]));
  return sum;
}

static unsigned long runtime_function_value(unsigned x) {
  return static_cast<unsigned long>((x * 1103515245u) + 12345u);
}

static unsigned long runtime_c_function_value(void* ctx, unsigned x) {
  (void)ctx;
  return runtime_function_value(x);
}

static bool runtime_int_less(int a, int b) {
  return a < b;
}

struct runtime_pair_less {
  bool operator()(const std::pair<int, runtime_record>& a,
                  const std::pair<int, runtime_record>& b) const {
    return a.first < b.first;
  }
};

struct runtime_record_priority_less {
  bool operator()(const runtime_record& a, const runtime_record& b) const {
    return a.id < b.id;
  }
};

static int runtime_record_cmp_ptr(const void* a, const void* b) {
  const runtime_record* left = static_cast<const runtime_record*>(a);
  const runtime_record* right = static_cast<const runtime_record*>(b);
  return (left->id > right->id) - (left->id < right->id);
}

static int runtime_qsort_int_compare(const void* a, const void* b) {
  return sstl_cmp_int_v(a, b);
}

static int runtime_algorithm_seed(unsigned i, unsigned cycle) {
  return static_cast<int>(((RUNTIME_ALGORITHM_CAP - i) * 17u + cycle) % RUNTIME_ALGORITHM_CAP);
}

static void exercise_c_generated_vector_surface() {
  /*
   * The C macro declares a whole typed surface, not just the hot operations
   * used by the benchmark loop.  Touch every generated vector function here so
   * strict compilers do not treat the intentionally generated helpers as dead
   * local functions, and so future readers can see the full C interface in one
   * compact place.
   */
  runtime_record first = make_record(1u);
  runtime_record second = make_record(2u);
  runtime_record out = make_record(0u);
  runtime_c_vector_init(&g_c_vector);
  SSTL_TEST_ASSERT(runtime_c_vector_empty(&g_c_vector));
  SSTL_TEST_ASSERT(!runtime_c_vector_full(&g_c_vector));
  SSTL_TEST_EQ(runtime_c_vector_size(&g_c_vector), 0u);
  SSTL_TEST_EQ(runtime_c_vector_capacity(&g_c_vector), static_cast<std::size_t>(RUNTIME_VECTOR_CAP));
  SSTL_TEST_ASSERT(runtime_c_vector_try_push_back(&g_c_vector, first));
  SSTL_TEST_ASSERT(runtime_c_vector_push_back(&g_c_vector, second));
  SSTL_TEST_ASSERT(runtime_c_vector_try_at(&g_c_vector, 0u) != 0);
  SSTL_TEST_ASSERT(runtime_c_vector_at(&g_c_vector, 1u) != 0);
  SSTL_TEST_ASSERT(runtime_c_vector_try_pop_back(&g_c_vector, &out));
  SSTL_TEST_ASSERT(runtime_c_vector_pop_back(&g_c_vector, &out));
  SSTL_TEST_ASSERT(runtime_c_vector_empty(&g_c_vector));
}

static void exercise_c_generated_string_surface() {
  /*
   * The string C API has no pop operation, so this probe focuses on capacity,
   * append, indexed access, and null-terminated view semantics.
   */
  runtime_c_string_init(&g_c_string);
  SSTL_TEST_ASSERT(runtime_c_string_empty(&g_c_string));
  SSTL_TEST_ASSERT(!runtime_c_string_full(&g_c_string));
  SSTL_TEST_EQ(runtime_c_string_size(&g_c_string), 0u);
  SSTL_TEST_EQ(runtime_c_string_capacity(&g_c_string), static_cast<std::size_t>(RUNTIME_STRING_CAP));
  SSTL_TEST_ASSERT(runtime_c_string_push_back(&g_c_string, 'x'));
  SSTL_TEST_ASSERT(runtime_c_string_try_at(&g_c_string, 0u) != 0);
  SSTL_TEST_ASSERT(runtime_c_string_at(&g_c_string, 0u) != 0);
  SSTL_TEST_EQ(runtime_c_string_c_str(&g_c_string)[1], '\0');
}

static double elapsed_seconds(std::clock_t begin, std::clock_t end) {
  return static_cast<double>(end - begin) / static_cast<double>(CLOCKS_PER_SEC);
}

static double percent_delta(double value, double baseline) {
  if (baseline == 0.0) return value == 0.0 ? 0.0 : 100.0;
  return ((value - baseline) * 100.0) / baseline;
}

static const char* runtime_csv_path() {
#if defined(_MSC_VER)
  static char env_path[1024];
  std::size_t required = 0u;
  if (getenv_s(&required, env_path, sizeof(env_path), "SSTL_RUNTIME_CSV") == 0 && required > 1u) {
    return env_path;
  }
  return "sstl_runtime_interface_comparison.csv";
#else
  const char* from_env = std::getenv("SSTL_RUNTIME_CSV");
  return from_env && from_env[0] ? from_env : "sstl_runtime_interface_comparison.csv";
#endif
}

static std::FILE* open_csv_for_write(const char* path) {
#if defined(_MSC_VER)
  std::FILE* f = 0;
  return fopen_s(&f, path, "w") == 0 ? f : 0;
#else
  return std::fopen(path, "w");
#endif
}

static void begin_runtime_csv_capture() {
  g_runtime_csv_path = runtime_csv_path();
  g_runtime_csv_row_count = 0u;
  std::printf("[runtime][csv] collecting rows in RAM; artifact will be written at process end: %s\n",
              g_runtime_csv_path);
}

static void append_runtime_csv_row(const char* group, const metric& row, double cpu_diff, double mem_diff) {
  if (g_runtime_csv_row_count >= RUNTIME_MAX_CSV_ROWS) {
    std::fprintf(stderr, "[runtime][csv] row buffer is full; increase RUNTIME_MAX_CSV_ROWS\n");
    std::fflush(stderr);
    std::exit(1);
  }
  csv_metric_row& out = g_runtime_csv_rows[g_runtime_csv_row_count++];
  out.group = group;
  out.name = row.name;
  out.object_bytes = row.object_bytes;
  out.storage_bytes = row.storage_bytes;
  out.seconds = row.seconds;
  out.checksum = row.checksum;
  out.cpu_diff_percent = cpu_diff;
  out.memory_diff_percent = mem_diff;
}

static void write_runtime_csv_artifact() {
  g_runtime_csv = open_csv_for_write(g_runtime_csv_path);
  if (!g_runtime_csv) {
    std::fprintf(stderr, "[runtime][csv] failed to create CSV artifact: %s\n", g_runtime_csv_path);
    std::fflush(stderr);
    std::exit(1);
  }

  std::fprintf(g_runtime_csv,
               "group,interface,object_bytes,storage_bytes,total_bytes,cpu_seconds,cpu_diff_percent,memory_diff_percent,checksum\n");
  for (unsigned i = 0; i != g_runtime_csv_row_count; ++i) {
    const csv_metric_row& row = g_runtime_csv_rows[i];
    const std::size_t total_bytes = row.object_bytes + row.storage_bytes;
    std::fprintf(g_runtime_csv,
                 "\"%s\",\"%s\",%lu,%lu,%lu,%.9f,%.6f,%.6f,%lu\n",
                 row.group,
                 row.name,
                 static_cast<unsigned long>(row.object_bytes),
                 static_cast<unsigned long>(row.storage_bytes),
                 static_cast<unsigned long>(total_bytes),
                 row.seconds,
                 row.cpu_diff_percent,
                 row.memory_diff_percent,
                 row.checksum);
  }
  std::fflush(g_runtime_csv);
  std::fclose(g_runtime_csv);
  g_runtime_csv = 0;
  std::printf("[runtime][csv] artifact complete: %s (%u rows)\n",
              g_runtime_csv_path ? g_runtime_csv_path : "",
              g_runtime_csv_row_count);
}

static void print_metric_table(const char* title, const metric* metrics, unsigned count) {
  const metric& baseline = metrics[0];
  std::printf("\n[runtime] %s\n", title);
  std::printf("[runtime] %-10s %14s %14s %11s %11s %11s %10s\n",
              "interface", "object-bytes", "storage-bytes", "cpu-sec", "cpu-diff%",
              "mem-diff%", "checksum");
  for (unsigned i = 0; i != count; ++i) {
    const std::size_t total_bytes = metrics[i].object_bytes + metrics[i].storage_bytes;
    const std::size_t baseline_total = baseline.object_bytes + baseline.storage_bytes;
    const double cpu_diff = percent_delta(metrics[i].seconds, baseline.seconds);
    const double mem_diff = percent_delta(static_cast<double>(total_bytes), static_cast<double>(baseline_total));
    std::printf("[runtime] %-10s %14lu %14lu %11.6f %11.2f %11.2f %10lu\n",
                metrics[i].name,
                static_cast<unsigned long>(metrics[i].object_bytes),
                static_cast<unsigned long>(metrics[i].storage_bytes),
                metrics[i].seconds,
                cpu_diff,
                mem_diff,
                metrics[i].checksum);
    append_runtime_csv_row(title, metrics[i], cpu_diff, mem_diff);
  }
}

static metric run_c_array_lane() {
  lane_guard guard("c-sstl array", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ARRAY_CAP; ++i) g_c_array.data[i] = make_record(i + cycle);
    for (unsigned i = 0; i < RUNTIME_ARRAY_CAP; i += 13u) {
      runtime_record* r = runtime_c_array_at(&g_c_array, i);
      SSTL_TEST_ASSERT(r != 0);
      checksum += checksum_record(*r);
    }
    checksum += static_cast<unsigned long>(runtime_c_array_size(&g_c_array));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_array), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_array_lane() {
  lane_guard guard("cpp-sstl array", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ARRAY_CAP; ++i) g_cpp_array[i] = make_record(i + cycle);
    for (unsigned i = 0; i < RUNTIME_ARRAY_CAP; i += 13u) checksum += checksum_record(g_cpp_array[i]);
    checksum += static_cast<unsigned long>(g_cpp_array.size());
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_array), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_array_proxy_lane() {
  lane_guard guard("stl array proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ARRAY_CAP; ++i) g_stl_array_proxy[i] = make_record(i + cycle);
    for (unsigned i = 0; i < RUNTIME_ARRAY_CAP; i += 13u) checksum += checksum_record(g_stl_array_proxy[i]);
    checksum += static_cast<unsigned long>(RUNTIME_ARRAY_CAP);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(runtime_record*) + sizeof(std::size_t), sizeof(g_stl_array_proxy),
                elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_span_lane() {
  lane_guard guard("c-sstl span", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    g_c_span = runtime_c_span_make(g_c_array.data, RUNTIME_ARRAY_CAP);
    for (unsigned i = 0; i < RUNTIME_ARRAY_CAP; i += 11u) {
      runtime_record* r = runtime_c_span_at(&g_c_span, i);
      SSTL_TEST_ASSERT(r != 0);
      checksum += checksum_record(*r);
    }
    checksum += static_cast<unsigned long>(runtime_c_span_size(&g_c_span));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_span), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_span_lane() {
  lane_guard guard("cpp-sstl span", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    sstl::span<runtime_record> sp(g_cpp_array.data(), g_cpp_array.size());
    for (unsigned i = 0; i < RUNTIME_ARRAY_CAP; i += 11u) checksum += checksum_record(sp[i]);
    checksum += static_cast<unsigned long>(sp.size());
  }
  const std::clock_t end = std::clock();
  guard.finish();
  typedef sstl::span<runtime_record> span_type;
  metric out = {"cpp-sstl", sizeof(span_type), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_span_proxy_lane() {
  lane_guard guard("stl span proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  runtime_record* data = g_stl_array_proxy;
  const std::size_t size = RUNTIME_ARRAY_CAP;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i < RUNTIME_ARRAY_CAP; i += 11u) checksum += checksum_record(data[i]);
    checksum += static_cast<unsigned long>(size);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(data) + sizeof(size), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_vector_lane() {
  lane_guard guard("c-sstl vector", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_VECTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_VECTOR_CYCLES);
    runtime_c_vector_init(&g_c_vector);
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP; ++i) {
      const runtime_record r = make_record(i + cycle);
      SSTL_TEST_ASSERT(runtime_c_vector_push_back(&g_c_vector, r));
    }
    for (unsigned i = 0; i < RUNTIME_VECTOR_CAP; i += 17u) {
      runtime_record* r = runtime_c_vector_at(&g_c_vector, i);
      SSTL_TEST_ASSERT(r != 0);
      checksum += checksum_record(*r);
    }
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP / 2u; ++i) {
      runtime_record out;
      SSTL_TEST_ASSERT(runtime_c_vector_pop_back(&g_c_vector, &out));
      checksum += checksum_record(out);
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_vector), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_vector_lane() {
  lane_guard guard("cpp-sstl vector", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_VECTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_VECTOR_CYCLES);
    g_cpp_vector.clear();
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP; ++i) {
      const runtime_record r = make_record(i + cycle);
      SSTL_TEST_ASSERT(g_cpp_vector.push_back(r));
    }
    for (unsigned i = 0; i < RUNTIME_VECTOR_CAP; i += 17u) {
      checksum += checksum_record(g_cpp_vector[i]);
    }
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP / 2u; ++i) {
      runtime_record out;
      SSTL_TEST_ASSERT(g_cpp_vector.try_pop_back(&out));
      checksum += checksum_record(out);
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_vector), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_vector_lane() {
  lane_guard guard("stl vector", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<vector_alloc_tag>();
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_VECTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_VECTOR_CYCLES);
    typedef std::vector<runtime_record, counting_allocator<runtime_record, vector_alloc_tag> > vector_type;
    vector_type v;
    v.reserve(RUNTIME_VECTOR_CAP);
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP; ++i) {
      v.push_back(make_record(i + cycle));
    }
    for (unsigned i = 0; i < RUNTIME_VECTOR_CAP; i += 17u) {
      checksum += checksum_record(v[i]);
    }
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP / 2u; ++i) {
      checksum += checksum_record(v.back());
      v.pop_back();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  typedef std::vector<runtime_record, counting_allocator<runtime_record, vector_alloc_tag> > vector_type;
  metric out = {"stl", sizeof(vector_type), allocation_counter<vector_alloc_tag>::peak,
                elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_string_lane() {
  lane_guard guard("c-sstl string", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    runtime_c_string_init(&g_c_string);
    for (unsigned i = 0; i != RUNTIME_STRING_CAP; ++i) {
      const char c = static_cast<char>('a' + ((i + cycle) % 26u));
      SSTL_TEST_ASSERT(runtime_c_string_push_back(&g_c_string, c));
    }
    for (unsigned i = 0; i < RUNTIME_STRING_CAP; i += 31u) {
      char* p = runtime_c_string_at(&g_c_string, i);
      SSTL_TEST_ASSERT(p != 0);
      checksum += static_cast<unsigned long>(static_cast<unsigned char>(*p));
    }
    checksum += static_cast<unsigned long>(runtime_c_string_size(&g_c_string));
    checksum += static_cast<unsigned long>(runtime_c_string_c_str(&g_c_string)[RUNTIME_STRING_CAP]);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_string), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_string_lane() {
  lane_guard guard("cpp-sstl string", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    g_cpp_string.clear();
    for (unsigned i = 0; i != RUNTIME_STRING_CAP; ++i) {
      const char c = static_cast<char>('a' + ((i + cycle) % 26u));
      SSTL_TEST_ASSERT(g_cpp_string.push_back(c));
    }
    for (unsigned i = 0; i < RUNTIME_STRING_CAP; i += 31u) {
      checksum += static_cast<unsigned long>(static_cast<unsigned char>(g_cpp_string[i]));
    }
    checksum += static_cast<unsigned long>(g_cpp_string.size());
    checksum += static_cast<unsigned long>(g_cpp_string.c_str()[RUNTIME_STRING_CAP]);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_string), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_string_lane() {
  lane_guard guard("stl string", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<string_alloc_tag>();
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    typedef std::basic_string<char, std::char_traits<char>, counting_allocator<char, string_alloc_tag> > string_type;
    string_type s;
    s.reserve(RUNTIME_STRING_CAP);
    for (unsigned i = 0; i != RUNTIME_STRING_CAP; ++i) {
      const char c = static_cast<char>('a' + ((i + cycle) % 26u));
      s.push_back(c);
    }
    for (unsigned i = 0; i < RUNTIME_STRING_CAP; i += 31u) {
      checksum += static_cast<unsigned long>(static_cast<unsigned char>(s[i]));
    }
    checksum += static_cast<unsigned long>(s.size());
    checksum += static_cast<unsigned long>(s.c_str()[RUNTIME_STRING_CAP]);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  typedef std::basic_string<char, std::char_traits<char>, counting_allocator<char, string_alloc_tag> > string_type;
  metric out = {"stl", sizeof(string_type), allocation_counter<string_alloc_tag>::peak, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_deque_lane() {
  lane_guard guard("c-sstl deque", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_VECTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_VECTOR_CYCLES);
    runtime_c_deque_init(&g_c_deque);
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP; ++i) {
      SSTL_TEST_ASSERT(runtime_c_deque_push_back(&g_c_deque, make_record(i + cycle)));
    }
    for (unsigned i = 0; i < RUNTIME_VECTOR_CAP; i += 19u) {
      runtime_record* r = runtime_c_deque_at(&g_c_deque, i);
      SSTL_TEST_ASSERT(r != 0);
      checksum += checksum_record(*r);
    }
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP / 2u; ++i) {
      runtime_record out;
      SSTL_TEST_ASSERT(runtime_c_deque_pop_back(&g_c_deque, &out));
      checksum += checksum_record(out);
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_deque), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_deque_lane() {
  lane_guard guard("cpp-sstl deque", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_VECTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_VECTOR_CYCLES);
    g_cpp_deque.clear();
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP; ++i) {
      SSTL_TEST_ASSERT(g_cpp_deque.push_back(make_record(i + cycle)));
    }
    for (unsigned i = 0; i < RUNTIME_VECTOR_CAP; i += 19u) {
      checksum += checksum_record(g_cpp_deque[i]);
    }
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP / 2u; ++i) {
      runtime_record out;
      SSTL_TEST_ASSERT(g_cpp_deque.try_pop_back(&out));
      checksum += checksum_record(out);
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_deque), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_deque_lane() {
  lane_guard guard("stl deque", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<deque_alloc_tag>();
  typedef std::deque<runtime_record, counting_allocator<runtime_record, deque_alloc_tag> > deque_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_VECTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_VECTOR_CYCLES);
    deque_type d;
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP; ++i) {
      d.push_back(make_record(i + cycle));
    }
    for (unsigned i = 0; i < RUNTIME_VECTOR_CAP; i += 19u) {
      checksum += checksum_record(d[i]);
    }
    for (unsigned i = 0; i != RUNTIME_VECTOR_CAP / 2u; ++i) {
      checksum += checksum_record(d.back());
      d.pop_back();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(deque_type), allocation_counter<deque_alloc_tag>::peak, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_list_lane() {
  lane_guard guard("c-sstl list", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_NODE_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_NODE_CYCLES);
    runtime_c_list_init(&g_c_list);
    for (unsigned i = 0; i != RUNTIME_NODE_CAP; ++i) {
      SSTL_TEST_ASSERT(runtime_c_list_push_back(&g_c_list, make_record(i + cycle)));
    }
    for (runtime_c_list_iterator it = runtime_c_list_begin(&g_c_list);
         it != runtime_c_list_end(&g_c_list);
         it = runtime_c_list_next(&g_c_list, it)) {
      runtime_record* value = runtime_c_list_at(&g_c_list, it);
      SSTL_TEST_ASSERT(value != 0);
      checksum += checksum_record(*value);
    }
    while (!runtime_c_list_empty(&g_c_list)) {
      runtime_record out;
      SSTL_TEST_ASSERT(runtime_c_list_pop_front(&g_c_list, &out));
      checksum += checksum_record(out);
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_list), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_list_lane() {
  lane_guard guard("cpp-sstl list", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_NODE_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_NODE_CYCLES);
    g_cpp_list.clear();
    for (unsigned i = 0; i != RUNTIME_NODE_CAP; ++i) {
      SSTL_TEST_ASSERT(g_cpp_list.push_back(make_record(i + cycle)));
    }
    for (sstl::list<runtime_record, RUNTIME_NODE_CAP>::iterator it = g_cpp_list.begin(); it != g_cpp_list.end(); ++it) {
      checksum += checksum_record(*it);
    }
    while (!g_cpp_list.empty()) {
      runtime_record out;
      SSTL_TEST_ASSERT(g_cpp_list.try_pop_front(&out));
      checksum += checksum_record(out);
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_list), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_forward_list_lane() {
  lane_guard guard("c-sstl forward_list", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_NODE_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_NODE_CYCLES);
    runtime_c_forward_list_init(&g_c_forward_list);
    for (unsigned i = 0; i != RUNTIME_NODE_CAP; ++i) {
      SSTL_TEST_ASSERT(runtime_c_forward_list_push_front(&g_c_forward_list, make_record(RUNTIME_NODE_CAP - 1u - i + cycle)));
    }
    for (runtime_c_forward_list_iterator it = runtime_c_forward_list_begin(&g_c_forward_list);
         it != runtime_c_forward_list_end(&g_c_forward_list);
         it = runtime_c_forward_list_next(&g_c_forward_list, it)) {
      checksum += checksum_record(*runtime_c_forward_list_at(&g_c_forward_list, it));
    }
    while (!runtime_c_forward_list_empty(&g_c_forward_list)) {
      runtime_record out;
      SSTL_TEST_ASSERT(runtime_c_forward_list_pop_front(&g_c_forward_list, &out));
      checksum += checksum_record(out);
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_forward_list), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_forward_list_lane() {
  lane_guard guard("cpp-sstl forward_list", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_NODE_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_NODE_CYCLES);
    g_cpp_forward_list.clear();
    for (unsigned i = 0; i != RUNTIME_NODE_CAP; ++i) {
      SSTL_TEST_ASSERT(g_cpp_forward_list.insert_after(g_cpp_forward_list.before_begin(), make_record(RUNTIME_NODE_CAP - 1u - i + cycle)) != g_cpp_forward_list.end());
    }
    for (sstl::forward_list<runtime_record, RUNTIME_NODE_CAP>::iterator it = g_cpp_forward_list.begin(); it != g_cpp_forward_list.end(); ++it) {
      checksum += checksum_record(*it);
    }
    while (!g_cpp_forward_list.empty()) {
      checksum += checksum_record(*g_cpp_forward_list.begin());
      g_cpp_forward_list.erase_after(g_cpp_forward_list.before_begin());
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_forward_list), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_forward_list_proxy_lane() {
  lane_guard guard("stl forward_list proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<list_alloc_tag>();
  typedef std::list<runtime_record, counting_allocator<runtime_record, list_alloc_tag> > list_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_NODE_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_NODE_CYCLES);
    list_type l;
    for (unsigned i = 0; i != RUNTIME_NODE_CAP; ++i) {
      l.push_front(make_record(RUNTIME_NODE_CAP - 1u - i + cycle));
    }
    for (list_type::iterator it = l.begin(); it != l.end(); ++it) checksum += checksum_record(*it);
    while (!l.empty()) {
      checksum += checksum_record(l.front());
      l.pop_front();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(list_type), allocation_counter<list_alloc_tag>::peak, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_list_lane() {
  lane_guard guard("stl list", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<list_alloc_tag>();
  typedef std::list<runtime_record, counting_allocator<runtime_record, list_alloc_tag> > list_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_NODE_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_NODE_CYCLES);
    list_type l;
    for (unsigned i = 0; i != RUNTIME_NODE_CAP; ++i) {
      l.push_back(make_record(i + cycle));
    }
    for (list_type::iterator it = l.begin(); it != l.end(); ++it) {
      checksum += checksum_record(*it);
    }
    while (!l.empty()) {
      checksum += checksum_record(l.front());
      l.pop_front();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(list_type), allocation_counter<list_alloc_tag>::peak, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_map_lane() {
  lane_guard guard("c-sstl map", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    runtime_c_map_init(&g_c_map);
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(runtime_c_map_insert(&g_c_map, key, make_record(i + cycle)));
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      runtime_record* found = runtime_c_map_find(&g_c_map, key);
      SSTL_TEST_ASSERT(found != 0);
      checksum += checksum_record(*found);
    }
    checksum += static_cast<unsigned long>(runtime_c_map_size(&g_c_map));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_map), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_map_lane() {
  lane_guard guard("cpp-sstl map", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    sstl::map<int, runtime_record, RUNTIME_ASSOC_CAP> m;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      if ((i & 15u) == 0u) guard.checkpoint("insert", i, RUNTIME_ASSOC_CAP);
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(m.insert(sstl::make_pair(key, make_record(i + cycle))).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      guard.checkpoint("find", i, RUNTIME_ASSOC_CAP);
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      sstl::map<int, runtime_record, RUNTIME_ASSOC_CAP>::iterator it = m.find(key);
      SSTL_TEST_ASSERT(it != m.end());
      checksum += checksum_record(it->second);
    }
    checksum += m.size();
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_map), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_map_lane() {
  lane_guard guard("stl map", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<map_alloc_tag>();
  typedef std::pair<const int, runtime_record> stl_pair_type;
  typedef std::map<int, runtime_record, std::less<int>, counting_allocator<stl_pair_type, map_alloc_tag> > map_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    map_type m;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      if ((i & 15u) == 0u) guard.checkpoint("insert", i, RUNTIME_ASSOC_CAP);
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(m.insert(stl_pair_type(key, make_record(i + cycle))).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      guard.checkpoint("find", i, RUNTIME_ASSOC_CAP);
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      map_type::iterator it = m.find(key);
      SSTL_TEST_ASSERT(it != m.end());
      checksum += checksum_record(it->second);
    }
    checksum += static_cast<unsigned long>(m.size());
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(map_type), allocation_counter<map_alloc_tag>::peak, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_set_lane() {
  lane_guard guard("c-sstl set", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    runtime_c_set_init(&g_c_set);
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(runtime_c_set_insert(&g_c_set, key));
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      int* found = runtime_c_set_find(&g_c_set, key);
      SSTL_TEST_ASSERT(found != 0);
      checksum += static_cast<unsigned long>(*found);
    }
    checksum += cycle;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_set), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_set_lane() {
  lane_guard guard("cpp-sstl set", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    sstl::set<int, RUNTIME_ASSOC_CAP> s;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      if ((i & 15u) == 0u) guard.checkpoint("insert", i, RUNTIME_ASSOC_CAP);
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(s.insert(key).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      guard.checkpoint("find", i, RUNTIME_ASSOC_CAP);
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      sstl::set<int, RUNTIME_ASSOC_CAP>::iterator it = s.find(key);
      SSTL_TEST_ASSERT(it != s.end());
      checksum += static_cast<unsigned long>(*it);
    }
    checksum += cycle;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_set), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_set_lane() {
  lane_guard guard("stl set", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<set_alloc_tag>();
  typedef std::set<int, std::less<int>, counting_allocator<int, set_alloc_tag> > set_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    set_type s;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      if ((i & 15u) == 0u) guard.checkpoint("insert", i, RUNTIME_ASSOC_CAP);
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(s.insert(key).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      guard.checkpoint("find", i, RUNTIME_ASSOC_CAP);
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      set_type::iterator it = s.find(key);
      SSTL_TEST_ASSERT(it != s.end());
      checksum += static_cast<unsigned long>(*it);
    }
    checksum += cycle;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(set_type), allocation_counter<set_alloc_tag>::peak, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_flat_map_lane() {
  lane_guard guard("c-sstl flat_map", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    runtime_c_flat_map_init(&g_c_flat_map);
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(runtime_c_flat_map_insert(&g_c_flat_map, key, make_record(i + cycle)));
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      runtime_record* found = runtime_c_flat_map_find(&g_c_flat_map, key);
      SSTL_TEST_ASSERT(found != 0);
      checksum += checksum_record(*found);
    }
    checksum += static_cast<unsigned long>(runtime_c_flat_map_size(&g_c_flat_map));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_flat_map), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_flat_map_lane() {
  lane_guard guard("cpp-sstl flat_map", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    sstl::flat_map<int, runtime_record, RUNTIME_ASSOC_CAP> m;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(m.insert(sstl::make_pair(key, make_record(i + cycle))).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      sstl::flat_map<int, runtime_record, RUNTIME_ASSOC_CAP>::iterator it = m.find(key);
      SSTL_TEST_ASSERT(it != m.end());
      checksum += checksum_record(it->second);
    }
    checksum += m.size();
  }
  const std::clock_t end = std::clock();
  guard.finish();
  typedef sstl::flat_map<int, runtime_record, RUNTIME_ASSOC_CAP> flat_map_type;
  metric out = {"cpp-sstl", sizeof(flat_map_type), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_flat_map_proxy_lane() {
  lane_guard guard("stl flat_map proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<flat_map_alloc_tag>();
  typedef std::pair<int, runtime_record> pair_type;
  typedef std::vector<pair_type, counting_allocator<pair_type, flat_map_alloc_tag> > flat_map_proxy_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    flat_map_proxy_type m;
    m.reserve(RUNTIME_ASSOC_CAP);
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      m.push_back(pair_type(key, make_record(i + cycle)));
    }
    std::sort(m.begin(), m.end(), runtime_pair_less());
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      for (flat_map_proxy_type::iterator it = m.begin(); it != m.end(); ++it) {
        if (it->first == key) { checksum += checksum_record(it->second); break; }
      }
    }
    checksum += static_cast<unsigned long>(m.size());
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(flat_map_proxy_type), allocation_counter<flat_map_alloc_tag>::peak,
                elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_flat_set_lane() {
  lane_guard guard("c-sstl flat_set", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    runtime_c_flat_set_init(&g_c_flat_set);
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(runtime_c_flat_set_insert(&g_c_flat_set, key));
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      int* found = runtime_c_flat_set_find(&g_c_flat_set, key);
      SSTL_TEST_ASSERT(found != 0);
      checksum += static_cast<unsigned long>(*found);
    }
    checksum += cycle;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_flat_set), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_flat_set_lane() {
  lane_guard guard("cpp-sstl flat_set", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    sstl::flat_set<int, RUNTIME_ASSOC_CAP> s;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(s.insert(key).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      sstl::flat_set<int, RUNTIME_ASSOC_CAP>::iterator it = s.find(key);
      SSTL_TEST_ASSERT(it != s.end());
      checksum += static_cast<unsigned long>(*it);
    }
    checksum += cycle;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  typedef sstl::flat_set<int, RUNTIME_ASSOC_CAP> flat_set_type;
  metric out = {"cpp-sstl", sizeof(flat_set_type), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_flat_set_proxy_lane() {
  lane_guard guard("stl flat_set proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<flat_set_alloc_tag>();
  typedef std::vector<int, counting_allocator<int, flat_set_alloc_tag> > flat_set_proxy_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    flat_set_proxy_type s;
    s.reserve(RUNTIME_ASSOC_CAP);
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) s.push_back(static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP));
    std::sort(s.begin(), s.end(), runtime_int_less);
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      flat_set_proxy_type::iterator it = std::lower_bound(s.begin(), s.end(), key);
      SSTL_TEST_ASSERT(it != s.end() && *it == key);
      checksum += static_cast<unsigned long>(*it);
    }
    checksum += cycle;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(flat_set_proxy_type), allocation_counter<flat_set_alloc_tag>::peak,
                elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_unordered_map_lane() {
  lane_guard guard("c-sstl unordered_map", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    runtime_c_unordered_map_init(&g_c_unordered_map);
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(runtime_c_unordered_map_insert(&g_c_unordered_map, key, make_record(i + cycle)));
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      runtime_record* found = runtime_c_unordered_map_find(&g_c_unordered_map, key);
      SSTL_TEST_ASSERT(found != 0);
      checksum += checksum_record(*found);
    }
    checksum += static_cast<unsigned long>(runtime_c_unordered_map_bucket_count(&g_c_unordered_map));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_unordered_map), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_unordered_map_lane() {
  lane_guard guard("cpp-sstl unordered_map", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    sstl::unordered_map<int, runtime_record, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS> m;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(m.insert(sstl::make_pair(key, make_record(i + cycle))).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      sstl::unordered_map<int, runtime_record, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS>::iterator it = m.find(key);
      SSTL_TEST_ASSERT(it != m.end());
      checksum += checksum_record(it->second);
    }
    checksum += static_cast<unsigned long>(m.bucket_count());
  }
  const std::clock_t end = std::clock();
  guard.finish();
  typedef sstl::unordered_map<int, runtime_record, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS> unordered_map_type;
  metric out = {"cpp-sstl", sizeof(unordered_map_type), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_unordered_map_proxy_lane() {
  lane_guard guard("stl unordered_map proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<unordered_map_alloc_tag>();
  typedef std::pair<const int, runtime_record> stl_pair_type;
  typedef std::map<int, runtime_record, std::less<int>, counting_allocator<stl_pair_type, unordered_map_alloc_tag> > map_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    map_type m;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(m.insert(stl_pair_type(key, make_record(i + cycle))).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      map_type::iterator it = m.find(key);
      SSTL_TEST_ASSERT(it != m.end());
      checksum += checksum_record(it->second);
    }
    checksum += static_cast<unsigned long>(RUNTIME_ASSOC_BUCKETS);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(map_type), allocation_counter<unordered_map_alloc_tag>::peak,
                elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_unordered_set_lane() {
  lane_guard guard("c-sstl unordered_set", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    runtime_c_unordered_set_init(&g_c_unordered_set);
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(runtime_c_unordered_set_insert(&g_c_unordered_set, key));
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      int* found = runtime_c_unordered_set_find(&g_c_unordered_set, key);
      SSTL_TEST_ASSERT(found != 0);
      checksum += static_cast<unsigned long>(*found);
    }
    checksum += static_cast<unsigned long>(runtime_c_unordered_set_bucket_count(&g_c_unordered_set));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_unordered_set), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_unordered_set_lane() {
  lane_guard guard("cpp-sstl unordered_set", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    sstl::unordered_set<int, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS> s;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      SSTL_TEST_ASSERT(s.insert(key).second);
    }
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      sstl::unordered_set<int, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS>::iterator found = s.find(key);
      SSTL_TEST_ASSERT(found != s.end());
      checksum += static_cast<unsigned long>(*found);
    }
    checksum += static_cast<unsigned long>(s.bucket_count());
  }
  const std::clock_t end = std::clock();
  guard.finish();
  typedef sstl::unordered_set<int, RUNTIME_ASSOC_CAP, RUNTIME_ASSOC_BUCKETS> unordered_set_type;
  metric out = {"cpp-sstl", sizeof(unordered_set_type), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_unordered_set_proxy_lane() {
  lane_guard guard("stl unordered_set proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<unordered_set_alloc_tag>();
  typedef std::set<int, std::less<int>, counting_allocator<int, unordered_set_alloc_tag> > set_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ASSOC_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ASSOC_CYCLES);
    set_type s;
    for (unsigned i = 0; i != RUNTIME_ASSOC_CAP; ++i) s.insert(static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP));
    for (unsigned i = 0; i < RUNTIME_ASSOC_CAP; i += 7u) {
      const int key = static_cast<int>((i * 37u) % RUNTIME_ASSOC_CAP);
      set_type::iterator it = s.find(key);
      SSTL_TEST_ASSERT(it != s.end());
      checksum += static_cast<unsigned long>(*it);
    }
    checksum += static_cast<unsigned long>(RUNTIME_ASSOC_BUCKETS);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(set_type), allocation_counter<unordered_set_alloc_tag>::peak,
                elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_bitset_lane() {
  lane_guard guard("c-sstl bitset", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    runtime_c_bitset_reset(&g_c_bitset);
    for (unsigned i = 0; i < RUNTIME_BITSET_BITS; i += 3u) runtime_c_bitset_set(&g_c_bitset, i);
    for (unsigned i = 1u; i < RUNTIME_BITSET_BITS; i += 5u) runtime_c_bitset_flip(&g_c_bitset, i);
    for (unsigned i = 0; i < RUNTIME_BITSET_BITS; i += 29u) if (runtime_c_bitset_test(&g_c_bitset, i)) ++checksum;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_bitset), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_bitset_lane() {
  lane_guard guard("cpp-sstl bitset", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    g_cpp_bitset.reset();
    for (unsigned i = 0; i < RUNTIME_BITSET_BITS; i += 3u) g_cpp_bitset.set(i);
    for (unsigned i = 1u; i < RUNTIME_BITSET_BITS; i += 5u) g_cpp_bitset.flip(i);
    for (unsigned i = 0; i < RUNTIME_BITSET_BITS; i += 29u) if (g_cpp_bitset.test(i)) ++checksum;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_bitset), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_bitset_lane() {
  lane_guard guard("stl bitset", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  typedef std::bitset<RUNTIME_BITSET_BITS> bitset_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    bitset_type b;
    for (unsigned i = 0; i < RUNTIME_BITSET_BITS; i += 3u) b.set(i);
    for (unsigned i = 1u; i < RUNTIME_BITSET_BITS; i += 5u) b.flip(i);
    for (unsigned i = 0; i < RUNTIME_BITSET_BITS; i += 29u) if (b.test(i)) ++checksum;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(bitset_type), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_string_view_lane() {
  lane_guard guard("cpp-sstl string_view", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    sstl::string_view view(g_cpp_string.c_str(), g_cpp_string.size());
    checksum += static_cast<unsigned long>(view.find("abc"));
    checksum += static_cast<unsigned long>(view.rfind("xyz"));
    checksum += view.starts_with("abc") ? 17u : 3u;
    checksum += view.ends_with("xyz") ? 19u : 5u;
    view.remove_prefix(cycle % 7u);
    view.remove_suffix(cycle % 5u);
    for (unsigned i = 0; i < view.size(); i += 257u) checksum += static_cast<unsigned long>(static_cast<unsigned char>(view[i]));
    checksum += view.size();
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(sstl::string_view), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_string_view_lane() {
  lane_guard guard("c-sstl string_view", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    runtime_c_string_view view = runtime_c_string_view_make(runtime_c_string_c_str(&g_c_string), runtime_c_string_size(&g_c_string));
    checksum += static_cast<unsigned long>(runtime_c_string_view_find(&view, "abc"));
    checksum += static_cast<unsigned long>(runtime_c_string_view_rfind(&view, "xyz"));
    checksum += runtime_c_string_view_starts_with(&view, "abc") ? 17u : 3u;
    checksum += runtime_c_string_view_ends_with(&view, "xyz") ? 19u : 5u;
    runtime_c_string_view_remove_prefix(&view, cycle % 7u);
    runtime_c_string_view_remove_suffix(&view, cycle % 5u);
    for (unsigned i = 0; i < runtime_c_string_view_size(&view); i += 257u) {
      const char* p = runtime_c_string_view_at(&view, i);
      SSTL_TEST_ASSERT(p != 0);
      checksum += static_cast<unsigned long>(static_cast<unsigned char>(*p));
    }
    checksum += static_cast<unsigned long>(runtime_c_string_view_size(&view));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(runtime_c_string_view), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_string_view_proxy_lane() {
  lane_guard guard("stl string_view proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  typedef std::basic_string<char, std::char_traits<char>, counting_allocator<char, string_alloc_tag> > string_type;
  reset_counter<string_alloc_tag>();
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_STRING_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_STRING_CYCLES);
    string_type text(g_cpp_string.c_str());
    string_type::size_type first = cycle % 7u;
    string_type::size_type last_trim = cycle % 5u;
    string_type::size_type logical_size = text.size() - first - last_trim;
    checksum += static_cast<unsigned long>(text.find("abc"));
    checksum += static_cast<unsigned long>(text.rfind("xyz"));
    checksum += text.compare(0u, 3u, "abc") == 0 ? 17u : 3u;
    checksum += text.compare(text.size() - 3u, 3u, "xyz") == 0 ? 19u : 5u;
    for (string_type::size_type i = 0; i < logical_size; i += 257u) {
      checksum += static_cast<unsigned long>(static_cast<unsigned char>(text[first + i]));
    }
    checksum += static_cast<unsigned long>(logical_size);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(string_type), allocation_counter<string_alloc_tag>::peak,
                elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_optional_lane() {
  lane_guard guard("c-sstl optional", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  runtime_c_optional_init(&g_c_optional);
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    runtime_c_optional_set(&g_c_optional, make_record(i));
    runtime_record* value = runtime_c_optional_try_value(&g_c_optional);
    SSTL_TEST_ASSERT(value != 0);
    checksum += checksum_record(*value);
    if ((i & 3u) == 0u) runtime_c_optional_reset(&g_c_optional);
    checksum += runtime_c_optional_has_value(&g_c_optional) ? 1u : 0u;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_optional), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_optional_lane() {
  lane_guard guard("cpp-sstl optional", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  sstl::optional<runtime_record> opt;
  const std::clock_t begin = std::clock();
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    opt = make_record(i);
    checksum += checksum_record(*opt);
    if ((i & 3u) == 0u) opt.reset();
    checksum += opt.has_value() ? 1u : 0u;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(opt), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_optional_proxy_lane() {
  lane_guard guard("stl optional proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  struct optional_proxy { bool has; runtime_record value; } opt;
  opt.has = false;
  const std::clock_t begin = std::clock();
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    opt.value = make_record(i);
    opt.has = true;
    checksum += checksum_record(opt.value);
    if ((i & 3u) == 0u) opt.has = false;
    checksum += opt.has ? 1u : 0u;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(opt), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_variant_lane() {
  lane_guard guard("c-sstl variant", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    if ((i & 1u) == 0u) {
      runtime_c_variant_set0(&g_c_variant, make_record(i));
      checksum += checksum_record(*runtime_c_variant_get0(&g_c_variant));
    } else {
      runtime_c_variant_set1(&g_c_variant, static_cast<int>(i));
      checksum += static_cast<unsigned long>(*runtime_c_variant_get1(&g_c_variant));
    }
    checksum += runtime_c_variant_index(&g_c_variant);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_variant), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_variant_lane() {
  lane_guard guard("cpp-sstl variant", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  sstl::variant2<runtime_record, int> var(make_record(0u));
  const std::clock_t begin = std::clock();
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    if ((i & 1u) == 0u) {
      var = make_record(i);
      checksum += checksum_record(sstl::get<0>(var));
    } else {
      var = static_cast<int>(i);
      checksum += static_cast<unsigned long>(sstl::get<1>(var));
    }
    checksum += var.index();
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(var), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_variant_proxy_lane() {
  lane_guard guard("stl variant proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  struct variant_proxy { unsigned index; runtime_record record; int integer; } var;
  const std::clock_t begin = std::clock();
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    if ((i & 1u) == 0u) {
      var.index = 0u;
      var.record = make_record(i);
      checksum += checksum_record(var.record);
    } else {
      var.index = 1u;
      var.integer = static_cast<int>(i);
      checksum += static_cast<unsigned long>(var.integer);
    }
    checksum += var.index;
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(var), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_function_lane() {
  lane_guard guard("c-sstl function", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  runtime_c_function1 fn = runtime_c_function1_make(runtime_c_function_value, 0);
  const std::clock_t begin = std::clock();
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    checksum += runtime_c_function1_call(&fn, i);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(fn), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_function_lane() {
  lane_guard guard("cpp-sstl function", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  sstl::function1<unsigned long, unsigned, 32> fn(runtime_function_value);
  const std::clock_t begin = std::clock();
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    checksum += fn(i);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(fn), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_function_proxy_lane() {
  lane_guard guard("stl function proxy", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  unsigned long (*fn)(unsigned) = runtime_function_value;
  const std::clock_t begin = std::clock();
  for (unsigned i = 0; i != RUNTIME_UTILITY_CYCLES; ++i) {
    guard.checkpoint("cycle", i, RUNTIME_UTILITY_CYCLES);
    checksum += fn(i);
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl-proxy", sizeof(fn), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_stack_lane() {
  lane_guard guard("c-sstl stack", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  runtime_c_stack_init(&g_c_stack);
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      SSTL_TEST_ASSERT(runtime_c_stack_push(&g_c_stack, make_record(i + cycle)));
    }
    while (!runtime_c_stack_empty(&g_c_stack)) {
      runtime_record out = make_record(0u);
      checksum += checksum_record(*runtime_c_stack_top(&g_c_stack));
      SSTL_TEST_ASSERT(runtime_c_stack_pop(&g_c_stack, &out));
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_stack), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_stack_lane() {
  lane_guard guard("cpp-sstl stack", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      SSTL_TEST_ASSERT(g_cpp_stack.push(make_record(i + cycle)));
    }
    while (!g_cpp_stack.empty()) {
      checksum += checksum_record(g_cpp_stack.top());
      g_cpp_stack.pop();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_stack), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_stack_lane() {
  lane_guard guard("stl stack", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<stack_alloc_tag>();
  typedef std::vector<runtime_record, counting_allocator<runtime_record, stack_alloc_tag> > stack_storage_type;
  typedef std::stack<runtime_record, stack_storage_type> stack_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    stack_storage_type storage;
    storage.reserve(RUNTIME_ADAPTOR_CAP);
    stack_type st(storage);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      st.push(make_record(i + cycle));
    }
    while (!st.empty()) {
      checksum += checksum_record(st.top());
      st.pop();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(stack_type), allocation_counter<stack_alloc_tag>::peak, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_queue_lane() {
  lane_guard guard("c-sstl queue", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  runtime_c_queue_init(&g_c_queue);
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      SSTL_TEST_ASSERT(runtime_c_queue_push(&g_c_queue, make_record(i + cycle)));
    }
    while (!runtime_c_queue_empty(&g_c_queue)) {
      runtime_record out = make_record(0u);
      checksum += checksum_record(*runtime_c_queue_front(&g_c_queue));
      SSTL_TEST_ASSERT(runtime_c_queue_pop(&g_c_queue, &out));
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_queue), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_queue_lane() {
  lane_guard guard("cpp-sstl queue", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      SSTL_TEST_ASSERT(g_cpp_queue.push(make_record(i + cycle)));
    }
    while (!g_cpp_queue.empty()) {
      checksum += checksum_record(g_cpp_queue.front());
      g_cpp_queue.pop();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_queue), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_queue_lane() {
  lane_guard guard("stl queue", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<queue_alloc_tag>();
  typedef std::deque<runtime_record, counting_allocator<runtime_record, queue_alloc_tag> > queue_storage_type;
  typedef std::queue<runtime_record, queue_storage_type> queue_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    queue_storage_type storage;
    queue_type q(storage);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      q.push(make_record(i + cycle));
    }
    while (!q.empty()) {
      checksum += checksum_record(q.front());
      q.pop();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(queue_type), allocation_counter<queue_alloc_tag>::peak, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_priority_queue_lane() {
  lane_guard guard("c-sstl priority_queue", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  runtime_c_priority_queue_init(&g_c_priority_queue);
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      SSTL_TEST_ASSERT(runtime_c_priority_queue_push(&g_c_priority_queue, make_record(i + cycle)));
    }
    while (!runtime_c_priority_queue_empty(&g_c_priority_queue)) {
      runtime_record out = make_record(0u);
      checksum += checksum_record(*runtime_c_priority_queue_top(&g_c_priority_queue));
      SSTL_TEST_ASSERT(runtime_c_priority_queue_pop(&g_c_priority_queue, &out));
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_priority_queue), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_priority_queue_lane() {
  lane_guard guard("cpp-sstl priority_queue", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  typedef sstl::priority_queue<runtime_record,
                               RUNTIME_ADAPTOR_CAP,
                               sstl::vector<runtime_record, RUNTIME_ADAPTOR_CAP>,
                               runtime_record_priority_less> priority_queue_type;
  static priority_queue_type pq;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      SSTL_TEST_ASSERT(pq.push(make_record(i + cycle)));
    }
    while (!pq.empty()) {
      checksum += checksum_record(pq.top());
      pq.pop();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(priority_queue_type), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_priority_queue_lane() {
  lane_guard guard("stl priority_queue", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<priority_queue_alloc_tag>();
  typedef std::vector<runtime_record, counting_allocator<runtime_record, priority_queue_alloc_tag> > priority_storage_type;
  typedef std::priority_queue<runtime_record, priority_storage_type, runtime_record_priority_less> priority_queue_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    priority_storage_type storage;
    storage.reserve(RUNTIME_ADAPTOR_CAP);
    priority_queue_type pq(runtime_record_priority_less(), storage);
    for (unsigned i = 0; i != RUNTIME_ADAPTOR_CAP; ++i) {
      pq.push(make_record(i + cycle));
    }
    while (!pq.empty()) {
      checksum += checksum_record(pq.top());
      pq.pop();
    }
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(priority_queue_type), allocation_counter<priority_queue_alloc_tag>::peak,
                elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_c_algorithm_lane() {
  lane_guard guard("c-sstl algorithm", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ALGORITHM_CAP; ++i) {
      g_c_algorithm_data[i] = runtime_algorithm_seed(i, cycle);
    }
    std::qsort(g_c_algorithm_data,
               RUNTIME_ALGORITHM_CAP,
               sizeof(g_c_algorithm_data[0]),
               runtime_qsort_int_compare);
    int* found = static_cast<int*>(std::bsearch(&g_c_algorithm_data[RUNTIME_ALGORITHM_CAP / 2u],
                                                g_c_algorithm_data,
                                                RUNTIME_ALGORITHM_CAP,
                                                sizeof(g_c_algorithm_data[0]),
                                                runtime_qsort_int_compare));
    SSTL_TEST_ASSERT(found != 0);
    for (unsigned i = 0; i < RUNTIME_ALGORITHM_CAP; i += 97u) {
      checksum += static_cast<unsigned long>(g_c_algorithm_data[i]);
    }
    checksum += static_cast<unsigned long>(found - g_c_algorithm_data);
    checksum += static_cast<unsigned long>(sstl_hash_bytes_v(g_c_algorithm_data, sizeof(g_c_algorithm_data)));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"c-sstl", sizeof(g_c_algorithm_data), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_cpp_algorithm_lane() {
  lane_guard guard("cpp-sstl algorithm", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    for (unsigned i = 0; i != RUNTIME_ALGORITHM_CAP; ++i) {
      g_cpp_algorithm_data[i] = runtime_algorithm_seed(i, cycle);
    }
    sstl::sort(g_cpp_algorithm_data, g_cpp_algorithm_data + RUNTIME_ALGORITHM_CAP);
    int* found = sstl::lower_bound(g_cpp_algorithm_data,
                                   g_cpp_algorithm_data + RUNTIME_ALGORITHM_CAP,
                                   g_cpp_algorithm_data[RUNTIME_ALGORITHM_CAP / 2u]);
    SSTL_TEST_ASSERT(found != g_cpp_algorithm_data + RUNTIME_ALGORITHM_CAP);
    for (unsigned i = 0; i < RUNTIME_ALGORITHM_CAP; i += 97u) {
      checksum += static_cast<unsigned long>(g_cpp_algorithm_data[i]);
    }
    checksum += static_cast<unsigned long>(found - g_cpp_algorithm_data);
    checksum += static_cast<unsigned long>(sstl_hash_bytes_v(g_cpp_algorithm_data, sizeof(g_cpp_algorithm_data)));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"cpp-sstl", sizeof(g_cpp_algorithm_data), 0u, elapsed_seconds(begin, end), checksum};
  return out;
}

static metric run_stl_algorithm_lane() {
  lane_guard guard("stl algorithm", RUNTIME_LANE_SECONDS);
  unsigned long checksum = 0u;
  reset_counter<algorithm_alloc_tag>();
  typedef std::vector<int, counting_allocator<int, algorithm_alloc_tag> > vector_type;
  const std::clock_t begin = std::clock();
  for (unsigned cycle = 0; cycle != RUNTIME_ADAPTOR_CYCLES; ++cycle) {
    guard.checkpoint("cycle", cycle, RUNTIME_ADAPTOR_CYCLES);
    vector_type v;
    v.reserve(RUNTIME_ALGORITHM_CAP);
    for (unsigned i = 0; i != RUNTIME_ALGORITHM_CAP; ++i) {
      const int value = runtime_algorithm_seed(i, cycle);
      g_stl_algorithm_data[i] = value;
      v.push_back(value);
    }
    std::sort(v.begin(), v.end());
    std::copy(v.begin(), v.end(), g_stl_algorithm_data);
    vector_type::iterator found = std::lower_bound(v.begin(), v.end(), v[RUNTIME_ALGORITHM_CAP / 2u]);
    SSTL_TEST_ASSERT(found != v.end());
    for (unsigned i = 0; i < RUNTIME_ALGORITHM_CAP; i += 97u) {
      checksum += static_cast<unsigned long>(v[i]);
    }
    checksum += static_cast<unsigned long>(found - v.begin());
    checksum += static_cast<unsigned long>(sstl_hash_bytes_v(g_stl_algorithm_data, sizeof(g_stl_algorithm_data)));
  }
  const std::clock_t end = std::clock();
  guard.finish();
  metric out = {"stl", sizeof(vector_type), allocation_counter<algorithm_alloc_tag>::peak,
                elapsed_seconds(begin, end), checksum};
  return out;
}

static void array_and_span_runtime_comparison_is_visible_and_equivalent() {
  metric array_metrics[3];
  metric span_metrics[3];
  array_metrics[0] = run_c_array_lane();
  array_metrics[1] = run_cpp_array_lane();
  array_metrics[2] = run_stl_array_proxy_lane();
  SSTL_TEST_EQ(array_metrics[0].checksum, array_metrics[1].checksum);
  SSTL_TEST_EQ(array_metrics[0].checksum, array_metrics[2].checksum);

  span_metrics[0] = run_c_span_lane();
  span_metrics[1] = run_cpp_span_lane();
  span_metrics[2] = run_stl_span_proxy_lane();
  SSTL_TEST_EQ(span_metrics[0].checksum, span_metrics[1].checksum);
  SSTL_TEST_EQ(span_metrics[0].checksum, span_metrics[2].checksum);

  print_metric_table("array indexed access comparison", array_metrics, 3u);
  print_metric_table("span view access comparison", span_metrics, 3u);
}

static void vector_runtime_comparison_is_visible_and_equivalent() {
  metric metrics[3];
  exercise_c_generated_vector_surface();
  metrics[0] = run_c_vector_lane();
  metrics[1] = run_cpp_vector_lane();
  metrics[2] = run_stl_vector_lane();

  /*
   * Matching checksums prove the three interfaces performed the same logical
   * work.  The timing and memory values are reported for human comparison, not
   * used as pass/fail thresholds.
   */
  SSTL_TEST_EQ(metrics[0].checksum, metrics[1].checksum);
  SSTL_TEST_EQ(metrics[0].checksum, metrics[2].checksum);
  print_metric_table("vector push/read/pop comparison", metrics, 3u);
}

static void string_runtime_comparison_is_visible_and_equivalent() {
  metric metrics[3];
  metric view_metrics[3];
  exercise_c_generated_string_surface();
  metrics[0] = run_c_string_lane();
  metrics[1] = run_cpp_string_lane();
  metrics[2] = run_stl_string_lane();

  SSTL_TEST_EQ(metrics[0].checksum, metrics[1].checksum);
  SSTL_TEST_EQ(metrics[0].checksum, metrics[2].checksum);
  print_metric_table("string append/read comparison", metrics, 3u);

  view_metrics[0] = run_c_string_view_lane();
  view_metrics[1] = run_cpp_string_view_lane();
  view_metrics[2] = run_stl_string_view_proxy_lane();
  SSTL_TEST_EQ(view_metrics[0].checksum, view_metrics[1].checksum);
  SSTL_TEST_EQ(view_metrics[0].checksum, view_metrics[2].checksum);
  print_metric_table("string_view find/trim/read comparison", view_metrics, 3u);
}

static void deque_runtime_comparison_is_visible_and_equivalent() {
  metric metrics[3];
  metrics[0] = run_c_deque_lane();
  metrics[1] = run_cpp_deque_lane();
  metrics[2] = run_stl_deque_lane();
  SSTL_TEST_EQ(metrics[0].checksum, metrics[1].checksum);
  SSTL_TEST_EQ(metrics[0].checksum, metrics[2].checksum);
  print_metric_table("deque push/read/pop comparison", metrics, 3u);
}

static void list_runtime_comparison_is_visible_and_equivalent() {
  metric metrics[3];
  metric forward_metrics[3];
  metrics[0] = run_c_list_lane();
  metrics[1] = run_cpp_list_lane();
  metrics[2] = run_stl_list_lane();
  SSTL_TEST_EQ(metrics[0].checksum, metrics[1].checksum);
  SSTL_TEST_EQ(metrics[0].checksum, metrics[2].checksum);

  forward_metrics[0] = run_c_forward_list_lane();
  forward_metrics[1] = run_cpp_forward_list_lane();
  forward_metrics[2] = run_stl_forward_list_proxy_lane();
  SSTL_TEST_EQ(forward_metrics[0].checksum, forward_metrics[1].checksum);
  SSTL_TEST_EQ(forward_metrics[0].checksum, forward_metrics[2].checksum);

  print_metric_table("list node traversal/pop comparison", metrics, 3u);
  print_metric_table("forward_list node traversal/pop comparison", forward_metrics, 3u);
}

static void associative_runtime_comparison_is_visible_and_equivalent() {
  metric map_metrics[3];
  metric set_metrics[3];
  metric flat_map_metrics[3];
  metric flat_set_metrics[3];
  metric unordered_map_metrics[3];
  metric unordered_set_metrics[3];
  map_metrics[0] = run_c_map_lane();
  map_metrics[1] = run_cpp_map_lane();
  map_metrics[2] = run_stl_map_lane();
  set_metrics[0] = run_c_set_lane();
  set_metrics[1] = run_cpp_set_lane();
  set_metrics[2] = run_stl_set_lane();
  flat_map_metrics[0] = run_c_flat_map_lane();
  flat_map_metrics[1] = run_cpp_flat_map_lane();
  flat_map_metrics[2] = run_stl_flat_map_proxy_lane();
  flat_set_metrics[0] = run_c_flat_set_lane();
  flat_set_metrics[1] = run_cpp_flat_set_lane();
  flat_set_metrics[2] = run_stl_flat_set_proxy_lane();
  unordered_map_metrics[0] = run_c_unordered_map_lane();
  unordered_map_metrics[1] = run_cpp_unordered_map_lane();
  unordered_map_metrics[2] = run_stl_unordered_map_proxy_lane();
  unordered_set_metrics[0] = run_c_unordered_set_lane();
  unordered_set_metrics[1] = run_cpp_unordered_set_lane();
  unordered_set_metrics[2] = run_stl_unordered_set_proxy_lane();
  SSTL_TEST_EQ(map_metrics[0].checksum, map_metrics[1].checksum);
  SSTL_TEST_EQ(map_metrics[0].checksum, map_metrics[2].checksum);
  SSTL_TEST_EQ(set_metrics[0].checksum, set_metrics[1].checksum);
  SSTL_TEST_EQ(set_metrics[0].checksum, set_metrics[2].checksum);
  SSTL_TEST_EQ(flat_map_metrics[0].checksum, flat_map_metrics[1].checksum);
  SSTL_TEST_EQ(flat_map_metrics[0].checksum, flat_map_metrics[2].checksum);
  SSTL_TEST_EQ(flat_set_metrics[0].checksum, flat_set_metrics[1].checksum);
  SSTL_TEST_EQ(flat_set_metrics[0].checksum, flat_set_metrics[2].checksum);
  SSTL_TEST_EQ(unordered_map_metrics[0].checksum, unordered_map_metrics[1].checksum);
  SSTL_TEST_EQ(unordered_map_metrics[0].checksum, unordered_map_metrics[2].checksum);
  SSTL_TEST_EQ(unordered_set_metrics[0].checksum, unordered_set_metrics[1].checksum);
  SSTL_TEST_EQ(unordered_set_metrics[0].checksum, unordered_set_metrics[2].checksum);
  print_metric_table("map insert/find comparison", map_metrics, 3u);
  print_metric_table("set insert/find comparison", set_metrics, 3u);
  print_metric_table("flat_map insert/find comparison", flat_map_metrics, 3u);
  print_metric_table("flat_set insert/find comparison", flat_set_metrics, 3u);
  print_metric_table("unordered_map insert/find comparison", unordered_map_metrics, 3u);
  print_metric_table("unordered_set insert/find comparison", unordered_set_metrics, 3u);
}

static void bitset_runtime_comparison_is_visible_and_equivalent() {
  metric metrics[3];
  metrics[0] = run_c_bitset_lane();
  metrics[1] = run_cpp_bitset_lane();
  metrics[2] = run_stl_bitset_lane();
  SSTL_TEST_EQ(metrics[0].checksum, metrics[1].checksum);
  SSTL_TEST_EQ(metrics[0].checksum, metrics[2].checksum);
  print_metric_table("bitset set/flip/test comparison", metrics, 3u);
}

static void utility_runtime_comparison_is_visible_and_equivalent() {
  metric optional_metrics[3];
  metric variant_metrics[3];
  metric function_metrics[3];
  optional_metrics[0] = run_c_optional_lane();
  optional_metrics[1] = run_cpp_optional_lane();
  optional_metrics[2] = run_stl_optional_proxy_lane();
  variant_metrics[0] = run_c_variant_lane();
  variant_metrics[1] = run_cpp_variant_lane();
  variant_metrics[2] = run_stl_variant_proxy_lane();
  function_metrics[0] = run_c_function_lane();
  function_metrics[1] = run_cpp_function_lane();
  function_metrics[2] = run_stl_function_proxy_lane();
  SSTL_TEST_EQ(optional_metrics[0].checksum, optional_metrics[1].checksum);
  SSTL_TEST_EQ(optional_metrics[0].checksum, optional_metrics[2].checksum);
  SSTL_TEST_EQ(variant_metrics[0].checksum, variant_metrics[1].checksum);
  SSTL_TEST_EQ(variant_metrics[0].checksum, variant_metrics[2].checksum);
  SSTL_TEST_EQ(function_metrics[0].checksum, function_metrics[1].checksum);
  SSTL_TEST_EQ(function_metrics[0].checksum, function_metrics[2].checksum);
  print_metric_table("optional set/reset/value comparison", optional_metrics, 3u);
  print_metric_table("variant assign/read comparison", variant_metrics, 3u);
  print_metric_table("function call comparison", function_metrics, 3u);
}

static void adaptor_runtime_comparison_is_visible_and_equivalent() {
  metric stack_metrics[3];
  metric queue_metrics[3];
  metric priority_metrics[3];
  stack_metrics[0] = run_c_stack_lane();
  stack_metrics[1] = run_cpp_stack_lane();
  stack_metrics[2] = run_stl_stack_lane();
  queue_metrics[0] = run_c_queue_lane();
  queue_metrics[1] = run_cpp_queue_lane();
  queue_metrics[2] = run_stl_queue_lane();
  priority_metrics[0] = run_c_priority_queue_lane();
  priority_metrics[1] = run_cpp_priority_queue_lane();
  priority_metrics[2] = run_stl_priority_queue_lane();
  SSTL_TEST_EQ(stack_metrics[0].checksum, stack_metrics[1].checksum);
  SSTL_TEST_EQ(stack_metrics[0].checksum, stack_metrics[2].checksum);
  SSTL_TEST_EQ(queue_metrics[0].checksum, queue_metrics[1].checksum);
  SSTL_TEST_EQ(queue_metrics[0].checksum, queue_metrics[2].checksum);
  SSTL_TEST_EQ(priority_metrics[0].checksum, priority_metrics[1].checksum);
  SSTL_TEST_EQ(priority_metrics[0].checksum, priority_metrics[2].checksum);
  print_metric_table("stack adaptor push/top/pop comparison", stack_metrics, 3u);
  print_metric_table("queue adaptor push/front/pop comparison", queue_metrics, 3u);
  print_metric_table("priority_queue adaptor push/top/pop comparison", priority_metrics, 3u);
}

static void algorithm_runtime_comparison_is_visible_and_equivalent() {
  metric metrics[3];
  metrics[0] = run_c_algorithm_lane();
  metrics[1] = run_cpp_algorithm_lane();
  metrics[2] = run_stl_algorithm_lane();
  SSTL_TEST_EQ(metrics[0].checksum, metrics[1].checksum);
  SSTL_TEST_EQ(metrics[0].checksum, metrics[2].checksum);
  print_metric_table("algorithm sort/lower_bound/hash comparison", metrics, 3u);
}

} // namespace

int main() {
  /*
   * Runtime reports are often redirected to files by CTest or the Python GUI.
   * Disable buffering so a long-running lane leaves visible breadcrumbs
   * immediately instead of producing an empty log until process exit.
   */
  std::setvbuf(stdout, 0, _IONBF, 0);
  std::setvbuf(stderr, 0, _IONBF, 0);
  std::printf("[runtime] runtime_interface_comparison starting\n");
  std::printf("[runtime] guardrail: each measured lane has a %u second wall-clock limit\n",
              RUNTIME_LANE_SECONDS);
  begin_runtime_csv_capture();

  const sstl_test::test_case tests[] = {
    {"array_and_span_runtime_comparison_is_visible_and_equivalent", array_and_span_runtime_comparison_is_visible_and_equivalent},
    {"vector_runtime_comparison_is_visible_and_equivalent", vector_runtime_comparison_is_visible_and_equivalent},
    {"string_runtime_comparison_is_visible_and_equivalent", string_runtime_comparison_is_visible_and_equivalent},
    {"deque_runtime_comparison_is_visible_and_equivalent", deque_runtime_comparison_is_visible_and_equivalent},
    {"list_runtime_comparison_is_visible_and_equivalent", list_runtime_comparison_is_visible_and_equivalent},
    {"associative_runtime_comparison_is_visible_and_equivalent", associative_runtime_comparison_is_visible_and_equivalent},
    {"bitset_runtime_comparison_is_visible_and_equivalent", bitset_runtime_comparison_is_visible_and_equivalent},
    {"utility_runtime_comparison_is_visible_and_equivalent", utility_runtime_comparison_is_visible_and_equivalent},
    {"adaptor_runtime_comparison_is_visible_and_equivalent", adaptor_runtime_comparison_is_visible_and_equivalent},
    {"algorithm_runtime_comparison_is_visible_and_equivalent", algorithm_runtime_comparison_is_visible_and_equivalent}
  };
  const int result = sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
  if (result == 0) write_runtime_csv_artifact();
  return result;
}
