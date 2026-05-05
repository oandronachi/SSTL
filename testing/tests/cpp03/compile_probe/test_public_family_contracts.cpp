// test_id: cpp03.public_families.basic_contracts
// api_ids: [cpp.array.contract, cpp.array.comparison, cpp.span.contract, cpp.deque.contract, cpp.list.contract, cpp.forward_list.contract, cpp.map.contract, cpp.set.contract, cpp.set.erase, cpp.flat_map.contract, cpp.flat_map.erase, cpp.flat_map.bounds, cpp.flat_set.contract, cpp.flat_set.erase, cpp.unordered_map.contract, cpp.unordered_map.erase, cpp.unordered_set.contract, cpp.unordered_set.erase, cpp.bitset.contract, cpp.optional.contract, cpp.variant.contract, cpp.function.contract, cpp.functional.contract, cpp.numeric.contract, cpp.iterator.contract, cpp.utility.non_member_hooks, cpp.lifetime.shims, cpp.vector.cross_capacity.sfinae, cpp.queue.contract, cpp.stack.contract, cpp.priority_queue.contract, cpp.owning.try_capacity]
// req_ids: [REQ-02, REQ-03, REQ-05, REQ-06]
// patterns: [CAPACITY-BOUNDARY, STL-COMPILE-PARITY, AUDIT-NOALLOC]
/*
 * Notes:
 *   - This file is intentionally verbose: the comments are part of the test contract, not decoration.
 *   - This is primarily a compile/link contract test. Passing means the public headers are self-contained in the intended dialect/profile.
 *   - Runtime behavior is intentionally minimal; the important failure mode is a diagnostic from the compiler or linker.
 *   - Add new public headers here when the implementation grows so dialect drift is caught early.
 */
#include <sstl/array.hpp>
#include <sstl/span.hpp>
#include <sstl/vector.hpp>
#include <sstl/string.hpp>
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
#include <sstl/functional.hpp>
#include <sstl/numeric.hpp>
#include <sstl/queue.hpp>
#include <sstl/stack.hpp>
#include <sstl/priority_queue.hpp>
#include <cstddef>
#include "test_harness.hpp"
#include "noalloc_audit.hpp"

static int plus_one(int x) { return x + 1; }
static int add_two(int a, int b) { return a + b; }
static int add_three(int a, int b, int c) { return a + b + c; }

typedef char yes_probe;
struct no_probe { char bytes[2]; };

template <sstl::size_t M>
static yes_probe vector_copy_fits_probe(char (*)[sizeof(sstl::vector<int, 2>(*static_cast<const sstl::vector<int, M>*>(0)))] = 0);

template <sstl::size_t M>
static no_probe vector_copy_fits_probe(...);

struct stateful_adder {
  int bias;
  explicit stateful_adder(int b) : bias(b) {}
  int operator()(int x) const { return x + bias; }
};

struct variant_visit_counter {
  int* seen;
  explicit variant_visit_counter(int* p) : seen(p) {}
  template <class T>
  void operator()(const T&) const { ++*seen; }
};

template <class Iter>
static void require_iterator_nested_typedefs(void) {
  typename Iter::value_type* value = 0;
  typename Iter::pointer pointer = 0;
  typename Iter::difference_type distance = 0;
  typename Iter::iterator_category category;
  (void)value;
  (void)pointer;
  (void)distance;
  (void)category;
}

static void contiguous_and_deque_contracts() {
  sstl_test::noalloc_guard guard;
  sstl::array<int, 3> a;
  a.fill(4);
  SSTL_TEST_EQ(a.size(), 3u);
  SSTL_TEST_EQ(sstl::size(a), 3u);
  SSTL_TEST_ASSERT(!sstl::empty(a));
  SSTL_TEST_EQ(*sstl::begin(a), 4);
  SSTL_TEST_EQ(sstl::end(a) - sstl::begin(a), 3);
  SSTL_TEST_EQ(*sstl::data(a), 4);
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(a, a.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(a, a.end()));
  SSTL_TEST_EQ(*a.try_front(), 4);
  SSTL_TEST_EQ(*a.try_back(), 4);
  SSTL_TEST_EQ(*a.cbegin(), 4);
  SSTL_TEST_EQ(*a.rbegin(), 4);
  SSTL_TEST_EQ(*(a.crbegin() + 1), 4);
  sstl::array<int, 3> swapped;
  swapped.fill(1);
  sstl::swap(a, swapped);
  SSTL_TEST_EQ(a[0], 1);
  SSTL_TEST_EQ(swapped[0], 4);
  sstl::swap(a, swapped);
  SSTL_TEST_EQ(a[0], 4);
  sstl::array<int, 3> equal;
  equal.fill(4);
  sstl::array<int, 3> less;
  less.fill(4);
  less[2] = 3;
  sstl::array<int, 3> greater;
  greater.fill(4);
  greater[2] = 5;
  SSTL_TEST_ASSERT(a == equal);
  SSTL_TEST_ASSERT(!(a != equal));
  SSTL_TEST_ASSERT(less < a);
  SSTL_TEST_ASSERT(less <= a);
  SSTL_TEST_ASSERT(a <= equal);
  SSTL_TEST_ASSERT(greater > a);
  SSTL_TEST_ASSERT(greater >= a);
  SSTL_TEST_ASSERT(a >= equal);
  SSTL_TEST_ASSERT(!(a < less));
  sstl::array<int, 0> empty_left;
  sstl::array<int, 0> empty_right;
  SSTL_TEST_ASSERT(empty_left == empty_right);
  SSTL_TEST_ASSERT(!(empty_left != empty_right));
  SSTL_TEST_ASSERT(!(empty_left < empty_right));
  SSTL_TEST_ASSERT(empty_left <= empty_right);
  SSTL_TEST_ASSERT(!(empty_left > empty_right));
  SSTL_TEST_ASSERT(empty_left >= empty_right);
  sstl::span<int> sp(a.data(), a.size());
  SSTL_TEST_EQ(sp.size(), 3u);
  SSTL_TEST_EQ(sp.size_bytes(), 3u * static_cast<unsigned>(sizeof(int)));
  SSTL_TEST_EQ(sstl::size(sp), 3u);
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(sp, sp.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(sp, sp.end()));
  sp[1] = 9;
  SSTL_TEST_EQ(a[1], 9);
  SSTL_TEST_EQ(sp.at(1), 9);
  SSTL_TEST_EQ(sp.front(), 4);
  SSTL_TEST_EQ(sp.back(), 4);
  SSTL_TEST_EQ(*sp.cbegin(), 4);
  SSTL_TEST_EQ(*sp.rbegin(), 4);
  SSTL_TEST_EQ(*(sp.crbegin() + 1), 9);
  sstl::span<int> sp_from_array(a);
  SSTL_TEST_EQ(sp_from_array.size(), 3u);
  SSTL_TEST_EQ(sp_from_array.first(2u).size(), 2u);
  SSTL_TEST_EQ(sp_from_array.first(9u).size(), 3u);
  SSTL_TEST_EQ(sp_from_array.last(2u)[0], 9);
  SSTL_TEST_EQ(sp_from_array.last(9u).size(), 3u);
  SSTL_TEST_EQ(sp_from_array.subspan(1u, 1u)[0], 9);
  SSTL_TEST_EQ(sp_from_array.subspan(2u, 9u).size(), 1u);
  SSTL_TEST_ASSERT(sp_from_array.subspan(9u, 2u).empty());
  SSTL_TEST_ASSERT(sp_from_array.subspan(9u).empty());
  sstl::vector<int, 4> v;
  SSTL_TEST_ASSERT(v.push_back(10));
  SSTL_TEST_ASSERT(v.push_back(20));
  SSTL_TEST_ASSERT(v.push_back(30));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(v, v.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(v, v.end()));
  sstl::span<int> sp_from_vector(v);
  SSTL_TEST_EQ(sp_from_vector.size(), 3u);
  SSTL_TEST_EQ(sp_from_vector.subspan(1u)[0], 20);
  SSTL_TEST_EQ(*v.cbegin(), 10);
  SSTL_TEST_EQ(*v.rbegin(), 30);
  SSTL_TEST_EQ(*(v.crbegin() + 1), 20);
  sstl::string<8> text("abc");
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(text, text.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(text, text.end()));
  SSTL_TEST_EQ(*text.cbegin(), 'a');
  SSTL_TEST_EQ(*text.rbegin(), 'c');
  SSTL_TEST_EQ(*(text.crbegin() + 1), 'b');
  int raw[2] = {5, 6};
  sstl::span<int> sp_from_raw(raw);
  SSTL_TEST_EQ(sp_from_raw.size(), 2u);
  SSTL_TEST_EQ(sp_from_raw.last(1u)[0], 6);
  SSTL_TEST_EQ(sstl::size(raw), 2u);
  SSTL_TEST_EQ(*sstl::begin(raw), 5);
  SSTL_TEST_EQ(*(sstl::end(raw) - 1), 6);
  SSTL_TEST_ASSERT(!sstl::empty(raw));
  SSTL_TEST_EQ(*sstl::data(raw), 5);

  sstl::deque<int, 3> d;
  SSTL_TEST_ASSERT(d.push_back(2));
  SSTL_TEST_ASSERT(d.push_front(1));
  SSTL_TEST_ASSERT(d.push_back(3));
  SSTL_TEST_ASSERT(d.full());
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(d, d.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(d, d.end()));
  SSTL_TEST_ASSERT(!d.push_back(4));
  SSTL_TEST_EQ(*d.try_front(), 1);
  SSTL_TEST_EQ(*d.try_back(), 3);
  SSTL_TEST_EQ(d.front(), 1);
  SSTL_TEST_EQ(d.back(), 3);
  d.pop_front();
  SSTL_TEST_ASSERT(d.push_back(4));
  SSTL_TEST_EQ(d.front(), 2);
  SSTL_TEST_EQ(d.back(), 4);
}

static void ordered_and_unordered_contracts() {
  sstl_test::noalloc_guard guard;
  sstl::map<int, int, 3> m;
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(2, 20)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(m.insert(sstl::make_pair(3, 30)).second);
  SSTL_TEST_EQ(m.max_size(), 3u);
  SSTL_TEST_EQ(m.count(2), 1u);
  SSTL_TEST_EQ(m.count(9), 0u);
  SSTL_TEST_ASSERT(m.find(2) != m.end());
  SSTL_TEST_EQ(m.begin()->first, 1);
  SSTL_TEST_EQ(m.lower_bound(2)->first, 2);
  SSTL_TEST_EQ(m.lower_bound(0)->first, 1);
  SSTL_TEST_EQ(m.upper_bound(2)->first, 3);
  SSTL_TEST_ASSERT(m.equal_range(2).first == m.find(2));
  SSTL_TEST_ASSERT(m.equal_range(2).second == m.upper_bound(2));

  sstl::set<int, 3> s;
  SSTL_TEST_ASSERT(s.insert(7).second);
  SSTL_TEST_ASSERT(s.insert(8).second);
  SSTL_TEST_EQ(s.max_size(), 3u);
  SSTL_TEST_EQ(s.count(7), 1u);
  SSTL_TEST_EQ(s.count(9), 0u);
  SSTL_TEST_EQ(*s.lower_bound(7), 7);
  SSTL_TEST_EQ(*s.upper_bound(7), 8);
  SSTL_TEST_ASSERT(s.equal_range(7).first == s.find(7));
  SSTL_TEST_ASSERT(s.equal_range(7).second == s.upper_bound(7));
  SSTL_TEST_ASSERT(!s.insert(7).second);
  SSTL_TEST_EQ(s.erase(7), 1u);
  SSTL_TEST_ASSERT(s.find(7) == s.end());
  sstl::set<int, 3>::iterator set_it = s.find(8);
  sstl::set<int, 3>::iterator set_after = s.erase(set_it);
  SSTL_TEST_ASSERT(set_after == s.end());
  SSTL_TEST_ASSERT(s.empty());

  sstl::flat_map<int, int, 3> fm;
  SSTL_TEST_ASSERT(fm.insert(sstl::make_pair(3, 30)).second);
  SSTL_TEST_ASSERT(fm.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(fm.find(3) != fm.end());
  SSTL_TEST_EQ(fm.lower_bound(2)->first, 3);
  SSTL_TEST_EQ(fm.upper_bound(1)->first, 3);
  SSTL_TEST_ASSERT(fm.equal_range(3).first == fm.find(3));
  SSTL_TEST_EQ(fm.erase(1), 1u);
  sstl::flat_map<int, int, 3>::iterator fm_after = fm.erase(fm.find(3));
  SSTL_TEST_ASSERT(fm_after == fm.end());
  SSTL_TEST_ASSERT(fm.empty());

  sstl::flat_set<int, 3> fs;
  SSTL_TEST_ASSERT(fs.insert(5).second);
  SSTL_TEST_ASSERT(fs.find(5) != fs.end());
  SSTL_TEST_EQ(fs.erase(5), 1u);
  SSTL_TEST_ASSERT(fs.insert(6).second);
  sstl::flat_set<int, 3>::iterator fs_after = fs.erase(fs.find(6));
  SSTL_TEST_ASSERT(fs_after == fs.end());
  SSTL_TEST_ASSERT(fs.empty());

  sstl::unordered_map<int, int, 3, 7> um;
  SSTL_TEST_EQ(um.bucket_count(), 7u);
  SSTL_TEST_ASSERT(um.insert(sstl::make_pair(4, 40)).second);
  SSTL_TEST_ASSERT(um.find(4) != um.end());
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(um, um.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(um, um.end()));
  SSTL_TEST_EQ(um.erase(4), 1u);
  SSTL_TEST_ASSERT(um.insert(sstl::make_pair(5, 50)).second);
  sstl::unordered_map<int, int, 3, 7>::iterator um_after = um.erase(um.find(5));
  SSTL_TEST_ASSERT(um_after == um.end());
  SSTL_TEST_ASSERT(um.empty());

  sstl::unordered_map<int, int, 4> default_bucket_map;
  SSTL_TEST_EQ(sstl::next_prime_ge<0>::value, 2u);
  SSTL_TEST_EQ(sstl::next_prime_ge<1>::value, 2u);
  SSTL_TEST_EQ(sstl::next_prime_ge<4>::value, 5u);
  SSTL_TEST_EQ(sstl::next_prime_ge<16>::value, 17u);
  SSTL_TEST_EQ(default_bucket_map.bucket_count(), 5u);

  sstl::unordered_set<int, 3, 7> us;
  SSTL_TEST_EQ(us.bucket_count(), 7u);
  SSTL_TEST_ASSERT(us.insert(6).second);
  SSTL_TEST_ASSERT(us.find(6) != us.end());
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(us, us.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(us, us.end()));
  SSTL_TEST_EQ(us.erase(6), 1u);
  SSTL_TEST_ASSERT(us.insert(9).second);
  sstl::unordered_set<int, 3, 7>::iterator us_after = us.erase(us.find(9));
  SSTL_TEST_ASSERT(us_after == us.end());
  SSTL_TEST_ASSERT(us.empty());

  sstl::unordered_set<int, 4> default_bucket_set;
  SSTL_TEST_EQ(default_bucket_set.bucket_count(), 5u);
}

static void try_and_capacity_contracts_cover_zero_and_one() {
  sstl_test::noalloc_guard guard;

  SSTL_TEST_EQ((sstl::vector<int, 1>::capacity()), 1u);
  SSTL_TEST_EQ((sstl::string<0>::capacity()), 0u);
  SSTL_TEST_EQ((sstl::deque<int, 0>::capacity()), 0u);
  SSTL_TEST_EQ((sstl::list<int, 1>::capacity()), 1u);
  SSTL_TEST_EQ((sstl::forward_list<int, 1>::capacity()), 1u);
  SSTL_TEST_EQ((sstl::map<int, int, 1>::capacity()), 1u);
  SSTL_TEST_EQ((sstl::set<int, 1>::capacity()), 1u);
  SSTL_TEST_EQ((sstl::flat_map<int, int, 1>::capacity()), 1u);
  SSTL_TEST_EQ((sstl::flat_set<int, 1>::capacity()), 1u);
  SSTL_TEST_EQ((sstl::unordered_map<int, int, 0, 0>::capacity()), 0u);
  SSTL_TEST_EQ((sstl::unordered_set<int, 0, 0>::capacity()), 0u);

  sstl::string<0> s0;
  SSTL_TEST_EQ(s0.capacity(), 0u);
  SSTL_TEST_ASSERT(s0.try_assign(""));
  SSTL_TEST_ASSERT(s0.try_resize(0u));
  SSTL_TEST_ASSERT(s0.try_insert(0u, ""));
  SSTL_TEST_ASSERT(!s0.try_push_back('x'));
  SSTL_TEST_ASSERT(!s0.try_append("x"));
  SSTL_TEST_ASSERT(!s0.try_insert(0u, 'x'));
  SSTL_TEST_ASSERT(!s0.try_replace(0u, 0u, "x"));

  sstl::string<1> s1;
  SSTL_TEST_ASSERT(s1.try_push_back('a'));
  SSTL_TEST_ASSERT(!s1.try_push_back('b'));
  SSTL_TEST_ASSERT(!s1.try_append("b"));
  SSTL_TEST_ASSERT(s1.try_replace(0u, 1u, "b"));
  SSTL_TEST_ASSERT(s1.try_assign("c"));
  SSTL_TEST_ASSERT(s1.try_resize(1u, 'd'));

  sstl::vector<int, 1> v1;
  int range_value = 8;
  sstl::vector<int, 1>::iterator first_insert = v1.try_insert(v1.begin(), 7);
  SSTL_TEST_ASSERT(first_insert != v1.end());
  SSTL_TEST_ASSERT(v1.try_insert(v1.begin(), 1u, 9) == v1.end());
  SSTL_TEST_ASSERT(v1.try_insert(v1.begin(), &range_value, &range_value + 1) == v1.end());

  sstl::deque<int, 0> d0;
  SSTL_TEST_EQ(d0.capacity(), 0u);
  SSTL_TEST_ASSERT(!d0.try_push_back(1));
  SSTL_TEST_ASSERT(!d0.try_push_front(1));
  sstl::deque<int, 1> d1;
  SSTL_TEST_ASSERT(d1.try_push_back(1));
  SSTL_TEST_ASSERT(!d1.try_push_front(2));

  sstl::list<int, 0> l0;
  SSTL_TEST_EQ(l0.capacity(), 0u);
  SSTL_TEST_ASSERT(!l0.try_push_back(1));
  SSTL_TEST_ASSERT(!l0.try_push_front(1));
  SSTL_TEST_ASSERT(l0.try_insert(l0.begin(), 1) == l0.end());
  sstl::list<int, 1> l1;
  SSTL_TEST_EQ(l1.max_size(), 1u);
  SSTL_TEST_ASSERT(l1.try_push_front(1));
  SSTL_TEST_ASSERT(!l1.try_push_back(2));

  sstl::forward_list<int, 0> fl0;
  SSTL_TEST_EQ(fl0.capacity(), 0u);
  SSTL_TEST_ASSERT(!fl0.try_push_front(1));
  SSTL_TEST_ASSERT(fl0.try_insert_after(fl0.before_begin(), 1) == fl0.end());
  sstl::forward_list<int, 1> fl1;
  SSTL_TEST_EQ(fl1.max_size(), 1u);
  SSTL_TEST_ASSERT(fl1.try_push_front(1));
  SSTL_TEST_ASSERT(!fl1.try_push_front(2));

  sstl::flat_set<int, 0> fs0;
  SSTL_TEST_EQ(fs0.capacity(), 0u);
  sstl::flat_set<int, 1> fs1;
  SSTL_TEST_EQ(fs1.max_size(), 1u);
  SSTL_TEST_ASSERT(fs1.insert(1).second);

  sstl::unordered_map<int, int, 0, 0> um0;
  SSTL_TEST_EQ(um0.capacity(), 0u);
  SSTL_TEST_EQ(um0.bucket_count(), 0u);
  sstl::unordered_set<int, 0, 0> us0;
  SSTL_TEST_EQ(us0.capacity(), 0u);
  SSTL_TEST_EQ(us0.bucket_count(), 0u);
}

static void linked_and_heap_contracts() {
  sstl_test::noalloc_guard guard;
  sstl::list<int, 4> l;
  SSTL_TEST_ASSERT(l.push_back(1));
  SSTL_TEST_ASSERT(l.push_back(2));
  SSTL_TEST_ASSERT(l.push_back(3));
  SSTL_TEST_EQ(l.back(), 3);
  l.reverse();
  SSTL_TEST_EQ(l.front(), 3);
  int popped = 0;
  SSTL_TEST_ASSERT(l.try_pop_back(&popped));
  SSTL_TEST_EQ(popped, 1);
  SSTL_TEST_ASSERT(l.push_back(1));
  l.sort();
  sstl::list<int, 4>::iterator lit = l.begin();
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(l, lit));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(l, l.end()));
  SSTL_TEST_EQ(*lit, 1);
  ++lit;
  SSTL_TEST_EQ(*lit, 2);
  ++lit;
  SSTL_TEST_EQ(*lit, 3);

  sstl::queue<int, 3> q;
  SSTL_TEST_ASSERT(q.push(1));
  SSTL_TEST_ASSERT(q.push(2));
  SSTL_TEST_EQ(q.front(), 1);
  SSTL_TEST_EQ(q.back(), 2);
  const sstl::queue<int, 3>& cq = q;
  SSTL_TEST_EQ(cq.front(), 1);
  SSTL_TEST_EQ(cq.back(), 2);
  q.pop();
  SSTL_TEST_EQ(q.front(), 2);

  sstl::stack<int, 3> st;
  SSTL_TEST_ASSERT(st.push(1));
  SSTL_TEST_ASSERT(st.push(2));
  SSTL_TEST_EQ(st.top(), 2);
  const sstl::stack<int, 3>& cst = st;
  SSTL_TEST_EQ(cst.top(), 2);
  st.pop();
  SSTL_TEST_EQ(st.top(), 1);

  sstl::priority_queue<int, 4> pq;
  SSTL_TEST_ASSERT(pq.push(2));
  SSTL_TEST_ASSERT(pq.push(5));
  SSTL_TEST_ASSERT(pq.push(1));
  SSTL_TEST_EQ(pq.top(), 5);
  const sstl::priority_queue<int, 4>& cpq = pq;
  SSTL_TEST_EQ(cpq.top(), 5);
  pq.pop();
  SSTL_TEST_EQ(pq.top(), 2);
}

static void utility_family_contracts() {
  sstl_test::noalloc_guard guard;
  SSTL_TEST_EQ(sizeof(vector_copy_fits_probe<1>(0)), sizeof(yes_probe));
  SSTL_TEST_EQ(sizeof(vector_copy_fits_probe<3>(0)), sizeof(no_probe));

  sstl::raw_storage<int, 1> raw;
  int* constructed = SSTL_CONSTRUCT_AT(raw.ptr(0), 99);
  SSTL_TEST_EQ(*constructed, 99);
  SSTL_DESTROY_AT(constructed);
  SSTL_TEST_EQ(sizeof(sstl::size_t), sizeof(std::size_t));
  SSTL_TEST_EQ(sizeof(sstl::vector<int, 4>::size_type), sizeof(std::size_t));
  SSTL_TEST_EQ(sizeof(sstl::string<8>::size_type), sizeof(std::size_t));

  int numeric_values[4] = {0, 0, 0, 0};
  sstl::iota(numeric_values, numeric_values + 4, 1);
  SSTL_TEST_EQ(sstl::accumulate(numeric_values, numeric_values + 4, 0), 10);

  sstl::bitset<8> bits;
  SSTL_TEST_EQ(sizeof(bits), sizeof(unsigned long));
  SSTL_TEST_EQ(bits.size(), 8u);
  bits.set();
  SSTL_TEST_ASSERT(bits.all());
  SSTL_TEST_EQ(bits.count(), 8u);
  bits.reset();
  bits.set(3);
  SSTL_TEST_ASSERT(bits.test(3));
  bits.set(3, false);
  SSTL_TEST_ASSERT(!bits.test(3));
  bits.set(3, true);
  SSTL_TEST_ASSERT(bits.test(3));
  bits[4] = true;
  SSTL_TEST_ASSERT(bits[4]);
  bits[4].flip();
  SSTL_TEST_ASSERT(!bits[4]);
  bits[4] = bits[3];
  SSTL_TEST_ASSERT(bits[4]);
  bits.reset(4);
  bits.flip(3);
  SSTL_TEST_ASSERT(!bits.test(3));
  bits.set(1).set(2);
  SSTL_TEST_EQ(bits.count(), 2u);
  bits <<= 1u;
  SSTL_TEST_ASSERT(bits.test(2));
  SSTL_TEST_ASSERT(bits.test(3));
  SSTL_TEST_ASSERT(!bits.test(1));
  sstl::bitset<8> shifted_left = bits << 1u;
  SSTL_TEST_ASSERT(shifted_left.test(3));
  SSTL_TEST_ASSERT(shifted_left.test(4));
  sstl::bitset<8> shifted_right = shifted_left >> 1u;
  SSTL_TEST_ASSERT(shifted_right == bits);
  SSTL_TEST_ASSERT(!(shifted_right != bits));
  sstl::bitset<8> inverted = ~bits;
  SSTL_TEST_ASSERT(inverted.test(1));
  SSTL_TEST_ASSERT(bits.any());
  SSTL_TEST_ASSERT(!bits.none());
  SSTL_TEST_ASSERT(bits.to_string() == "00001100");
  sstl::bitset<8> from_ulong(5ul);
  SSTL_TEST_ASSERT(from_ulong.test(0));
  SSTL_TEST_ASSERT(from_ulong.test(2));
  SSTL_TEST_ASSERT(!from_ulong.test(1));
  sstl::bitset<8> from_text("10100001");
  SSTL_TEST_ASSERT(from_text.test(0));
  SSTL_TEST_ASSERT(from_text.test(5));
  SSTL_TEST_ASSERT(from_text.test(7));
  sstl::string<8> bit_text("00010000");
  sstl::bitset<8> from_sstl_string(bit_text);
  SSTL_TEST_ASSERT(from_sstl_string.test(4));
  sstl::bitset<0> empty_bits;
  SSTL_TEST_ASSERT(empty_bits.to_string() == "");
  sstl::bitset<sizeof(unsigned long) * 8u + 2u> overflow_bits;
  overflow_bits.set(0u).set(sizeof(unsigned long) * 8u + 1u);
  SSTL_TEST_EQ(overflow_bits.to_ulong(), 1ul);

  sstl::optional<int> opt;
  SSTL_TEST_ASSERT(!opt);
  opt = 42;
  SSTL_TEST_ASSERT(opt);
  SSTL_TEST_EQ(*opt, 42);
  SSTL_TEST_EQ(opt.value(), 42);
  SSTL_TEST_EQ(opt.value_or(7), 42);
  opt = sstl::nullopt;
  SSTL_TEST_ASSERT(opt == sstl::nullopt);
  SSTL_TEST_EQ(opt.value_or(7), 7);

  sstl::variant2<int, char> var(7);
  SSTL_TEST_EQ(var.index(), 0u);
  SSTL_TEST_ASSERT(sstl::holds_alternative<0>(var));
  SSTL_TEST_ASSERT(!sstl::holds_alternative<1>(var));
  SSTL_TEST_EQ(sstl::get<0>(var), 7);
  var = 'a';
  SSTL_TEST_ASSERT(sstl::get_if<0>(&var) == 0);
  SSTL_TEST_ASSERT(sstl::get_if<1>(&var) != 0);
  SSTL_TEST_ASSERT(!sstl::holds_alternative<0>(var));
  SSTL_TEST_ASSERT(sstl::holds_alternative<1>(var));
  SSTL_TEST_ASSERT(sstl::holds_alternative<char>(var));
  sstl::variant3<int, char, long> var3(9l);
  SSTL_TEST_EQ(var3.index(), 2u);
  SSTL_TEST_ASSERT(sstl::holds_alternative<2>(var3));
  SSTL_TEST_EQ(sstl::get<2>(var3), 9l);
  sstl::variant4<int, char, long, unsigned> var4(11u);
  SSTL_TEST_EQ(var4.index(), 3u);
  SSTL_TEST_ASSERT(sstl::holds_alternative<3>(var4));
  int visited = 0;
  var4.visit(variant_visit_counter(&visited));
  SSTL_TEST_EQ(visited, 1);

  sstl::function1<int, int, 32> fn(plus_one);
  SSTL_TEST_ASSERT(fn);
  SSTL_TEST_EQ(fn(8), 9);
  sstl::function1<int, int, 32> stateful(stateful_adder(5));
  SSTL_TEST_EQ(stateful(8), 13);
  sstl::function2<int, int, int, 32> fn2(add_two);
  SSTL_TEST_EQ(fn2(3, 4), 7);
  sstl::function3<int, int, int, int, 32> fn3(add_three);
  SSTL_TEST_EQ(fn3(1, 2, 3), 6);
}

static void iterator_and_functional_contracts() {
  sstl_test::noalloc_guard guard;
  require_iterator_nested_typedefs<sstl::array<int, 3>::iterator>();
  require_iterator_nested_typedefs<sstl::array<int, 3>::const_iterator>();
  require_iterator_nested_typedefs<sstl::span<int>::iterator>();
  require_iterator_nested_typedefs<sstl::span<int>::const_iterator>();
  require_iterator_nested_typedefs<sstl::vector<int, 3>::iterator>();
  require_iterator_nested_typedefs<sstl::vector<int, 3>::const_iterator>();
  require_iterator_nested_typedefs<sstl::string<8>::iterator>();
  require_iterator_nested_typedefs<sstl::string<8>::const_iterator>();
  require_iterator_nested_typedefs<sstl::deque<int, 3>::iterator>();
  require_iterator_nested_typedefs<sstl::deque<int, 3>::const_iterator>();
  require_iterator_nested_typedefs<sstl::forward_list<int, 3>::iterator>();
  require_iterator_nested_typedefs<sstl::forward_list<int, 3>::const_iterator>();
  require_iterator_nested_typedefs<sstl::list<int, 3>::iterator>();
  require_iterator_nested_typedefs<sstl::list<int, 3>::const_iterator>();
  require_iterator_nested_typedefs<sstl::map<int, int, 3>::iterator>();
  require_iterator_nested_typedefs<sstl::map<int, int, 3>::const_iterator>();
  require_iterator_nested_typedefs<sstl::unordered_map<int, int, 3, 7>::iterator>();
  require_iterator_nested_typedefs<sstl::unordered_map<int, int, 3, 7>::const_iterator>();
  require_iterator_nested_typedefs<sstl::unordered_set<int, 3, 7>::iterator>();
  require_iterator_nested_typedefs<sstl::unordered_set<int, 3, 7>::const_iterator>();

  sstl::forward_list<int, 3> flist;
  SSTL_TEST_ASSERT(flist.insert_after(flist.before_begin(), 1) != flist.end());
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(flist, flist.before_begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(flist, flist.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(flist, flist.end()));

  sstl::map<int, int, 3> hook_map;
  SSTL_TEST_ASSERT(hook_map.insert(sstl::make_pair(1, 10)).second);
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(hook_map, hook_map.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(hook_map, hook_map.end()));

  sstl::flat_map<int, int, 3> hook_flat_map;
  SSTL_TEST_ASSERT(hook_flat_map.insert(sstl::make_pair(2, 20)).second);
  hook_flat_map.revalidate_iterators();
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(hook_flat_map, hook_flat_map.begin()));
  SSTL_TEST_ASSERT(SSTL_TEST_ITERATOR_IS_VALID(hook_flat_map, hook_flat_map.end()));

  SSTL_TEST_ASSERT(sstl::not_equal_to<int>()(1, 2));
  SSTL_TEST_EQ(sstl::plus<int>()(2, 3), 5);
  SSTL_TEST_EQ(sstl::minus<int>()(7, 3), 4);
  SSTL_TEST_EQ(sstl::multiplies<int>()(3, 4), 12);
  SSTL_TEST_EQ(sstl::divides<int>()(8, 2), 4);
  SSTL_TEST_EQ(sstl::modulus<int>()(9, 4), 1);
  SSTL_TEST_EQ(sstl::negate<int>()(5), -5);
  SSTL_TEST_ASSERT(sstl::logical_and<bool>()(true, true));
  SSTL_TEST_ASSERT(sstl::logical_or<bool>()(false, true));
  SSTL_TEST_ASSERT(sstl::logical_not<bool>()(false));
  SSTL_TEST_EQ(sstl::bit_and<unsigned>()(6u, 3u), 2u);
  SSTL_TEST_EQ(sstl::bit_or<unsigned>()(4u, 1u), 5u);
  SSTL_TEST_EQ(sstl::bit_xor<unsigned>()(6u, 3u), 5u);
  SSTL_TEST_EQ(sstl::bit_not<unsigned>()(0u), ~0u);
}

int main() {
  const sstl_test::test_case tests[] = {
    {"contiguous_and_deque_contracts", contiguous_and_deque_contracts},
    {"ordered_and_unordered_contracts", ordered_and_unordered_contracts},
    {"try_and_capacity_contracts_cover_zero_and_one", try_and_capacity_contracts_cover_zero_and_one},
    {"linked_and_heap_contracts", linked_and_heap_contracts},
    {"utility_family_contracts", utility_family_contracts},
    {"iterator_and_functional_contracts", iterator_and_functional_contracts}
  };
  return sstl_test::run_all(tests, sizeof(tests) / sizeof(tests[0]));
}
