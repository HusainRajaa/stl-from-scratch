#include <stl/rb_tree.hpp>
#include <stl/splay_tree.hpp>

#include <iostream>

int main() {
  stl::red_black_tree<int> balanced;
  stl::splay_tree<int> adaptive;

  for (int value : {30, 10, 50, 20, 40}) {
    balanced.insert(value);
    adaptive.insert(value);
  }

  std::cout << "\nRed-black tree, sorted keys:";
  for (int value : balanced)
    std::cout << ' ' << value;

  auto [position, inserted] = balanced.insert(20);
  std::cout << "\nInsert duplicate " << *position << ": "
            << (inserted ? "inserted" : "already exists") << '\n';

  balanced.erase(30);
  std::cout << "After erasing 30:";
  for (int value : balanced)
    std::cout << ' ' << value;

  auto found = adaptive.find(20);
  std::cout << "\nSplay lookup for 20: "
            << (found != adaptive.end() ? "found and splayed" : "not found");
  std::cout << "\nSplay tree, reverse order:";
  for (auto it = adaptive.rbegin(); it != adaptive.rend(); ++it)
    std::cout << ' ' << *it;

  const bool valid = balanced.validate() && adaptive.validate();
  std::cout << "\nTree invariants: " << (valid ? "valid" : "invalid") << '\n';
  return valid ? 0 : 1;
}
