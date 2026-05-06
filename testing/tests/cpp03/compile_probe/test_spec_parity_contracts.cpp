// test_id: cpp03.spec_parity_contracts
// req_ids: [REQ-03, REQ-04, REQ-05, REQ-06]
// api_ids: [cpp.deque.contract, cpp.list.contract, cpp.forward_list.contract, cpp.map.contract, cpp.set.contract, cpp.flat_map.contract, cpp.flat_set.contract, cpp.unordered_map.contract, cpp.unordered_set.contract, cpp.optional.contract, cpp.priority_queue.contract, cpp.queue.contract, cpp.stack.contract, cpp.string.pop_back, cpp.iterator.contract, cpp.utility.non_member_hooks]
// patterns: [STL-COMPILE-PARITY, CAPACITY-BOUNDARY, EDGE-BRANCH-COVERAGE]

#define SSTL_NO_DEFAULT_PANIC 1

#include <cstdio>
#include <cstdlib>

#include <sstl/array.hpp>
#include <sstl/deque.hpp>
#include <sstl/flat_map.hpp>
#include <sstl/flat_set.hpp>
#include <sstl/forward_list.hpp>
#include <sstl/list.hpp>
#include <sstl/map.hpp>
#include <sstl/optional.hpp>
#include <sstl/priority_queue.hpp>
#include <sstl/queue.hpp>
#include <sstl/set.hpp>
#include <sstl/span.hpp>
#include <sstl/string.hpp>
#include <sstl/string_view.hpp>
#include <sstl/unordered_map.hpp>
#include <sstl/unordered_set.hpp>
#include <sstl/stack.hpp>

#include "test_harness.hpp"

extern "C" void sstl_panic(const char* msg) {
  std::fprintf(stderr, "sstl_panic: %s\n", msg);
  std::exit(2);
}

namespace {

void exercise_sequence_parity() {
  sstl::array<int, 2> ar;
  ar[0] = 7;
  ar[1] = 8;
  SSTL_TEST_ASSERT(ar.try_at(1) != 0);
  SSTL_TEST_EQ(*ar.try_at(1), 8);
  SSTL_TEST_ASSERT(ar.try_at(2) == 0);

  sstl::deque<int, 8> dq;
  SSTL_TEST_EQ(dq.max_size(), 8u);
  SSTL_TEST_ASSERT(dq.push_back(1));
  SSTL_TEST_ASSERT(dq.push_back(4));
  int middle[] = {2, 3};
  (void)dq.insert(dq.begin() + 1, middle, middle + 2);
  (void)dq.insert(dq.end(), 2u, 5);
  SSTL_TEST_EQ(dq.at(2), 3);
  SSTL_TEST_ASSERT(dq.try_at(2) != 0);
  SSTL_TEST_EQ(*dq.try_at(2), 3);
  SSTL_TEST_ASSERT(dq.try_at(99) == 0);
  sstl::deque<int, 8>::const_iterator dc = dq.cbegin();
  sstl::deque<int, 8>::const_reverse_iterator dr = dq.crbegin();
  SSTL_TEST_EQ(*dc, 1);
  SSTL_TEST_EQ(*dr, 5);
  (void)dq.erase(dq.begin() + 1, dq.begin() + 3);
  sstl::deque<int, 8> dq2(dq);
  SSTL_TEST_ASSERT(dq == dq2);
  SSTL_TEST_EQ(sstl::distance(dq.begin(), dq.end()), 4);
  sstl::deque<int, 8>::iterator moved = dq.begin();
  sstl::advance(moved, 2);
  SSTL_TEST_EQ(*moved, 5);
  SSTL_TEST_EQ(*sstl::next(dq.begin(), 1), 4);
  SSTL_TEST_EQ(*sstl::prev(dq.end(), 1), 5);

  sstl::deque<int, 4> try_dq;
  int try_tail[] = {4};
  SSTL_TEST_ASSERT(try_dq.push_back(1));
  SSTL_TEST_ASSERT(try_dq.push_back(3));
  SSTL_TEST_ASSERT(try_dq.try_insert(try_dq.begin() + 1, 2) != try_dq.end());
  sstl::deque<int, 4>::iterator try_range_result = try_dq.try_insert(try_dq.end(), try_tail, try_tail + 1);
  SSTL_TEST_ASSERT(try_range_result != try_dq.end());
  SSTL_TEST_ASSERT(try_dq.try_insert(try_dq.end(), 1u, 5) == try_dq.end());
  SSTL_TEST_EQ(*try_dq.try_at(3), 4);

  sstl::list<int, 8> li;
  int values[] = {1, 2, 2, 3};
  (void)li.insert(li.begin(), values, values + 4);
  li.unique(sstl::equal_to<int>());
  li.resize(5, 9);
  sstl::list<int, 8>::const_reverse_iterator lr = li.crbegin();
  SSTL_TEST_EQ(*lr, 9);
  (void)li.erase(li.begin(), li.end());
  SSTL_TEST_ASSERT(li.empty());

  sstl::forward_list<int, 8> fl;
  sstl::forward_list<int, 8>::iterator tail = fl.before_begin();
  tail = fl.insert_after(tail, values, values + 4);
  (void)tail;
  fl.unique(sstl::equal_to<int>());
  fl.reverse();
  SSTL_TEST_ASSERT(fl.cbefore_begin() != fl.cbegin());
  (void)fl.erase_after(fl.before_begin(), fl.end());
}

void exercise_string_view_parity() {
  sstl::string<16> text("abc");
  sstl::string<4> pop_text("xy");
  SSTL_TEST_ASSERT(pop_text.try_pop_back());
  SSTL_TEST_EQ(pop_text.size(), 1u);
  SSTL_TEST_EQ(pop_text[0], 'x');
  pop_text.pop_back();
  SSTL_TEST_ASSERT(pop_text.empty());
  SSTL_TEST_ASSERT(!pop_text.try_pop_back());
  SSTL_TEST_ASSERT(text.append(2u, 'd'));
  SSTL_TEST_ASSERT(text.insert(1u, 2u, 'x'));
  SSTL_TEST_ASSERT(text.replace(1u, 2u, "bc", 2u));
  sstl::string<16> suffix("dd");
  SSTL_TEST_ASSERT(text.find(suffix) != sstl::npos);
  SSTL_TEST_ASSERT(text.substr(1u) == "bcbcdd");

  sstl::string_view view(text.c_str(), text.size());
  char copied[8] = {};
  SSTL_TEST_EQ(view.length(), text.size());
  SSTL_TEST_EQ(view.size_bytes(), text.size());
  SSTL_TEST_EQ(view.copy(copied, 3u), 3u);
  SSTL_TEST_ASSERT(view.substr(1u, 2u) == sstl::string_view("bc", 2u));
  SSTL_TEST_ASSERT(view.try_at(1u) != 0);
  SSTL_TEST_EQ(*view.try_at(1u), 'b');
  SSTL_TEST_ASSERT(view.try_at(view.size()) == 0);
  SSTL_TEST_ASSERT(view.try_front() != 0);
  SSTL_TEST_EQ(*view.try_front(), 'a');
  SSTL_TEST_ASSERT(view.try_back() != 0);
  SSTL_TEST_EQ(*view.try_back(), 'd');
  SSTL_TEST_ASSERT(view.starts_with('a'));
  SSTL_TEST_ASSERT(view.ends_with(sstl::string_view("dd")));
  SSTL_TEST_ASSERT(view.find(sstl::string_view("bc")) != sstl::npos);
  SSTL_TEST_ASSERT(view.rfind('d') != sstl::npos);
  sstl::string_view::const_reverse_iterator rv = view.crbegin();
  SSTL_TEST_EQ(*rv, 'd');
}

void exercise_span_const_parity() {
  int values[] = {1, 2, 3};
  const sstl::span<int> view(values, 3u);
  view[0] = 4;
  view.at(1) = 5;
  *view.begin() = 6;
  *view.rbegin() = 7;
  *view.try_front() = 8;
  sstl::span<int> first = view.first(2u);
  first[1] = 9;
  sstl::span<int>::const_iterator ci = view.cbegin();
  sstl::span<int>::const_reverse_iterator cr = view.crbegin();
  SSTL_TEST_EQ(*ci, 8);
  SSTL_TEST_EQ(*cr, 7);
  SSTL_TEST_EQ(values[1], 9);
}

void exercise_associative_parity() {
  sstl::pair<int, int> pairs[] = {
    sstl::make_pair(1, 10),
    sstl::make_pair(2, 20)
  };

  sstl::map<int, int, 8> m;
  m.insert(pairs, pairs + 2);
  (void)m.insert(m.begin(), sstl::make_pair(3, 30));
  SSTL_TEST_EQ(m.at(2), 20);
  SSTL_TEST_ASSERT(m.try_at(2) != 0);
  SSTL_TEST_EQ(*m.try_at(2), 20);
  SSTL_TEST_ASSERT(m.try_at(99) == 0);
  const sstl::map<int, int, 8>& cm = m;
  SSTL_TEST_EQ(cm.at(3), 30);
  SSTL_TEST_ASSERT(cm.try_at(3) != 0);
  SSTL_TEST_EQ(*cm.try_at(3), 30);
  SSTL_TEST_ASSERT(cm.try_at(99) == 0);
  SSTL_TEST_EQ(m.key_comp()(1, 2), true);
  SSTL_TEST_ASSERT(m.value_comp()(*m.begin(), *m.rbegin()));
  (void)m.erase(m.begin(), m.end());
  SSTL_TEST_ASSERT(m.empty());

  sstl::set<int, 8> s;
  int keys[] = {3, 1, 2};
  s.insert(keys, keys + 3);
  (void)s.insert(s.begin(), 4);
  SSTL_TEST_EQ(*s.cbegin(), 1);
  SSTL_TEST_EQ(*s.crbegin(), 4);
  s.clear();

  sstl::flat_map<int, int, 8> fm;
  fm.insert(pairs, pairs + 2);
  (void)fm.insert(fm.begin(), sstl::make_pair(3, 30));
  SSTL_TEST_EQ(fm.count(2), 1u);
  SSTL_TEST_EQ(fm.at(1), 10);
  SSTL_TEST_ASSERT(fm.try_at(1) != 0);
  SSTL_TEST_EQ(*fm.try_at(1), 10);
  SSTL_TEST_ASSERT(fm.try_at(99) == 0);
  const sstl::flat_map<int, int, 8>& cfm = fm;
  SSTL_TEST_EQ(cfm.at(3), 30);
  SSTL_TEST_ASSERT(cfm.try_at(3) != 0);
  SSTL_TEST_EQ(*cfm.try_at(3), 30);
  SSTL_TEST_ASSERT(cfm.try_at(99) == 0);
  SSTL_TEST_EQ(*fm.crbegin(), sstl::make_pair(3, 30));
  (void)fm.erase(fm.begin(), fm.end());
  SSTL_TEST_ASSERT(fm.empty());

  sstl::flat_set<int, 8> fs;
  fs.insert(keys, keys + 3);
  SSTL_TEST_EQ(fs.count(2), 1u);
  SSTL_TEST_EQ(*fs.crbegin(), 3);
  fs.clear();
}

void exercise_unordered_optional_adapter_parity() {
  sstl::unordered_map<int, int, 8> um;
  (void)um.insert(sstl::make_pair(1, 10));
  (void)um.insert(um.begin(), sstl::make_pair(2, 20));
  SSTL_TEST_EQ(um.count(1), 1u);
  SSTL_TEST_EQ(um.at(2), 20);
  SSTL_TEST_ASSERT(um.try_at(2) != 0);
  SSTL_TEST_EQ(*um.try_at(2), 20);
  SSTL_TEST_ASSERT(um.try_at(99) == 0);
  SSTL_TEST_ASSERT(um.equal_range(1).first != um.end());
  SSTL_TEST_ASSERT(um.bucket_size(um.bucket(1)) >= 1u);
  SSTL_TEST_ASSERT(um.begin(um.bucket(1)) != um.end(um.bucket(1)));
  SSTL_TEST_ASSERT(um.max_bucket_count() == um.bucket_count());
  SSTL_TEST_ASSERT(um.max_load_factor() >= um.load_factor());
  {
    sstl::unordered_map<int, int, 4, 5> lhs;
    sstl::unordered_map<int, int, 4, 7> same;
    sstl::unordered_map<int, int, 5, 7> different;
    (void)lhs.insert(sstl::make_pair(1, 10));
    (void)lhs.insert(sstl::make_pair(2, 20));
    (void)same.insert(sstl::make_pair(2, 20));
    (void)same.insert(sstl::make_pair(1, 10));
    (void)different.insert(sstl::make_pair(1, 10));
    (void)different.insert(sstl::make_pair(3, 30));
    SSTL_TEST_ASSERT(lhs == same);
    SSTL_TEST_ASSERT(!(lhs != same));
    SSTL_TEST_ASSERT(lhs != different);
  }

  sstl::unordered_set<int, 8> us;
  int keys[] = {1, 2, 3};
  us.insert(keys, keys + 3);
  SSTL_TEST_EQ(us.count(2), 1u);
  SSTL_TEST_ASSERT(us.equal_range(2).first != us.end());
  SSTL_TEST_ASSERT(us.begin(us.bucket(2)) != us.end(us.bucket(2)));
  {
    sstl::unordered_set<int, 4, 5> lhs;
    sstl::unordered_set<int, 4, 7> same;
    sstl::unordered_set<int, 5, 7> different;
    SSTL_TEST_ASSERT(lhs.insert(1).second);
    SSTL_TEST_ASSERT(lhs.insert(2).second);
    SSTL_TEST_ASSERT(same.insert(2).second);
    SSTL_TEST_ASSERT(same.insert(1).second);
    SSTL_TEST_ASSERT(different.insert(1).second);
    SSTL_TEST_ASSERT(different.insert(3).second);
    SSTL_TEST_ASSERT(lhs == same);
    SSTL_TEST_ASSERT(!(lhs != same));
    SSTL_TEST_ASSERT(lhs != different);
  }

  sstl::optional<int> a(1);
  sstl::optional<int> b(2);
  sstl::optional<int> made = sstl::make_optional(7);
  SSTL_TEST_EQ(made.value(), 7);
  SSTL_TEST_ASSERT(a < b);
  a.swap(b);
  SSTL_TEST_EQ(a.value(), 2);
  swap(a, b);
  SSTL_TEST_EQ(a.value(), 1);

  sstl::stack<int, 4> st;
  sstl::stack<int, 5> st_same;
  sstl::stack<int, 5> st_greater;
  sstl::queue<int, 4> qu;
  sstl::queue<int, 5> qu_same;
  sstl::queue<int, 5> qu_greater;
  int pq_values[] = {3, 1, 4};
  sstl::vector<int, 4> pq_storage;
  SSTL_TEST_ASSERT(st.push(1));
  SSTL_TEST_ASSERT(st_same.push(1));
  SSTL_TEST_ASSERT(st_greater.push(2));
  SSTL_TEST_ASSERT(st == st_same);
  SSTL_TEST_ASSERT(st != st_greater);
  SSTL_TEST_ASSERT(st < st_greater);
  SSTL_TEST_ASSERT(st <= st_same);
  SSTL_TEST_ASSERT(st_greater > st);
  SSTL_TEST_ASSERT(st_same >= st);
  SSTL_TEST_ASSERT(st.try_top() != 0);
  SSTL_TEST_EQ(*st.try_top(), 1);
  {
    int popped = 0;
    SSTL_TEST_ASSERT(st.try_pop(&popped));
    SSTL_TEST_EQ(popped, 1);
    SSTL_TEST_ASSERT(st.try_top() == 0);
    SSTL_TEST_ASSERT(st.try_push(1));
  }
  SSTL_TEST_ASSERT(qu.push(1));
  SSTL_TEST_ASSERT(qu_same.push(1));
  SSTL_TEST_ASSERT(qu_greater.push(2));
  SSTL_TEST_ASSERT(qu == qu_same);
  SSTL_TEST_ASSERT(qu != qu_greater);
  SSTL_TEST_ASSERT(qu < qu_greater);
  SSTL_TEST_ASSERT(qu <= qu_same);
  SSTL_TEST_ASSERT(qu_greater > qu);
  SSTL_TEST_ASSERT(qu_same >= qu);
  SSTL_TEST_ASSERT(qu.try_front() != 0);
  SSTL_TEST_ASSERT(qu.try_back() != 0);
  SSTL_TEST_EQ(*qu.try_front(), 1);
  SSTL_TEST_EQ(*qu.try_back(), 1);
  {
    int popped = 0;
    SSTL_TEST_ASSERT(qu.try_pop(&popped));
    SSTL_TEST_EQ(popped, 1);
    SSTL_TEST_ASSERT(qu.try_front() == 0);
    SSTL_TEST_ASSERT(qu.try_push(1));
  }
  sstl::priority_queue<int, 4> pq;
  sstl::priority_queue<int, 4> pq_range(pq_values, pq_values + 3);
  sstl::priority_queue<int, 4, sstl::vector<int, 4>, sstl::greater<int> > pq_min(
    pq_values, pq_values + 3, sstl::greater<int>());
  SSTL_TEST_ASSERT(pq_storage.push_back(2));
  SSTL_TEST_ASSERT(pq_storage.push_back(5));
  sstl::priority_queue<int, 4> pq_from_container(sstl::less<int>(), pq_storage);
  SSTL_TEST_EQ(st.capacity(), 4u);
  SSTL_TEST_EQ(qu.capacity(), 4u);
  SSTL_TEST_EQ(pq.capacity(), 4u);
  SSTL_TEST_EQ(pq_range.top(), 4);
  SSTL_TEST_EQ(pq_min.top(), 1);
  SSTL_TEST_EQ(pq_from_container.top(), 5);
  SSTL_TEST_ASSERT(pq.try_top() == 0);
  SSTL_TEST_ASSERT(pq.try_push(6));
  SSTL_TEST_ASSERT(pq.try_top() != 0);
  SSTL_TEST_EQ(*pq.try_top(), 6);
  {
    int popped = 0;
    SSTL_TEST_ASSERT(pq.try_pop(&popped));
    SSTL_TEST_EQ(popped, 6);
    SSTL_TEST_ASSERT(pq.try_top() == 0);
  }
  SSTL_TEST_ASSERT(!st.full());
  SSTL_TEST_ASSERT(!qu.full());
  SSTL_TEST_ASSERT(!pq.full());

  sstl::hash<int> int_hash;
  sstl::hash<int*> ptr_hash;
  sstl::hash<sstl::string<8> > string_hash;
  int hash_value = 3;
  sstl::string<8> hash_text("abc");
  SSTL_TEST_ASSERT(int_hash(42) != 0u);
  SSTL_TEST_ASSERT(ptr_hash(&hash_value) != 0u);
  SSTL_TEST_ASSERT(string_hash(hash_text) != 0u);
}

}  // namespace

int main() {
  exercise_sequence_parity();
  exercise_string_view_parity();
  exercise_span_const_parity();
  exercise_associative_parity();
  exercise_unordered_optional_adapter_parity();
  return 0;
}
