#pragma once

#include <memory>
#include <utility>

namespace stl {

template <class Allocator, class... Args>
[[nodiscard]] auto allocate_object(Allocator &allocator, Args &&...args) ->
    typename std::allocator_traits<Allocator>::pointer {
  using traits = std::allocator_traits<Allocator>;
  auto object = traits::allocate(allocator, 1);
  try {
    traits::construct(allocator, std::to_address(object),
                      std::forward<Args>(args)...);
  } catch (...) {
    traits::deallocate(allocator, object, 1);
    throw;
  }
  return object;
}

template <class Allocator>
void deallocate_object(
    Allocator &allocator,
    typename std::allocator_traits<Allocator>::pointer object) noexcept {
  using traits = std::allocator_traits<Allocator>;
  if (object == typename traits::pointer{})
    return;
  traits::destroy(allocator, std::to_address(object));
  traits::deallocate(allocator, object, 1);
}

}
