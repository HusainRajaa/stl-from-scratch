#pragma once

#include "stl_iterator.hpp"
#include "stl_memory.hpp"

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

namespace stl {

template <class T, class Compare = std::less<T>> class splay_tree {
  struct node {
    T value;
    node *parent = nullptr;
    node *left = nullptr;
    node *right = nullptr;

    template <class U> explicit node(U &&key) : value(std::forward<U>(key)) {}
  };

  template <class U> static node *create_node(U &&value) {
    std::allocator<node> allocator;
    return stl::allocate_object(allocator, std::forward<U>(value));
  }

  static void destroy_node(node *current) noexcept {
    std::allocator<node> allocator;
    stl::deallocate_object(allocator, current);
  }

  Compare compare_;
  node *root_ = nullptr;
  std::size_t size_ = 0;

  static node *minimum(node *p) noexcept {
    if (p)
      while (p->left)
        p = p->left;
    return p;
  }

  static node *maximum(node *p) noexcept {
    if (p)
      while (p->right)
        p = p->right;
    return p;
  }

  static node *successor(node *p) noexcept {
    if (p->right)
      return minimum(p->right);
    node *parent = p->parent;
    while (parent && p == parent->right) {
      p = parent;
      parent = parent->parent;
    }
    return parent;
  }

  static node *predecessor(node *p) noexcept {
    if (p->left)
      return maximum(p->left);
    node *parent = p->parent;
    while (parent && p == parent->left) {
      p = parent;
      parent = parent->parent;
    }
    return parent;
  }

  void rotate(node *p) noexcept {
    node *parent = p->parent;
    node *grandparent = parent->parent;
    if (p == parent->left) {
      parent->left = p->right;
      if (p->right)
        p->right->parent = parent;
      p->right = parent;
    } else {
      parent->right = p->left;
      if (p->left)
        p->left->parent = parent;
      p->left = parent;
    }
    parent->parent = p;
    p->parent = grandparent;
    if (!grandparent)
      root_ = p;
    else if (grandparent->left == parent)
      grandparent->left = p;
    else
      grandparent->right = p;
  }

  void splay(node *p) noexcept {
    while (p && p->parent) {
      node *parent = p->parent;
      node *grandparent = parent->parent;
      if (grandparent) {
        if ((p == parent->left) == (parent == grandparent->left))
          rotate(parent);
        else
          rotate(p);
      }
      rotate(p);
    }
  }

  template <class U> std::pair<node *, bool> insert_node(U &&key) {
    node *parent = nullptr;
    node *current = root_;
    bool left = false;
    while (current) {
      parent = current;
      if (compare_(key, current->value)) {
        left = true;
        current = current->left;
      } else if (compare_(current->value, key)) {
        left = false;
        current = current->right;
      } else {
        splay(current);
        return {current, false};
      }
    }

    node *inserted = create_node(std::forward<U>(key));
    inserted->parent = parent;
    if (!parent)
      root_ = inserted;
    else if (left)
      parent->left = inserted;
    else
      parent->right = inserted;
    ++size_;
    splay(inserted);
    return {inserted, true};
  }

  node *lookup(const T &key, node *&last) const {
    node *current = root_;
    last = nullptr;
    while (current) {
      last = current;
      if (compare_(key, current->value))
        current = current->left;
      else if (compare_(current->value, key))
        current = current->right;
      else
        return current;
    }
    return nullptr;
  }

  void clone(const splay_tree &other) {
    if (!other.root_)
      return;
    try {
      root_ = create_node(other.root_->value);
      size_ = 1;
      std::vector<std::pair<const node *, node *>> pending;
      pending.emplace_back(other.root_, root_);
      while (!pending.empty()) {
        auto [source, target] = pending.back();
        pending.pop_back();
        if (source->left) {
          target->left = create_node(source->left->value);
          target->left->parent = target;
          ++size_;
          pending.emplace_back(source->left, target->left);
        }
        if (source->right) {
          target->right = create_node(source->right->value);
          target->right->parent = target;
          ++size_;
          pending.emplace_back(source->right, target->right);
        }
      }
    } catch (...) {
      clear();
      throw;
    }
  }

public:
  using key_type = T;
  using value_type = T;
  using key_compare = Compare;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = const T &;
  using const_reference = const T &;

  class const_iterator {
    friend class splay_tree;
    const splay_tree *owner_ = nullptr;
    node *current_ = nullptr;

    const_iterator(const splay_tree *owner, node *current) noexcept
        : owner_(owner), current_(current) {}

  public:
    using iterator_category = stl::bidirectional_iterator_tag;
    using iterator_concept = stl::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T *;
    using reference = const T &;

    const_iterator() noexcept = default;
    reference operator*() const noexcept { return current_->value; }
    pointer operator->() const noexcept {
      return std::addressof(current_->value);
    }
    const_iterator &operator++() noexcept {
      current_ = successor(current_);
      return *this;
    }
    const_iterator operator++(int) noexcept {
      auto previous = *this;
      ++*this;
      return previous;
    }
    const_iterator &operator--() noexcept {
      current_ = current_ ? predecessor(current_) : maximum(owner_->root_);
      return *this;
    }
    const_iterator operator--(int) noexcept {
      auto previous = *this;
      --*this;
      return previous;
    }
    friend bool operator==(const const_iterator &a,
                           const const_iterator &b) noexcept {
      return a.current_ == b.current_;
    }
  };

  using iterator = const_iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  splay_tree() = default;
  explicit splay_tree(const Compare &compare) : compare_(compare) {}
  splay_tree(const splay_tree &other) : compare_(other.compare_) {
    clone(other);
  }
  splay_tree(splay_tree &&other) noexcept(
      std::is_nothrow_move_constructible_v<Compare>)
      : compare_(std::move(other.compare_)),
        root_(std::exchange(other.root_, nullptr)),
        size_(std::exchange(other.size_, 0)) {}

  splay_tree &operator=(const splay_tree &other) {
    if (this != &other) {
      splay_tree copy(other);
      swap(copy);
    }
    return *this;
  }

  splay_tree &operator=(splay_tree &&other) noexcept(
      std::is_nothrow_move_constructible_v<Compare> &&
      std::is_nothrow_swappable_v<Compare>) {
    if (this != &other) {
      splay_tree moved(std::move(other));
      swap(moved);
    }
    return *this;
  }

  ~splay_tree() { clear(); }

  void swap(splay_tree &other) noexcept(std::is_nothrow_swappable_v<Compare>) {
    using std::swap;
    swap(compare_, other.compare_);
    swap(root_, other.root_);
    swap(size_, other.size_);
  }

  friend void swap(splay_tree &a, splay_tree &b) noexcept(noexcept(a.swap(b))) {
    a.swap(b);
  }

  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
  [[nodiscard]] size_type size() const noexcept { return size_; }
  iterator begin() noexcept { return iterator(this, minimum(root_)); }
  const_iterator begin() const noexcept {
    return const_iterator(this, minimum(root_));
  }
  const_iterator cbegin() const noexcept { return begin(); }
  iterator end() noexcept { return iterator(this, nullptr); }
  const_iterator end() const noexcept { return const_iterator(this, nullptr); }
  const_iterator cend() const noexcept { return end(); }
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const noexcept { return rend(); }

  std::pair<iterator, bool> insert(const T &key) {
    auto [p, inserted] = insert_node(key);
    return {iterator(this, p), inserted};
  }
  std::pair<iterator, bool> insert(T &&key) {
    auto [p, inserted] = insert_node(std::move(key));
    return {iterator(this, p), inserted};
  }

  iterator find(const T &key) {
    node *last = nullptr;
    node *found = lookup(key, last);
    splay(last);
    return iterator(this, found);
  }
  const_iterator find(const T &key) const {
    node *last = nullptr;
    return const_iterator(this, lookup(key, last));
  }
  bool contains(const T &key) { return find(key) != end(); }
  bool contains(const T &key) const { return find(key) != end(); }

  size_type erase(const T &key) {
    auto position = find(key);
    if (position == end())
      return 0;
    erase(position);
    return 1;
  }

  iterator erase(iterator position) noexcept {
    node *removed = position.current_;
    if (!removed)
      return end();
    node *next = successor(removed);
    splay(removed);
    node *left = removed->left;
    node *right = removed->right;
    if (left)
      left->parent = nullptr;
    if (right)
      right->parent = nullptr;
    root_ = left;
    if (left) {
      splay(maximum(left));
      root_->right = right;
      if (right)
        right->parent = root_;
    } else {
      root_ = right;
    }
    destroy_node(removed);
    --size_;
    return iterator(this, next);
  }

  void clear() noexcept {

    node *current = root_;
    while (current) {
      if (current->left)
        current = current->left;
      else if (current->right)
        current = current->right;
      else {
        node *parent = current->parent;
        if (parent) {
          if (parent->left == current)
            parent->left = nullptr;
          else
            parent->right = nullptr;
        }
        destroy_node(current);
        current = parent;
      }
    }
    root_ = nullptr;
    size_ = 0;
  }

  [[nodiscard]] bool validate() const {
    if (!root_)
      return size_ == 0;
    if (root_->parent || size_ == 0)
      return false;
    struct frame {
      const node *current;
      const node *parent;
      const T *lower;
      const T *upper;
    };
    std::vector<frame> pending{{root_, nullptr, nullptr, nullptr}};
    size_type count = 0;
    while (!pending.empty()) {
      auto [current, parent, lower, upper] = pending.back();
      pending.pop_back();
      if (++count > size_ || current->parent != parent)
        return false;
      if (lower && !compare_(*lower, current->value))
        return false;
      if (upper && !compare_(current->value, *upper))
        return false;
      if (current->left)
        pending.push_back({current->left, current, lower, &current->value});
      if (current->right)
        pending.push_back({current->right, current, &current->value, upper});
    }
    return count == size_;
  }
};

}
