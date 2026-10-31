#include <stl/compressed_trie.hpp>

#include <iostream>

int main() {
  stl::compressed_trie words;
  for (const char *word : {"apple", "application", "apply", "banana"})
    words.insert(word);

  std::cout << "\nCompressed trie\n" << std::boolalpha;
  std::cout << "Contains apple: " << words.contains("apple") << '\n';
  std::cout << "Contains app: " << words.contains("app") << '\n';
  std::cout << "Has prefix app: " << words.starts_with("app") << '\n';
  std::cout << "Completions for app:";
  for (const auto &word : words.autocomplete("app"))
    std::cout << ' ' << word;

  words.erase("apple");
  std::cout << "\nAfter erasing apple:";
  for (const auto &word : words.autocomplete("app"))
    std::cout << ' ' << word;
  std::cout << '\n';
  return !words.contains("apple") && words.contains("apply") ? 0 : 1;
}
