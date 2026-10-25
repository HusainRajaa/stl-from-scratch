#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace stl {

using input_iterator_tag = std::input_iterator_tag;
using output_iterator_tag = std::output_iterator_tag;
using forward_iterator_tag = std::forward_iterator_tag;
using bidirectional_iterator_tag = std::bidirectional_iterator_tag;
using random_access_iterator_tag = std::random_access_iterator_tag;
using contiguous_iterator_tag = std::contiguous_iterator_tag;

template <class Iterator>
struct iterator_traits : std::iterator_traits<Iterator> {};

template <class T>
  requires std::is_object_v<T>
struct iterator_traits<T *> {
  using iterator_category = random_access_iterator_tag;
  using iterator_concept = contiguous_iterator_tag;
  using value_type = std::remove_cv_t<T>;
  using difference_type = std::ptrdiff_t;
  using pointer = T *;
  using reference = T &;
};

struct tree_node_base {
  tree_node_base *parent = nullptr;
  tree_node_base *left = nullptr;
  tree_node_base *right = nullptr;
};

}
