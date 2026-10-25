#include <stl/segment_tree.hpp>

#include <algorithm>
#include <array>
#include <cstddef>
#include <forward_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace {

[[noreturn]] void fail(const char *expression, const char *file, int line,
                       const std::string &detail = {}) {
  throw std::runtime_error(std::string(file) + ":" + std::to_string(line) +
                           ": " + expression +
                           (detail.empty() ? "" : " (" + detail + ")"));
}

#define CHECK(...)                                                             \
  do {                                                                         \
    if (!(__VA_ARGS__)) {                                                      \
      fail(#__VA_ARGS__, __FILE__, __LINE__);                                  \
    }                                                                          \
  } while (false)

template <class Exception, class Action>
void check_throws(Action &&action, const char *expression, const char *file,
                  int line) {
  try {
    std::forward<Action>(action)();
  } catch (const Exception &) {
    return;
  } catch (const std::exception &error) {
    fail(expression, file, line,
         std::string("wrong exception: ") + error.what());
  } catch (...) {
    fail(expression, file, line, "wrong non-standard exception");
  }
  fail(expression, file, line, "expected exception was not thrown");
}

#define CHECK_THROWS_AS(expression, exception)                                 \
  check_throws<exception>([&] { static_cast<void>(expression); }, #expression, \
                          __FILE__, __LINE__)

using number = long long;
using sum_tree = stl::segment_tree<number>;

number vector_sum(const std::vector<number> &values, std::size_t first,
                  std::size_t last) {
  return std::accumulate(values.begin() + static_cast<std::ptrdiff_t>(first),
                         values.begin() + static_cast<std::ptrdiff_t>(last),
                         number{0});
}

void add(std::vector<number> &values, std::size_t first, std::size_t last,
         number delta) {
  for (auto i = first; i < last; ++i) {
    values[i] += delta;
  }
}

void check_all_sums(const sum_tree &tree, const std::vector<number> &values) {
  CHECK(tree.size() == values.size());
  CHECK(tree.empty() == values.empty());
  for (std::size_t first = 0; first <= values.size(); ++first) {
    for (auto last = first; last <= values.size(); ++last) {
      CHECK(tree.query(first, last) == vector_sum(values, first, last));
    }
  }
}

void test_empty_and_singleton() {
  sum_tree empty(std::vector<number>{}, 0);
  CHECK(empty.size() == 0);
  CHECK(empty.empty());
  CHECK(empty.query(0, 0) == 0);
  empty.range_update(0, 0, -20);
  CHECK_THROWS_AS(empty.query(0, 1), std::out_of_range);
  CHECK_THROWS_AS(empty.query(1, 1), std::out_of_range);
  CHECK_THROWS_AS(empty.query(1, 0), std::out_of_range);
  CHECK_THROWS_AS(empty.range_update(0, 1, 0), std::out_of_range);
  CHECK_THROWS_AS(empty.range_update(1, 1, 0), std::out_of_range);
  CHECK_THROWS_AS(empty.range_update(1, 0, 0), std::out_of_range);
  CHECK_THROWS_AS(empty.point_update(0, 1), std::out_of_range);

  sum_tree singleton(std::vector<number>{-8}, 0);
  CHECK(!singleton.empty());
  CHECK(singleton.query(0, 1) == -8);
  singleton.range_update(0, 1, -4);
  CHECK(singleton.query(0, 1) == -12);
  singleton.point_update(0, 7);
  singleton.range_update(0, 1, 0);
  singleton.range_update(0, 0, 100);
  singleton.range_update(1, 1, 100);
  CHECK(singleton.query(0, 1) == 7);
  CHECK(singleton.query(0, 0) == 0);
  CHECK(singleton.query(1, 1) == 0);
}

void test_boundaries_and_pending_assignment() {
  std::vector<number> values{-4, 2, -7, 9, 3, -1, 5};
  sum_tree tree(values, 0);
  check_all_sums(tree, values);
  tree.range_update(0, values.size(), 11);
  add(values, 0, values.size(), 11);

  check_all_sums(std::as_const(tree), values);
  tree.point_update(3, -50);
  values[3] = -50;
  tree.range_update(1, 6, -9);
  add(values, 1, 6, -9);
  tree.range_update(0, 1, -20);
  values[0] -= 20;
  tree.range_update(6, 7, 30);
  values[6] += 30;
  tree.range_update(0, 7, -11);
  add(values, 0, 7, -11);
  tree.point_update(0, 100);
  values[0] = 100;
  tree.point_update(6, -100);
  values[6] = -100;
  tree.range_update(7, 7, 99);
  check_all_sums(tree, values);

  const auto huge = std::numeric_limits<std::size_t>::max();
  for (const auto &bounds :
       std::vector<std::pair<std::size_t, std::size_t>>{{4, 3},
                                                        {0, 8},
                                                        {8, 8},
                                                        {8, 9},
                                                        {huge, huge},
                                                        {0, huge},
                                                        {huge, 0}}) {
    CHECK_THROWS_AS(tree.query(bounds.first, bounds.second), std::out_of_range);
    CHECK_THROWS_AS(tree.range_update(bounds.first, bounds.second, 0),
                    std::out_of_range);
  }
  CHECK_THROWS_AS(tree.point_update(7, 0), std::out_of_range);
  CHECK_THROWS_AS(tree.point_update(huge, 0), std::out_of_range);
  check_all_sums(tree, values);
}

void test_iterator_construction() {
  const std::forward_list<number> values{2, -5, 9, 4};
  const sum_tree from_forward(values.begin(), values.end(), 0);
  CHECK(from_forward.query(0, 4) == 10);
  CHECK(from_forward.query(1, 3) == 4);

  std::istringstream stream("-3 7 -8 20");
  const sum_tree from_input{std::istream_iterator<number>(stream),
                            std::istream_iterator<number>(), 0};
  CHECK(from_input.query(0, 4) == 16);

  const std::array<number, 3> array{4, -2, 6};
  const sum_tree from_sentinel(std::counted_iterator(array.begin(), 3),
                               std::default_sentinel, 0);
  CHECK(from_sentinel.query(0, 3) == 8);
  const sum_tree empty(array.begin(), array.begin(), 0);
  CHECK(empty.empty());
  CHECK(empty.query(0, 0) == 0);
}

void test_random_sums() {
  std::mt19937 random(0x5E67u);
  std::uniform_int_distribution<int> value(-100, 100);
  for (const std::size_t size : {0u, 1u, 2u, 3u, 7u, 16u, 31u, 64u}) {
    std::vector<number> values(size, 0);
    for (auto &element : values) {
      element = value(random);
    }
    sum_tree tree(values, 0);
    std::uniform_int_distribution<std::size_t> endpoint(0, size);
    for (int step = 0; step < 2000; ++step) {
      auto first = endpoint(random);
      auto last = endpoint(random);
      if (first > last) {
        std::swap(first, last);
      }
      switch (random() % 4) {
      case 0:
        if (size != 0) {
          const auto index = endpoint(random) % size;
          const number replacement = value(random);
          tree.point_update(index, replacement);
          values[index] = replacement;
        }
        break;
      case 1:
      case 2: {
        const number delta = value(random);
        tree.range_update(first, last, delta);
        add(values, first, last, delta);
        break;
      }
      default:
        CHECK(std::as_const(tree).query(first, last) ==
              vector_sum(values, first, last));
      }
      CHECK(tree.query(0, size) == vector_sum(values, 0, size));
      if (step % 97 == 0) {
        check_all_sums(tree, values);
      }
    }
    check_all_sums(tree, values);
  }
}

struct extrema_merge {
  bool minimum;
  explicit extrema_merge(bool use_minimum) : minimum(use_minimum) {}

  number operator()(number left, number right) const {
    return minimum ? std::min(left, right) : std::max(left, right);
  }
};

struct min_add_apply {
  number operator()(number aggregate, number delta, std::size_t) const {
    return aggregate + delta;
  }
};

void test_custom_minimum() {
  using min_tree = stl::segment_tree<number, extrema_merge, min_add_apply>;
  const number identity = std::numeric_limits<number>::max();
  std::vector<number> values{8, -3, 6, 2, -9, 11, 0, 5, 4};
  min_tree tree(values.begin(), values.end(), identity, extrema_merge{true},
                min_add_apply{}, std::plus<number>{});
  std::mt19937 random(0xA11u);
  for (int step = 0; step < 600; ++step) {
    auto first = static_cast<std::size_t>(random() % (values.size() + 1));
    auto last = static_cast<std::size_t>(random() % (values.size() + 1));
    if (first > last) {
      std::swap(first, last);
    }
    const number delta = static_cast<number>(random() % 41) - 20;
    if (step % 4 == 0) {
      const auto index = static_cast<std::size_t>(random() % values.size());
      tree.point_update(index, delta);
      values[index] = delta;
    } else {
      tree.range_update(first, last, delta);
      add(values, first, last, delta);
    }
    for (std::size_t left = 0; left <= values.size(); ++left) {
      number expected = identity;
      CHECK(std::as_const(tree).query(left, left) == identity);
      for (auto right = left + 1; right <= values.size(); ++right) {
        expected = std::min(expected, values[right - 1]);
        CHECK(std::as_const(tree).query(left, right) == expected);
      }
    }
  }
  const min_tree empty(std::vector<number>{}, identity, extrema_merge{true});
  CHECK(empty.query(0, 0) == identity);
}

struct affine_value {
  number sum;
  number multiplier;
  number increment;

  explicit affine_value(number value)
      : sum(value), multiplier(1), increment(0) {}

  static affine_value update(number multiplier, number increment) {
    affine_value result(0);
    result.multiplier = multiplier;
    result.increment = increment;
    return result;
  }
};

static_assert(!std::is_default_constructible_v<affine_value>);

struct affine_merge {
  affine_value operator()(const affine_value &left,
                          const affine_value &right) const {
    return affine_value(left.sum + right.sum);
  }
};

struct affine_apply {
  affine_value operator()(const affine_value &aggregate,
                          const affine_value &update,
                          std::size_t length) const {
    return affine_value(update.multiplier * aggregate.sum +
                        update.increment * static_cast<number>(length));
  }
};

struct affine_compose {
  affine_value operator()(const affine_value &old,
                          const affine_value &next) const {
    return affine_value::update(next.multiplier * old.multiplier,
                                next.multiplier * old.increment +
                                    next.increment);
  }
};

void test_chronological_composition() {
  using affine_tree = stl::segment_tree<affine_value, affine_merge,
                                        affine_apply, affine_compose>;
  std::vector<number> values{1, -2, 3, 4, -5, 6, 7, -8, 9};
  std::vector<affine_value> initial;
  for (const auto value : values) {
    initial.emplace_back(value);
  }
  affine_tree tree(initial, affine_value(0), affine_merge{}, affine_apply{},
                   affine_compose{});
  const auto update = [&](std::size_t first, std::size_t last,
                          number multiplier, number increment) {
    tree.range_update(first, last, affine_value::update(multiplier, increment));
    for (auto i = first; i < last; ++i) {
      values[i] = multiplier * values[i] + increment;
    }
  };
  const auto check = [&] {
    for (std::size_t first = 0; first <= values.size(); ++first) {
      for (auto last = first; last <= values.size(); ++last) {
        CHECK(std::as_const(tree).query(first, last).sum ==
              vector_sum(values, first, last));
      }
    }
  };
  update(0, 9, 2, 3);
  update(0, 9, 3, -4);
  check();
  update(2, 7, -1, 5);
  update(0, 9, 0, 0);
  check();
  tree.point_update(4, affine_value(-17));
  values[4] = -17;
  check();
  update(0, 4, -1, 7);
  update(0, 9, 1, -8);
  check();

  std::mt19937 random(0xAFF1u);
  for (int step = 0; step < 900; ++step) {
    auto first = static_cast<std::size_t>(random() % 10);
    auto last = static_cast<std::size_t>(random() % 10);
    if (first > last) {
      std::swap(first, last);
    }
    if (step % 5 == 0) {
      const auto index = static_cast<std::size_t>(random() % values.size());
      const number value = static_cast<number>(random() % 51) - 25;
      tree.point_update(index, affine_value(value));
      values[index] = value;
    } else {
      update(first, last, static_cast<number>(random() % 3) - 1,
             static_cast<number>(random() % 21) - 10);
    }
    check();
  }
}

struct counters {
  std::size_t applies = 0;
  std::size_t compositions = 0;
};

struct counted_apply {
  std::shared_ptr<counters> counts;
  explicit counted_apply(std::shared_ptr<counters> value)
      : counts(std::move(value)) {}

  number operator()(number aggregate, number delta, std::size_t length) const {
    ++counts->applies;
    return aggregate + delta * static_cast<number>(length);
  }
};

struct counted_compose {
  std::shared_ptr<counters> counts;
  explicit counted_compose(std::shared_ptr<counters> value)
      : counts(std::move(value)) {}

  number operator()(number old, number next) const {
    ++counts->compositions;
    return old + next;
  }
};

void test_real_laziness_and_policy_values() {
  const auto counts = std::make_shared<counters>();
  stl::segment_tree<number, std::plus<number>, counted_apply, counted_compose>
      tree(std::vector<number>(4096, 1), 0, std::plus<number>{},
           counted_apply(counts), counted_compose(counts));
  tree.range_update(0, tree.size(), 0);
  CHECK(counts->applies == 1);
  CHECK(counts->compositions == 0);
  tree.range_update(0, tree.size(), 3);
  CHECK(counts->applies == 2);
  CHECK(counts->compositions == 1);
  CHECK(std::as_const(tree).query(0, tree.size()) == 4096 * 4);
  CHECK(counts->applies == 2);
  const auto before_query = counts->applies;
  CHECK(std::as_const(tree).query(17, 18) == 4);
  const auto query_cost = counts->applies - before_query;
  CHECK(query_cost <= 12);
  const auto before_repeat = counts->applies;
  CHECK(std::as_const(tree).query(17, 18) == 4);
  CHECK(counts->applies - before_repeat == query_cost);
  const auto before_point = counts->applies;
  tree.point_update(17, -10);
  CHECK(counts->applies - before_point <= 24);
  CHECK(tree.query(0, tree.size()) == 4096 * 4 - 14);
}

void test_copies_and_moves() {
  std::vector<number> expected{1, 2, 3, 4, 5};
  sum_tree original(expected, 0);
  original.range_update(0, 5, 7);
  add(expected, 0, 5, 7);
  sum_tree copy(original);
  sum_tree assigned(std::vector<number>{99}, 0);
  assigned = original;
  original.point_update(2, -99);
  check_all_sums(copy, expected);
  check_all_sums(assigned, expected);
  copy.range_update(1, 4, -3);
  auto copy_expected = expected;
  add(copy_expected, 1, 4, -3);
  check_all_sums(copy, copy_expected);
  check_all_sums(assigned, expected);
  expected[2] = -99;
  check_all_sums(original, expected);

  sum_tree moved(std::move(copy));
  CHECK(copy.empty());
  check_all_sums(moved, copy_expected);
  sum_tree move_assigned(std::vector<number>{}, 0);
  move_assigned = std::move(assigned);
  CHECK(assigned.empty());
  check_all_sums(move_assigned, std::vector<number>{8, 9, 10, 11, 12});
  copy = moved;
  check_all_sums(copy, copy_expected);
  assigned = sum_tree(std::vector<number>{-8}, 0);
  CHECK(assigned.query(0, 1) == -8);
  sum_tree pending(std::vector<number>{1, 2, 3, 4}, 0);
  pending.range_update(0, 4, -10);
  sum_tree moved_pending(std::move(pending));
  check_all_sums(moved_pending, std::vector<number>{-9, -8, -7, -6});
}

}

int main() {
  try {
    test_empty_and_singleton();
    test_boundaries_and_pending_assignment();
    test_iterator_construction();
    test_random_sums();
    test_custom_minimum();
    test_chronological_composition();
    test_real_laziness_and_policy_values();
    test_copies_and_moves();
    std::cout << "All segment_tree tests passed\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << error.what() << '\n';
    return 1;
  } catch (...) {
    std::cerr << "Unexpected non-standard exception\n";
    return 1;
  }
}
