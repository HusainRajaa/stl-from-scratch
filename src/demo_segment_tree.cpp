#include <stl/segment_tree.hpp>

#include <iostream>
#include <vector>

int main() {
  const std::vector<long long> values{1, 2, 3, 4};
  stl::segment_tree<long long> sums(values, 0LL);

  std::cout << "\nSegment tree: 1 2 3 4\n";
  std::cout << "Sum of [0, 4): " << sums.query(0, 4) << '\n';

  sums.range_update(1, 4, 5);
  std::cout << "Add 5 to [1, 4): 1 7 8 9\n";
  std::cout << "Updated sum: " << sums.query(0, 4) << '\n';

  sums.point_update(2, 10);
  std::cout << "Assign index 2 to 10: 1 7 10 9\n";
  const auto total = sums.query(0, 4);
  const auto middle = sums.query(1, 3);
  std::cout << "Sum of [0, 4): " << total << '\n';
  std::cout << "Sum of [1, 3): " << middle << '\n';
  return total == 27 && middle == 17 ? 0 : 1;
}
