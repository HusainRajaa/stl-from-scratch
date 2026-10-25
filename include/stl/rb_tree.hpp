#pragma once

#include "stl_iterator.hpp"
#include "stl_memory.hpp"

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

namespace stl {

template <class T, class Compare = std::less<T>> class red_black_tree {
  enum class color { red, black };

  struct node_base {
    node_base *parent = nullptr;
    node_base *left = nullptr;
    node_base *right = nullptr;
    color shade = color::black;
    bool is_header = false;
  };

  struct node : node_base {
    template <class U>
    explicit node(U &&value) : value(std::forward<U>(value)) {}

    T value;
  };

  using node_allocator = std::allocator<node>;

  static node_base *minimum(node_base *current) noexcept {
    while (current->left) {
      current = current->left;
    }
    return current;
  }

  static node_base *maximum(node_base *current) noexcept {
    while (current->right) {
      current = current->right;
    }
    return current;
  }

  static const T &value_of(const node_base *current) noexcept {
    return static_cast<const node *>(current)->value;
  }

  static color color_of(const node_base *current) noexcept {
    return current ? current->shade : color::black;
  }

public:
  using key_type = T;
  using value_type = T;
  using key_compare = Compare;
  using value_compare = Compare;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = const T &;
  using const_reference = const T &;
  using pointer = const T *;
  using const_pointer = const T *;

  class iterator {
    friend class red_black_tree;

    const node_base *current_ = nullptr;

    explicit iterator(const node_base *current) noexcept : current_(current) {}

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using iterator_concept = std::bidirectional_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = const T *;
    using reference = const T &;

    iterator() noexcept = default;

    reference operator*() const noexcept { return value_of(current_); }
    pointer operator->() const noexcept { return std::addressof(operator*()); }

    iterator &operator++() noexcept {
      if (current_->right) {
        current_ = minimum(current_->right);
      } else {
        const node_base *parent = current_->parent;
        while (!parent->is_header && current_ == parent->right) {
          current_ = parent;
          parent = parent->parent;
        }
        current_ = parent;
      }
      return *this;
    }

    iterator operator++(int) noexcept {
      iterator previous = *this;
      ++*this;
      return previous;
    }

    iterator &operator--() noexcept {
      if (current_->is_header) {
        current_ = current_->right;
      } else if (current_->left) {
        current_ = maximum(current_->left);
      } else {
        const node_base *parent = current_->parent;
        while (!parent->is_header && current_ == parent->left) {
          current_ = parent;
          parent = parent->parent;
        }
        current_ = parent;
      }
      return *this;
    }

    iterator operator--(int) noexcept {
      iterator previous = *this;
      --*this;
      return previous;
    }

    friend bool operator==(const iterator &,
                           const iterator &) noexcept = default;
  };

  using const_iterator = iterator;

  red_black_tree() noexcept(std::is_nothrow_default_constructible_v<Compare>)
      : compare_() {
    reset_header();
  }

  explicit red_black_tree(const Compare &compare) noexcept(
      std::is_nothrow_copy_constructible_v<Compare>)
      : compare_(compare) {
    reset_header();
  }

  red_black_tree(const red_black_tree &other) : compare_(other.compare_) {
    reset_header();
    header_.parent = clone_subtree(other.header_.parent, &header_);
    size_ = other.size_;
    refresh_extrema();
  }

  red_black_tree(red_black_tree &&other) noexcept(
      std::is_nothrow_move_constructible_v<Compare>)
      : compare_(std::move(other.compare_)) {
    reset_header();
    take_nodes(other);
  }

  red_black_tree &operator=(const red_black_tree &other) {
    if (this != &other) {
      red_black_tree copy(other);
      swap(copy);
    }
    return *this;
  }

  red_black_tree &operator=(red_black_tree &&other) noexcept(
      std::is_nothrow_move_assignable_v<Compare>) {
    if (this != &other) {

      clear();
      compare_ = std::move(other.compare_);
      take_nodes(other);
    }
    return *this;
  }

  ~red_black_tree() { clear(); }

  [[nodiscard]] iterator begin() noexcept { return iterator(header_.left); }
  [[nodiscard]] const_iterator begin() const noexcept { return cbegin(); }
  [[nodiscard]] const_iterator cbegin() const noexcept {
    return const_iterator(header_.left);
  }
  [[nodiscard]] iterator end() noexcept { return iterator(&header_); }
  [[nodiscard]] const_iterator end() const noexcept { return cend(); }
  [[nodiscard]] const_iterator cend() const noexcept {
    return const_iterator(&header_);
  }

  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
  [[nodiscard]] size_type size() const noexcept { return size_; }

  std::pair<iterator, bool> insert(const T &value) {
    return insert_value(value);
  }
  std::pair<iterator, bool> insert(T &&value) {
    return insert_value(std::move(value));
  }

  [[nodiscard]] iterator find(const T &key) { return iterator(find_node(key)); }
  [[nodiscard]] const_iterator find(const T &key) const {
    return const_iterator(find_node(key));
  }
  [[nodiscard]] bool contains(const T &key) const {
    return find_node(key) != &header_;
  }

  size_type erase(const T &key) {
    iterator position = find(key);
    if (position == end()) {
      return 0;
    }
    erase(position);
    return 1;
  }

  iterator erase(iterator position) noexcept {
    iterator next = position;
    ++next;
    node_base *removed = const_cast<node_base *>(position.current_);
    node_base *replacement = removed;
    color removed_color = replacement->shade;
    node_base *child = nullptr;
    node_base *child_parent = nullptr;

    if (!removed->left) {
      child = removed->right;
      child_parent = removed->parent;
      transplant(removed, child);
    } else if (!removed->right) {
      child = removed->left;
      child_parent = removed->parent;
      transplant(removed, child);
    } else {
      replacement = minimum(removed->right);
      removed_color = replacement->shade;
      child = replacement->right;
      if (replacement->parent == removed) {
        child_parent = replacement;
      } else {
        child_parent = replacement->parent;
        transplant(replacement, child);
        replacement->right = removed->right;
        replacement->right->parent = replacement;
      }
      transplant(removed, replacement);
      replacement->left = removed->left;
      replacement->left->parent = replacement;
      replacement->shade = removed->shade;
    }

    if (removed_color == color::black) {
      fix_erase(child, child_parent);
    }
    destroy_node(removed);
    --size_;
    refresh_extrema();
    return next;
  }

  void clear() noexcept {
    destroy_subtree(header_.parent);
    size_ = 0;
    reset_header();
  }

  void
  swap(red_black_tree &other) noexcept(std::is_nothrow_swappable_v<Compare>) {
    if (this == &other) {
      return;
    }
    using std::swap;
    swap(compare_, other.compare_);
    swap(header_.parent, other.header_.parent);
    swap(header_.left, other.header_.left);
    swap(header_.right, other.header_.right);
    swap(size_, other.size_);
    repair_header();
    other.repair_header();
  }

  friend void swap(red_black_tree &left,
                   red_black_tree &right) noexcept(noexcept(left.swap(right))) {
    left.swap(right);
  }

  [[nodiscard]] bool validate() const {
    if (!header_.is_header || header_.shade != color::black) {
      return false;
    }
    if (!header_.parent) {
      return size_ == 0 && header_.left == &header_ &&
             header_.right == &header_;
    }
    if (size_ == 0 || header_.parent->parent != &header_ ||
        header_.parent->shade != color::black) {
      return false;
    }
    size_type count = 0;
    if (validate_subtree(header_.parent, &header_, nullptr, nullptr, count) ==
            0 ||
        count != size_) {
      return false;
    }
    return header_.left == minimum(header_.parent) &&
           header_.right == maximum(header_.parent);
  }

private:
  Compare compare_;
  node_base header_;
  size_type size_ = 0;

  void reset_header() noexcept {
    header_.parent = nullptr;
    header_.left = &header_;
    header_.right = &header_;
    header_.shade = color::black;
    header_.is_header = true;
  }

  void refresh_extrema() noexcept {
    if (header_.parent) {
      header_.left = minimum(header_.parent);
      header_.right = maximum(header_.parent);
    } else {
      header_.left = &header_;
      header_.right = &header_;
    }
  }

  void repair_header() noexcept {
    if (header_.parent) {
      header_.parent->parent = &header_;
    } else {
      header_.left = &header_;
      header_.right = &header_;
    }
  }

  void take_nodes(red_black_tree &other) noexcept {
    header_.parent = other.header_.parent;
    header_.left = other.header_.left;
    header_.right = other.header_.right;
    size_ = other.size_;
    repair_header();
    other.size_ = 0;
    other.reset_header();
  }

  template <class U> static node *create_node(U &&value) {
    node_allocator allocator;
    return stl::allocate_object(allocator, std::forward<U>(value));
  }

  static void destroy_node(node_base *current) noexcept {
    node_allocator allocator;
    stl::deallocate_object(allocator, static_cast<node *>(current));
  }

  static void destroy_subtree(node_base *current) noexcept {
    if (current) {
      destroy_subtree(current->left);
      destroy_subtree(current->right);
      destroy_node(current);
    }
  }

  static node_base *clone_subtree(const node_base *source, node_base *parent) {
    if (!source) {
      return nullptr;
    }
    node *copy = create_node(value_of(source));
    copy->parent = parent;
    copy->shade = source->shade;
    try {
      copy->left = clone_subtree(source->left, copy);
      copy->right = clone_subtree(source->right, copy);
    } catch (...) {
      destroy_subtree(copy);
      throw;
    }
    return copy;
  }

  const node_base *find_node(const T &key) const {
    const node_base *current = header_.parent;
    while (current) {
      if (compare_(key, value_of(current))) {
        current = current->left;
      } else if (compare_(value_of(current), key)) {
        current = current->right;
      } else {
        return current;
      }
    }
    return &header_;
  }

  template <class U> std::pair<iterator, bool> insert_value(U &&value) {
    node_base *parent = &header_;
    node_base *current = header_.parent;
    bool insert_left = true;
    while (current) {
      parent = current;
      if (compare_(value, value_of(current))) {
        insert_left = true;
        current = current->left;
      } else if (compare_(value_of(current), value)) {
        insert_left = false;
        current = current->right;
      } else {
        return {iterator(current), false};
      }
    }

    node *inserted = create_node(std::forward<U>(value));
    inserted->parent = parent;
    inserted->shade = color::red;
    if (parent == &header_) {
      header_.parent = inserted;
      header_.left = inserted;
      header_.right = inserted;
    } else if (insert_left) {
      parent->left = inserted;
      if (parent == header_.left) {
        header_.left = inserted;
      }
    } else {
      parent->right = inserted;
      if (parent == header_.right) {
        header_.right = inserted;
      }
    }
    fix_insert(inserted);
    ++size_;
    return {iterator(inserted), true};
  }

  void rotate_left(node_base *pivot) noexcept {
    node_base *promoted = pivot->right;
    pivot->right = promoted->left;
    if (promoted->left) {
      promoted->left->parent = pivot;
    }
    promoted->parent = pivot->parent;
    if (pivot->parent == &header_) {
      header_.parent = promoted;
    } else if (pivot == pivot->parent->left) {
      pivot->parent->left = promoted;
    } else {
      pivot->parent->right = promoted;
    }
    promoted->left = pivot;
    pivot->parent = promoted;
  }

  void rotate_right(node_base *pivot) noexcept {
    node_base *promoted = pivot->left;
    pivot->left = promoted->right;
    if (promoted->right) {
      promoted->right->parent = pivot;
    }
    promoted->parent = pivot->parent;
    if (pivot->parent == &header_) {
      header_.parent = promoted;
    } else if (pivot == pivot->parent->right) {
      pivot->parent->right = promoted;
    } else {
      pivot->parent->left = promoted;
    }
    promoted->right = pivot;
    pivot->parent = promoted;
  }

  void fix_insert(node_base *current) noexcept {
    while (current->parent->shade == color::red) {
      node_base *parent = current->parent;
      node_base *grandparent = parent->parent;
      if (parent == grandparent->left) {
        node_base *uncle = grandparent->right;
        if (color_of(uncle) == color::red) {
          parent->shade = color::black;
          uncle->shade = color::black;
          grandparent->shade = color::red;
          current = grandparent;
        } else {
          if (current == parent->right) {
            current = parent;
            rotate_left(current);
            parent = current->parent;
            grandparent = parent->parent;
          }
          parent->shade = color::black;
          grandparent->shade = color::red;
          rotate_right(grandparent);
        }
      } else {
        node_base *uncle = grandparent->left;
        if (color_of(uncle) == color::red) {
          parent->shade = color::black;
          uncle->shade = color::black;
          grandparent->shade = color::red;
          current = grandparent;
        } else {
          if (current == parent->left) {
            current = parent;
            rotate_right(current);
            parent = current->parent;
            grandparent = parent->parent;
          }
          parent->shade = color::black;
          grandparent->shade = color::red;
          rotate_left(grandparent);
        }
      }
    }
    header_.parent->shade = color::black;
  }

  void transplant(node_base *old_node, node_base *new_node) noexcept {
    if (old_node->parent == &header_) {
      header_.parent = new_node;
    } else if (old_node == old_node->parent->left) {
      old_node->parent->left = new_node;
    } else {
      old_node->parent->right = new_node;
    }
    if (new_node) {
      new_node->parent = old_node->parent;
    }
  }

  // Keep the parent separately when the replacement is a null leaf.
  void fix_erase(node_base *current, node_base *parent) noexcept {
    while (current != header_.parent && color_of(current) == color::black) {
      if (current == parent->left) {
        node_base *sibling = parent->right;
        if (color_of(sibling) == color::red) {
          sibling->shade = color::black;
          parent->shade = color::red;
          rotate_left(parent);
          sibling = parent->right;
        }
        if (!sibling) {
          current = parent;
          parent = current->parent;
          continue;
        }
        if (color_of(sibling->left) == color::black &&
            color_of(sibling->right) == color::black) {
          sibling->shade = color::red;
          current = parent;
          parent = current->parent;
        } else {
          if (color_of(sibling->right) == color::black) {
            sibling->left->shade = color::black;
            sibling->shade = color::red;
            rotate_right(sibling);
            sibling = parent->right;
          }
          sibling->shade = parent->shade;
          parent->shade = color::black;
          if (sibling->right) {
            sibling->right->shade = color::black;
          }
          rotate_left(parent);
          current = header_.parent;
          break;
        }
      } else {
        node_base *sibling = parent->left;
        if (color_of(sibling) == color::red) {
          sibling->shade = color::black;
          parent->shade = color::red;
          rotate_right(parent);
          sibling = parent->left;
        }
        if (!sibling) {
          current = parent;
          parent = current->parent;
          continue;
        }
        if (color_of(sibling->right) == color::black &&
            color_of(sibling->left) == color::black) {
          sibling->shade = color::red;
          current = parent;
          parent = current->parent;
        } else {
          if (color_of(sibling->left) == color::black) {
            sibling->right->shade = color::black;
            sibling->shade = color::red;
            rotate_left(sibling);
            sibling = parent->left;
          }
          sibling->shade = parent->shade;
          parent->shade = color::black;
          if (sibling->left) {
            sibling->left->shade = color::black;
          }
          rotate_right(parent);
          current = header_.parent;
          break;
        }
      }
    }
    if (current) {
      current->shade = color::black;
    }
  }

  size_type validate_subtree(const node_base *current, const node_base *parent,
                             const T *lower, const T *upper,
                             size_type &count) const {
    if (!current) {
      return 1;
    }
    if (current->is_header || current->parent != parent || ++count > size_) {
      return 0;
    }
    const T &value = value_of(current);
    if ((lower && !compare_(*lower, value)) ||
        (upper && !compare_(value, *upper))) {
      return 0;
    }
    if (current->shade != color::red && current->shade != color::black) {
      return 0;
    }
    if (current->shade == color::red &&
        (color_of(current->left) == color::red ||
         color_of(current->right) == color::red)) {
      return 0;
    }
    const size_type left_height = validate_subtree(
        current->left, current, lower, std::addressof(value), count);
    if (left_height == 0) {
      return 0;
    }
    const size_type right_height = validate_subtree(
        current->right, current, std::addressof(value), upper, count);
    if (right_height == 0 || left_height != right_height) {
      return 0;
    }
    return left_height + (current->shade == color::black ? 1 : 0);
  }
};

}
