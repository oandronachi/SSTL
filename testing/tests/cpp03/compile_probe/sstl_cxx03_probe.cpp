// test_id: cpp03.compile.public_headers
// api_ids: [cpp.public_headers]
// req_ids: [REQ-01, REQ-05, REQ-08, REQ-10]
// patterns: [STL-COMPILE-PARITY, FREESTANDING-COMPILE]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This is primarily a compile/link contract test. Passing means the public headers are self-contained in the intended dialect/profile.
 *   - Runtime behavior is intentionally minimal; the important failure mode is a diagnostic from the compiler or linker.
 *   - Add new public headers here when the implementation grows so dialect drift is caught early.
 */
#include <sstl/array.hpp>
#include <sstl/vector.hpp>
#include <sstl/string.hpp>
#include <sstl/span.hpp>
#include <sstl/deque.hpp>
#include <sstl/list.hpp>
#include <sstl/forward_list.hpp>
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
#include <sstl/queue.hpp>
#include <sstl/stack.hpp>
#include <sstl/priority_queue.hpp>
#include <sstl/algorithm.hpp>
#include <sstl/numeric.hpp>
#include <sstl/functional.hpp>
#include <cstddef>

struct probe_pod { int x; };
struct probe_alignas_storage { SSTL_ALIGNAS(16) char bytes[16]; };

int main() {
  sstl::array<int, 2> a;
  sstl::vector<int, 2> v;
  sstl::string<8> s;
  sstl::span<int> sp(a.data(), a.size());
  sstl::deque<int, 2> d;
  sstl::list<int, 2> l;
  sstl::forward_list<int, 2> fl;
  sstl::map<int, int, 2> m;
  sstl::set<int, 2> set;
  sstl::flat_map<int, int, 2> fm;
  sstl::flat_set<int, 2> fs;
  sstl::unordered_map<int, int, 2> um;
  sstl::unordered_set<int, 2> us;
  sstl::bitset<8> bits;
  sstl::optional<probe_pod> opt;
  sstl::variant2<int, probe_pod> var;
  sstl::function1<int, int, 32> fn;
  sstl::queue<int, 2> q;
  sstl::stack<int, 2> st;
  sstl::priority_queue<int, 2> pq;
  sstl::vector<int, static_cast<sstl::size_t>(3)> size_t_capacity_vector;
  sstl::string<0> s0;
  sstl::string<1> s1;
  sstl::deque<int, 0> d0;
  sstl::deque<int, 1> d1;
  sstl::list<int, 0> l0;
  sstl::list<int, 1> l1;
  sstl::forward_list<int, 0> fl0;
  sstl::forward_list<int, 1> fl1;
  sstl::flat_set<int, 0> fs0;
  sstl::flat_set<int, 1> fs1;
  sstl::unordered_map<int, int, 0, 0> um0;
  sstl::unordered_set<int, 0, 0> us0;
  q.push(1);
  q.push(2);
  const int queue_front = static_cast<const sstl::queue<int, 2>&>(q).front();
  const int queue_back = static_cast<const sstl::queue<int, 2>&>(q).back();
  st.push(3);
  const int stack_top = static_cast<const sstl::stack<int, 2>&>(st).top();
  pq.push(4);
  const int priority_top = static_cast<const sstl::priority_queue<int, 2>&>(pq).top();
  (void)sstl::vector<int, 0>::capacity();
  (void)sstl::string<0>::capacity();
  (void)sstl::deque<int, 0>::capacity();
  (void)sstl::list<int, 0>::capacity();
  (void)sstl::forward_list<int, 0>::capacity();
  (void)sstl::map<int, int, 0>::capacity();
  (void)sstl::set<int, 0>::capacity();
  (void)sstl::flat_map<int, int, 0>::capacity();
  (void)sstl::flat_set<int, 0>::capacity();
  (void)sstl::unordered_map<int, int, 0, 0>::capacity();
  (void)sstl::unordered_set<int, 0, 0>::capacity();
  if (sizeof(sstl::size_t) != sizeof(std::size_t)) return 80;
  if (sizeof(sstl::vector<int, 2>::size_type) != sizeof(std::size_t)) return 81;
  if (SSTL_ALIGNOF(probe_alignas_storage) < 16u) return 82;
  (void)s0.capacity();
  (void)s0.try_push_back('x');
  (void)s0.try_insert(0u, "x");
  (void)s0.try_append("x");
  (void)s0.try_assign("");
  (void)s0.try_resize(0u);
  (void)s0.try_replace(0u, 0u, "x");
  (void)s1.try_push_back('a');
  (void)s1.try_insert(0u, 'b');
  (void)s1.try_append("c");
  (void)d0.capacity();
  (void)d0.try_push_back(1);
  (void)d0.try_push_front(1);
  (void)d1.try_push_back(1);
  (void)l0.capacity();
  (void)l0.try_push_back(1);
  (void)l0.try_push_front(1);
  (void)l0.try_insert(l0.begin(), 1);
  (void)l1.try_push_back(1);
  (void)fl0.capacity();
  (void)fl0.try_push_front(1);
  (void)fl0.try_insert_after(fl0.before_begin(), 1);
  (void)fl1.try_push_front(1);
  (void)fs0.capacity();
  (void)fs1.capacity();
  (void)um0.capacity();
  (void)us0.capacity();
  (void)v; (void)s; (void)sp; (void)d; (void)l; (void)fl; (void)m;
  (void)set; (void)fm; (void)fs; (void)um; (void)us; (void)bits;
  (void)opt; (void)var; (void)fn; (void)q; (void)st; (void)pq; (void)size_t_capacity_vector;
  (void)queue_front; (void)queue_back; (void)stack_top; (void)priority_top;
  return 0;
}
