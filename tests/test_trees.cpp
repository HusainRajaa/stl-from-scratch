#include <stl/rb_tree.hpp>
#include <stl/splay_tree.hpp>

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <iostream>
#include <iterator>
#include <memory>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

#define CHECK(...)                                                             \
  do {                                                                         \
    if (!(__VA_ARGS__)) {                                                      \
      throw std::runtime_error(std::string(__FILE__) + ":" +                   \
                               std::to_string(__LINE__) +                      \
                               ": CHECK(" #__VA_ARGS__ ") failed");            \
    }                                                                          \
  } while (false)

template <class Tree, class Compare>
void check_equal(const Tree &tree, const std::set<int, Compare> &expected) {
  CHECK(tree.validate());
  CHECK(tree.size() == expected.size());
  CHECK(tree.empty() == expected.empty());
  auto actual = tree.begin();
  for (int value : expected) {
    CHECK(actual != tree.end());
    CHECK(*actual == value);
    ++actual;
  }
  CHECK(actual == tree.end());

  auto backwards = tree.end();
  for (auto it = expected.rbegin(); it != expected.rend(); ++it) {
    CHECK(backwards != tree.begin());
    --backwards;
    CHECK(*backwards == *it);
  }
  CHECK(backwards == tree.begin());

  using iterator = decltype(tree.begin());
  std::reverse_iterator<iterator> reverse(tree.end());
  const std::reverse_iterator<iterator> reverse_end(tree.begin());
  for (auto it = expected.rbegin(); it != expected.rend(); ++it) {
    CHECK(reverse != reverse_end);
    CHECK(*reverse == *it);
    ++reverse;
  }
  CHECK(reverse == reverse_end);
}

template <class Tree, class Compare>
void differential(Compare compare, std::uint32_t seed) {
  using iterator = decltype(std::declval<Tree &>().begin());
  using const_iterator = decltype(std::declval<const Tree &>().begin());
  static_assert(std::bidirectional_iterator<iterator>);
  static_assert(std::bidirectional_iterator<const_iterator>);
  static_assert(std::is_const_v<
                std::remove_reference_t<decltype(*std::declval<iterator>())>>);
  static_assert(
      std::is_same_v<typename std::iterator_traits<iterator>::value_type, int>);

  Tree tree(compare);
  std::set<int, Compare> expected(compare);
  check_equal(tree, expected);
  CHECK(tree.find(10) == tree.end());
  CHECK(!tree.contains(10));
  CHECK(tree.erase(10) == 0);

  std::mt19937 random(seed);
  for (int step = 0; step < 6000; ++step) {
    int key = static_cast<int>(random() % 501) - 250;
    switch (random() % 6) {
    case 0:
    case 1: {
      auto actual = tree.insert(key);
      auto reference = expected.insert(key);
      CHECK(actual.second == reference.second);
      CHECK(*actual.first == *reference.first);
      break;
    }
    case 2:
      CHECK(tree.erase(key) == expected.erase(key));
      break;
    case 3: {
      auto actual = tree.find(key);
      auto reference = expected.find(key);
      CHECK((actual == tree.end()) == (reference == expected.end()));
      if (reference != expected.end())
        CHECK(*actual == *reference);
      CHECK(tree.contains(key) == expected.contains(key));
      break;
    }
    case 4: {
      const Tree &constant = tree;
      auto actual = constant.find(key);
      auto reference = expected.find(key);
      CHECK((actual == constant.end()) == (reference == expected.end()));
      if (reference != expected.end())
        CHECK(*actual == *reference);
      CHECK(constant.contains(key) == expected.contains(key));
      break;
    }
    case 5:
      if (!expected.empty()) {
        auto offset = static_cast<std::ptrdiff_t>(random() % expected.size());
        auto actual = tree.begin();
        auto reference = expected.begin();
        std::advance(actual, offset);
        std::advance(reference, offset);
        auto expected_next = expected.erase(reference);
        auto next = tree.erase(actual);
        CHECK((next == tree.end()) == (expected_next == expected.end()));
        if (expected_next != expected.end())
          CHECK(*next == *expected_next);
      }
      break;
    }
    check_equal(tree, expected);
  }

  if (tree.size() >= 2) {
    auto it = tree.begin();
    auto old = it++;
    CHECK(old == tree.begin());
    CHECK(old != it);
    auto second = it--;
    CHECK(it == tree.begin());
    CHECK(second != it);
    CHECK(*it.operator->() == *it);
  }

  Tree copy(tree);
  check_equal(copy, expected);
  copy.insert(10000);
  CHECK(!tree.contains(10000));
  CHECK(copy.validate());
  copy = tree;
  check_equal(copy, expected);
  Tree &copy_alias = copy;
  copy = copy_alias;
  check_equal(copy, expected);

  Tree moved(std::move(copy));
  check_equal(moved, expected);
  CHECK(copy.validate());
  copy.clear();
  CHECK(copy.insert(42).second);
  CHECK(copy.contains(42));
  CHECK(copy.validate());
  copy.clear();
  CHECK(copy.empty());

  Tree assigned(compare);
  assigned.insert(9999);
  assigned = std::move(moved);
  check_equal(assigned, expected);
  CHECK(moved.validate());
  moved.clear();
  CHECK(moved.insert(17).second);
  CHECK(moved.validate());

  for (auto it = assigned.begin(); it != assigned.end();) {
    it = assigned.erase(it);
    CHECK(assigned.validate());
  }
  CHECK(assigned.empty());
  CHECK(assigned.begin() == assigned.end());
  assigned.clear();
  assigned.clear();
  CHECK(assigned.insert(9).second);
  CHECK(assigned.erase(9) == 1);
  CHECK(assigned.empty());
  CHECK(assigned.validate());

  Tree empty(compare);
  Tree empty_copy(empty);
  Tree empty_move(std::move(empty_copy));
  CHECK(empty.validate());
  CHECK(empty_copy.validate());
  CHECK(empty_move.validate());
  tree = empty;
  CHECK(tree.empty());
  CHECK(tree.validate());
}

struct Direction {
  bool descending;
  bool operator()(int a, int b) const noexcept {
    return descending ? a > b : a < b;
  }
};

struct Key {
  int value;
  Key() = delete;
  explicit Key(int v) : value(v) {}
};

struct KeyLess {
  bool operator()(const Key &a, const Key &b) const noexcept {
    return a.value < b.value;
  }
};

template <class Tree> void non_default_key() {
  Tree tree;
  CHECK(tree.insert(Key(3)).second);
  Key one(1);
  CHECK(tree.insert(one).second);
  CHECK(tree.insert(Key(2)).second);
  CHECK(!tree.insert(Key(2)).second);
  CHECK(tree.size() == 3);
  CHECK(tree.find(Key(2))->value == 2);
  CHECK(tree.contains(Key(1)));
  const Tree &constant = tree;
  CHECK(constant.find(Key(3))->value == 3);
  CHECK(constant.contains(Key(2)));
  CHECK(tree.validate());
  Tree copy(tree);
  CHECK(copy.size() == 3);
  CHECK(copy.validate());
  CHECK(tree.erase(Key(2)) == 1);
  auto it = tree.begin();
  CHECK(it->value == 1);
  it = tree.erase(it);
  CHECK(it->value == 3);
  CHECK(tree.erase(it) == tree.end());
  CHECK(tree.empty());
  CHECK(tree.validate());
}

struct CopyFailure : std::exception {};

struct ThrowingKey {
  inline static int live = 0;
  inline static int copies_left = -1;
  int value;

  ThrowingKey() = delete;
  explicit ThrowingKey(int v) : value(v) { ++live; }
  ThrowingKey(const ThrowingKey &other) : value(other.value) {
    if (copies_left == 0)
      throw CopyFailure{};
    if (copies_left > 0)
      --copies_left;
    ++live;
  }
  ThrowingKey(ThrowingKey &&other) noexcept : value(other.value) { ++live; }
  ThrowingKey &operator=(const ThrowingKey &) = delete;
  ThrowingKey &operator=(ThrowingKey &&) = delete;
  ~ThrowingKey() { --live; }
};

struct ThrowingKeyLess {
  bool operator()(const ThrowingKey &a, const ThrowingKey &b) const noexcept {
    return a.value < b.value;
  }
};

template <class Tree> std::vector<int> values(const Tree &tree) {
  std::vector<int> result;
  for (const auto &key : tree)
    result.push_back(key.value);
  return result;
}

template <class Tree> void copy_exception_safety() {
  CHECK(ThrowingKey::live == 0);
  {
    Tree source;
    for (int i = 0; i < 24; ++i)
      source.insert(ThrowingKey(i));
    const auto original = values(source);
    Tree destination;
    destination.insert(ThrowingKey(100));
    const auto old_destination = values(destination);
    const int baseline = ThrowingKey::live;

    for (int fail_after = 0; fail_after < 24; ++fail_after) {
      ThrowingKey::copies_left = fail_after;
      bool threw = false;
      try {
        Tree copy(source);
      } catch (const CopyFailure &) {
        threw = true;
      }
      ThrowingKey::copies_left = -1;
      CHECK(threw);
      CHECK(ThrowingKey::live == baseline);
      CHECK(values(source) == original);
      CHECK(source.validate());

      ThrowingKey::copies_left = fail_after;
      threw = false;
      try {
        destination = source;
      } catch (const CopyFailure &) {
        threw = true;
      }
      ThrowingKey::copies_left = -1;
      CHECK(threw);
      CHECK(ThrowingKey::live == baseline);
      CHECK(values(destination) == old_destination);
      CHECK(values(source) == original);
      CHECK(destination.validate());
      CHECK(source.validate());
    }

    ThrowingKey key(50);
    ThrowingKey::copies_left = 0;
    bool threw = false;
    try {
      source.insert(key);
    } catch (const CopyFailure &) {
      threw = true;
    }
    ThrowingKey::copies_left = -1;
    CHECK(threw);
    CHECK(ThrowingKey::live == baseline + 1);
    CHECK(values(source) == original);
    CHECK(source.validate());
    CHECK(source.insert(key).second);
    CHECK(source.validate());
    destination = source;
    CHECK(values(destination) == values(source));
    CHECK(destination.validate());
  }
  CHECK(ThrowingKey::live == 0);
}

template <class Tree> void skew_stress() {
  constexpr int count = 60000;
  Tree tree;
  for (int i = 0; i < count; ++i)
    CHECK(tree.insert(i).second);
  CHECK(tree.size() == static_cast<std::size_t>(count));
  CHECK(tree.validate());
  int expected = 0;
  for (int value : tree)
    CHECK(value == expected++);
  CHECK(expected == count);
  auto it = tree.end();
  for (int i = count; i > 0; --i)
    CHECK(*--it == i - 1);
  CHECK(it == tree.begin());

  {
    Tree copy(tree);
    CHECK(copy.size() == tree.size());
    CHECK(copy.validate());
  }
  for (int i = 0; i < count; ++i)
    CHECK(tree.find(i) != tree.end());
  CHECK(tree.validate());
  for (int i = count - 1; i >= 0; --i)
    CHECK(tree.find(i) != tree.end());
  CHECK(tree.validate());
  tree.clear();
  CHECK(tree.empty());
  CHECK(tree.validate());

  for (int i = count - 1; i >= 0; --i)
    CHECK(tree.insert(i).second);
  CHECK(tree.validate());
  for (int i = 0; i < count; ++i) {
    CHECK(tree.erase(i) == 1);
    if (i % 1024 == 0)
      CHECK(tree.validate());
  }
  CHECK(tree.empty());
  CHECK(tree.validate());
  CHECK(tree.insert(123).second);
  CHECK(tree.validate());

  Tree destroy_skew;
  for (int i = 0; i < count; ++i)
    destroy_skew.insert(i);
  CHECK(destroy_skew.validate());
}

struct CountingLess {
  std::shared_ptr<std::size_t> calls;
  bool operator()(int a, int b) const {
    ++*calls;
    return a < b;
  }
};

void splay_access_behavior() {
  auto calls = std::make_shared<std::size_t>(0);
  stl::splay_tree<int, CountingLess> tree(CountingLess{calls});
  for (int i = 0; i < 100; ++i)
    tree.insert(i);

  const auto &constant = tree;
  CHECK(*constant.find(0) == 0);
  *calls = 0;
  CHECK(*tree.find(99) == 99);
  CHECK(*calls == 2);

  CHECK(*tree.find(25) == 25);
  *calls = 0;
  CHECK(*tree.find(25) == 25);
  CHECK(*calls == 2);

  CHECK(tree.find(-1) == tree.end());
  *calls = 0;
  CHECK(*tree.find(0) == 0);
  CHECK(*calls == 2);
  CHECK(tree.find(100) == tree.end());
  *calls = 0;
  CHECK(*tree.find(99) == 99);
  CHECK(*calls == 2);

  CHECK(!tree.insert(42).second);
  *calls = 0;
  CHECK(*tree.find(42) == 42);
  CHECK(*calls == 2);
  CHECK(tree.validate());
}

template <template <class, class> class Tree>
void run_tree_tests(const char *name) {
  std::cout << "Testing " << name << "..." << std::endl;
  differential<Tree<int, std::less<int>>>(std::less<int>{}, 0x12345678u);
  differential<Tree<int, std::greater<int>>>(std::greater<int>{}, 0x9abcdef0u);
  differential<Tree<int, Direction>>(Direction{true}, 0x31415926u);
  non_default_key<Tree<Key, KeyLess>>();
  copy_exception_safety<Tree<ThrowingKey, ThrowingKeyLess>>();
  skew_stress<Tree<int, std::less<int>>>();
  std::cout << name << " passed" << std::endl;
}

}

int main() {
  try {
    run_tree_tests<stl::splay_tree>("splay_tree");
    splay_access_behavior();
    run_tree_tests<stl::red_black_tree>("red_black_tree");
    std::cout << "All tree tests passed\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Tree tests failed: " << error.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "Tree tests failed: unknown exception\n";
    return 1;
  }
}
