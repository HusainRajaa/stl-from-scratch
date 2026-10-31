#include <stl/binomial_heap.hpp>
#include <stl/fibonacci_heap.hpp>

#include <iostream>

int main() {
  stl::binomial_heap<int> binomial;
  for (int value : {10, 30, 20})
    binomial.push(value);

  stl::binomial_heap<int> other;
  other.push(40);
  binomial.merge(other);

  std::cout << "\nBinomial max-heap after merging 40:";
  while (!binomial.empty()) {
    std::cout << ' ' << binomial.top();
    binomial.pop();
  }

  stl::fibonacci_heap<int> fibonacci;
  auto handle = fibonacci.push(10);
  fibonacci.push(30);
  fibonacci.push(20);
  fibonacci.update_key(handle, 40);

  std::cout << "\nFibonacci max-heap after changing 10 to 40:";
  while (!fibonacci.empty()) {
    std::cout << ' ' << fibonacci.top();
    fibonacci.pop();
  }
  std::cout << '\n';
  return other.empty() ? 0 : 1;
}
