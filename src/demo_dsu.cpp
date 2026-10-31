#include <stl/dsu.hpp>

#include <iostream>

int main() {
  stl::disjoint_set components(6);
  components.unite(0, 1);
  components.unite(1, 2);
  components.unite(3, 4);

  std::cout << "\nDisjoint set union\n" << std::boolalpha;
  std::cout << "Connected 0 and 2: " << components.connected(0, 2) << '\n';
  std::cout << "Connected 0 and 4: " << components.connected(0, 4) << '\n';
  std::cout << "Component size for 1: " << components.set_size(1) << '\n';
  std::cout << "Components remaining: " << components.num_sets() << '\n';

  for (const auto &group : components.sets()) {
    std::cout << "Group:";
    for (auto element : group)
      std::cout << ' ' << element;
    std::cout << '\n';
  }

  const auto added = components.add_element();
  components.unite(5, added);
  std::cout << "Added element " << added << " and joined it with 5\n";
  std::cout << "Size of that component: " << components.set_size(5) << '\n';
  return components.num_sets() == 3 && components.set_size(5) == 2 ? 0 : 1;
}
