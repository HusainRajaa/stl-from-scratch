#include <stl/stl_memory.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

#define CHECK(...)                                                             \
  do {                                                                         \
    if (!(__VA_ARGS__))                                                        \
      throw std::runtime_error(std::string("Check failed: ") + #__VA_ARGS__);  \
  } while (false)

struct allocation_state {
  std::size_t allocated = 0;
  std::size_t deallocated = 0;
  bool fail = false;
};

template <class T> struct tracked_allocator {
  using value_type = T;
  allocation_state *state;

  explicit tracked_allocator(allocation_state &stats) : state(&stats) {}

  template <class U>
  tracked_allocator(const tracked_allocator<U> &other) : state(other.state) {}

  T *allocate(std::size_t count) {
    if (state->fail)
      throw std::bad_alloc{};
    T *result = std::allocator<T>{}.allocate(count);
    state->allocated += count;
    return result;
  }

  void deallocate(T *pointer, std::size_t count) noexcept {
    state->deallocated += count;
    std::allocator<T>{}.deallocate(pointer, count);
  }

  template <class U>
  bool operator==(const tracked_allocator<U> &other) const noexcept {
    return state == other.state;
  }
};

struct alignas(128) tracked_value {
  inline static int live = 0;
  std::unique_ptr<int> value;

  tracked_value(std::unique_ptr<int> input, bool fail)
      : value(std::move(input)) {
    if (fail)
      throw std::runtime_error("Construction failed");
    ++live;
  }

  tracked_value(const tracked_value &) = delete;
  tracked_value &operator=(const tracked_value &) = delete;
  ~tracked_value() { --live; }
};

void test_lifetime_and_forwarding() {
  allocation_state state;
  tracked_allocator<tracked_value> allocator(state);
  auto input = std::make_unique<int>(42);
  auto *object = stl::allocate_object(allocator, std::move(input), false);
  CHECK(!input);
  CHECK(*object->value == 42);
  CHECK(tracked_value::live == 1);
  CHECK(state.allocated == 1);
  CHECK(state.deallocated == 0);
  CHECK(reinterpret_cast<std::uintptr_t>(object) % alignof(tracked_value) == 0);
  auto equivalent_allocator = allocator;
  stl::deallocate_object(equivalent_allocator, object);
  CHECK(tracked_value::live == 0);
  CHECK(state.deallocated == 1);
  stl::deallocate_object(allocator, nullptr);
  CHECK(state.deallocated == 1);
}

void test_construction_failure() {
  allocation_state state;
  tracked_allocator<tracked_value> allocator(state);
  for (int attempt = 0; attempt < 100; ++attempt) {
    bool threw = false;
    try {
      auto *object =
          stl::allocate_object(allocator, std::make_unique<int>(7), true);
      stl::deallocate_object(allocator, object);
    } catch (const std::runtime_error &) {
      threw = true;
    }
    CHECK(threw);
    CHECK(tracked_value::live == 0);
    CHECK(state.allocated == static_cast<std::size_t>(attempt + 1));
    CHECK(state.allocated == state.deallocated);
  }
  auto *object =
      stl::allocate_object(allocator, std::make_unique<int>(9), false);
  CHECK(*object->value == 9);
  stl::deallocate_object(allocator, object);
  CHECK(state.allocated == state.deallocated);
}

void test_allocation_failure() {
  allocation_state state;
  state.fail = true;
  tracked_allocator<tracked_value> allocator(state);
  bool threw = false;
  try {
    auto *object =
        stl::allocate_object(allocator, std::make_unique<int>(1), false);
    stl::deallocate_object(allocator, object);
  } catch (const std::bad_alloc &) {
    threw = true;
  }
  CHECK(threw);
  CHECK(state.allocated == 0);
  CHECK(state.deallocated == 0);
  CHECK(tracked_value::live == 0);
}

void test_standard_allocator() {
  std::allocator<int> allocator;
  auto *zero = stl::allocate_object(allocator);
  CHECK(*zero == 0);
  stl::deallocate_object(allocator, zero);
  auto *value = stl::allocate_object(allocator, 25);
  CHECK(*value == 25);
  stl::deallocate_object(allocator, value);
}

}

int main() {
  try {
    test_lifetime_and_forwarding();
    test_construction_failure();
    test_allocation_failure();
    test_standard_allocator();
    std::cout << "All memory tests passed\n";
    return 0;
  } catch (const std::exception &error) {
    std::cerr << "Memory tests failed: " << error.what() << '\n';
    return 1;
  }
}
