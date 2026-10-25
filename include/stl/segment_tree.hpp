#ifndef STL_SEGMENT_TREE_HPP
#define STL_SEGMENT_TREE_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace stl {

template <class T> struct segment_tree_sum_apply {
  T operator()(const T &aggregate, const T &delta, std::size_t length) const {
    return aggregate + delta * static_cast<T>(length);
  }
};

template <class T, class MergeOp = std::plus<T>,
          class ApplyOp = segment_tree_sum_apply<T>,
          class ComposeOp = std::plus<T>>
class segment_tree {
public:
  segment_tree(const std::vector<T> &values, T identity,
               MergeOp merge = MergeOp{}, ApplyOp apply = ApplyOp{},
               ComposeOp compose = ComposeOp{})
      : identity_(std::move(identity)), merge_(std::move(merge)),
        apply_(std::move(apply)), compose_(std::move(compose)) {
    initialize(values);
  }

  template <std::input_iterator InputIt, std::sentinel_for<InputIt> Sentinel>
  segment_tree(InputIt first, Sentinel last, T identity,
               MergeOp merge = MergeOp{}, ApplyOp apply = ApplyOp{},
               ComposeOp compose = ComposeOp{})
      : identity_(std::move(identity)), merge_(std::move(merge)),
        apply_(std::move(apply)), compose_(std::move(compose)) {
    std::vector<T> values;
    for (; first != last; ++first) {
      values.emplace_back(*first);
    }
    initialize(values);
  }

  segment_tree(const segment_tree &) = default;
  segment_tree &operator=(const segment_tree &) = default;

  segment_tree(segment_tree &&other) noexcept(
      std::is_nothrow_move_constructible_v<T> &&
      std::is_nothrow_move_constructible_v<MergeOp> &&
      std::is_nothrow_move_constructible_v<ApplyOp> &&
      std::is_nothrow_move_constructible_v<ComposeOp>)
      : identity_(std::move(other.identity_)), merge_(std::move(other.merge_)),
        apply_(std::move(other.apply_)), compose_(std::move(other.compose_)),
        tree_(std::move(other.tree_)), lazy_(std::move(other.lazy_)),
        size_(std::exchange(other.size_, 0)) {}

  segment_tree &operator=(segment_tree &&other) noexcept(
      std::is_nothrow_move_assignable_v<T> &&
      std::is_nothrow_move_assignable_v<MergeOp> &&
      std::is_nothrow_move_assignable_v<ApplyOp> &&
      std::is_nothrow_move_assignable_v<ComposeOp>) {
    if (this != &other) {
      identity_ = std::move(other.identity_);
      merge_ = std::move(other.merge_);
      apply_ = std::move(other.apply_);
      compose_ = std::move(other.compose_);
      tree_ = std::move(other.tree_);
      lazy_ = std::move(other.lazy_);
      size_ = std::exchange(other.size_, 0);
    }
    return *this;
  }

  [[nodiscard]] std::size_t size() const noexcept { return size_; }
  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

  [[nodiscard]] T query(std::size_t first, std::size_t last) const {
    validate_range(first, last);
    if (first == last) {
      return identity_;
    }
    return query_impl(1, 0, size_, first, last);
  }

  void point_update(std::size_t index, const T &value) {
    if (index >= size_) {
      throw std::out_of_range("segment_tree: point index out of range");
    }
    point_update_impl(1, 0, size_, index, value);
  }

  void range_update(std::size_t first, std::size_t last, const T &delta) {
    validate_range(first, last);
    if (first != last) {
      range_update_impl(1, 0, size_, first, last, delta);
    }
  }

private:
  T identity_;
  MergeOp merge_;
  ApplyOp apply_;
  ComposeOp compose_;
  std::vector<T> tree_;
  std::vector<std::optional<T>> lazy_;
  std::size_t size_ = 0;

  void initialize(const std::vector<T> &values) {
    size_ = values.size();
    if (empty()) {
      return;
    }
    if (size_ > tree_.max_size() / 4 || size_ > lazy_.max_size() / 4) {
      throw std::length_error("segment_tree: too many elements");
    }
    tree_.assign(4 * size_, identity_);
    lazy_.resize(4 * size_);
    build(1, 0, size_, values);
  }

  void validate_range(std::size_t first, std::size_t last) const {
    if (first > last || last > size_) {
      throw std::out_of_range("segment_tree: invalid half-open range");
    }
  }

  void build(std::size_t node, std::size_t left, std::size_t right,
             const std::vector<T> &values) {
    if (right - left == 1) {
      tree_[node] = values[left];
      return;
    }
    const auto mid = left + (right - left) / 2;
    build(2 * node, left, mid, values);
    build(2 * node + 1, mid, right, values);
    pull(node);
  }

  void pull(std::size_t node) {
    tree_[node] = merge_(tree_[2 * node], tree_[2 * node + 1]);
  }

  void apply_node(std::size_t node, std::size_t length, const T &delta) {
    tree_[node] = apply_(tree_[node], delta, length);
    if (length > 1) {
      if (lazy_[node]) {
        lazy_[node] = compose_(*lazy_[node], delta);
      } else {
        lazy_[node] = delta;
      }
    }
  }

  void push(std::size_t node, std::size_t left, std::size_t right) {
    if (lazy_[node]) {
      const auto mid = left + (right - left) / 2;
      apply_node(2 * node, mid - left, *lazy_[node]);
      apply_node(2 * node + 1, right - mid, *lazy_[node]);
      lazy_[node].reset();
    }
  }

  T query_impl(std::size_t node, std::size_t left, std::size_t right,
               std::size_t first, std::size_t last) const {
    if (first <= left && right <= last) {
      return tree_[node];
    }
    const auto mid = left + (right - left) / 2;
    T result = [&]() -> T {
      if (last <= mid) {
        return query_impl(2 * node, left, mid, first, last);
      }
      if (first >= mid) {
        return query_impl(2 * node + 1, mid, right, first, last);
      }
      return merge_(query_impl(2 * node, left, mid, first, last),
                    query_impl(2 * node + 1, mid, right, first, last));
    }();
    // The children haven't seen this node's pending update yet.
    if (lazy_[node]) {
      result = apply_(result, *lazy_[node],
                      std::min(right, last) - std::max(left, first));
    }
    return result;
  }

  void point_update_impl(std::size_t node, std::size_t left, std::size_t right,
                         std::size_t index, const T &value) {
    if (right - left == 1) {
      tree_[node] = value;
      return;
    }
    push(node, left, right);
    const auto mid = left + (right - left) / 2;
    if (index < mid) {
      point_update_impl(2 * node, left, mid, index, value);
    } else {
      point_update_impl(2 * node + 1, mid, right, index, value);
    }
    pull(node);
  }

  void range_update_impl(std::size_t node, std::size_t left, std::size_t right,
                         std::size_t first, std::size_t last, const T &delta) {
    if (first <= left && right <= last) {
      apply_node(node, right - left, delta);
      return;
    }
    push(node, left, right);
    const auto mid = left + (right - left) / 2;
    if (first < mid) {
      range_update_impl(2 * node, left, mid, first, last, delta);
    }
    if (last > mid) {
      range_update_impl(2 * node + 1, mid, right, first, last, delta);
    }
    pull(node);
  }
};

}

#endif
