#include <stl/rb_tree.hpp>
#include <stl/splay_tree.hpp>
#include <stl/stl_iterator.hpp>

#include <concepts>
#include <iostream>
#include <type_traits>

template <class Iterator>
concept has_iterator_value =
    requires { typename stl::iterator_traits<Iterator>::value_type; };

static_assert(!has_iterator_value<void *>);
static_assert(!has_iterator_value<const void *>);
static_assert(!has_iterator_value<void (*)()>);
static_assert(!has_iterator_value<int>);
static_assert(std::same_as<stl::iterator_traits<const int *>::value_type, int>);
static_assert(
    std::same_as<stl::iterator_traits<const int *>::reference, const int &>);
static_assert(std::same_as<stl::iterator_traits<volatile int *>::pointer,
                           volatile int *>);
static_assert(std::same_as<stl::iterator_traits<int *>::iterator_concept,
                           std::contiguous_iterator_tag>);
static_assert(std::derived_from<stl::bidirectional_iterator_tag,
                                stl::forward_iterator_tag>);

template <class Tree> constexpr bool tree_traits() {
  using iterator = typename Tree::iterator;
  using traits = stl::iterator_traits<iterator>;
  static_assert(std::bidirectional_iterator<iterator>);
  static_assert(std::same_as<typename traits::iterator_category,
                             stl::bidirectional_iterator_tag>);
  static_assert(std::same_as<typename traits::value_type, int>);
  static_assert(std::same_as<typename traits::reference, const int &>);
  return true;
}

static_assert(tree_traits<stl::red_black_tree<int>>());
static_assert(tree_traits<stl::splay_tree<int>>());

int main() {
  stl::tree_node_base node;
  if (node.parent || node.left || node.right)
    return 1;
  std::cout << "Iterator infrastructure tests passed\n";
}
