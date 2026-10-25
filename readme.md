# STL from Scratch: Advanced Heaps, Trie, Graph DSU & Benchmarks

Advanced Data Structures coursework: custom header-only C++20 standard template library containers implemented from scratch. This branch (`dev_husain`) implements Husain's component portfolio: Advanced Heaps, String Containers, Graph Disjoint Sets, C++20 Test Harness, and Performance Benchmarks.

---

## Portfolio Division (Husain's Implementation)

| Container / Module | Header File | Key Data Structure & Algorithm | Syllabus Target |
| :--- | :--- | :--- | :--- |
| **Binomial Heap** | `include/stl/binomial_heap.hpp` | Collection of binomial trees ($B_k$), binary addition union | Advanced Priority Queues |
| **Fibonacci Heap** | `include/stl/fibonacci_heap.hpp` | Circular doubly-linked root lists, lazy consolidation, cascading cuts | Amortized Optimal Priority Queue |
| **Compressed Trie** | `include/stl/compressed_trie.hpp` | Radix / Patricia Trie with edge label compression and dynamic recompression | String Data Structures |
| **Disjoint Set Union** | `include/stl/dsu.hpp` | Union by Rank with Path Compression ($\mathcal{O}(\alpha(n))$) | Advanced Graph Containers |
| **Test Runner** | `tests/test_runner.hpp` | Header-only test harness with timing measurement, C++20 type-safe assertions | Testing Framework |
| **Heap Benchmarks** | `benchmarks/benchmark_heaps.cpp` | Comprehensive microbenchmarks comparing against `std::priority_queue` | Empirical Complexity Evaluation |

---

## Build, Test & Benchmark Instructions

The project uses CMake 3.20+ targeting standard C++20 with `-Wall -Wextra -Wpedantic -O3`.

### 1. Configure and Build

```bash
cmake -B build
cmake --build build -j
```

### 2. Run All Unit Tests via CTest

```bash
ctest --test-dir build --output-on-failure
```

Or execute test suites directly:

```bash
./build/test_heaps
./build/test_compressed_trie
./build/test_dsu
```

### 3. Run Memory Safety & AddressSanitizer Suite

Verify 100% memory safety (no leaks, no undefined behavior, zero invalid accesses):

```bash
c++ -std=c++20 -fsanitize=address,undefined -g -O1 -Iinclude -Itests tests/test_heaps.cpp -o build/test_heaps_asan
./build/test_heaps_asan
```

### 4. Run Heap Performance Benchmarks

```bash
./build/benchmark_heaps
```

---

## Container Architecture & Usage

### 1. Binomial Heap (`stl::binomial_heap<T, Compare>`)

A priority queue composed of a list of binomial trees arranged in strictly increasing order of degree. Supports $\mathcal{O}(\log n)$ worst-case heap union and extraction.

```cpp
#include <stl/binomial_heap.hpp>
#include <iostream>

stl::binomial_heap<int> bh; // Default std::less<T> creates a max-heap
bh.push(15);
bh.push(42);
bh.push(7);

std::cout << "Top: " << bh.top() << "\n"; // 42
bh.pop();                                  // Pops 42

stl::binomial_heap<int> other{99, 12};
bh.merge(other); // O(log n) merge; 'other' becomes empty
std::cout << "New top after merge: " << bh.top() << "\n"; // 99
```

- Accepts custom comparator (e.g., `std::greater<T>` for min-heap).
- Supports `emplace()`, `push()`, `pop()`, `top()`, `merge()`, `update_key()`, `size()`, `empty()`, and `clear()`.
- Full Rule-of-5 compliance with deep recursive tree cloning and noexcept move operations.

---

### 2. Fibonacci Heap (`stl::fibonacci_heap<T, Compare>`)

An advanced priority queue featuring amortized $\mathcal{O}(1)$ insertion, merge, and decrease-key operations via lazy root consolidation and cascading cuts.

```cpp
#include <stl/fibonacci_heap.hpp>
#include <iostream>

stl::fibonacci_heap<int> fh;
auto h1 = fh.push(10);
auto h2 = fh.push(20);
auto h3 = fh.push(30);

// Amortized O(1) key priority update
fh.update_key(h1, 100);
std::cout << "Top: " << fh.top() << "\n"; // 100

// Amortized O(log n) arbitrary node erasure
fh.erase(h2);
std::cout << "Size: " << fh.size() << "\n"; // 2
```

- Circular doubly-linked sibling rings allow $\mathcal{O}(1)$ splicing and node insertions.
- Lazy consolidation during `pop()` groups trees using an array bounded by $\lfloor\log_\phi n\rfloor$.
- Node handles allow external tracking for efficient implementations of Dijkstra's algorithm and Prim's MST.

---

### 3. Compressed Trie (`stl::compressed_trie`)

A Patricia Trie (Radix Tree) where consecutive common paths are merged into composite edge strings.

```cpp
#include <stl/compressed_trie.hpp>
#include <iostream>

stl::compressed_trie trie{"card", "care", "careful", "careless", "cars", "cart"};

std::cout << std::boolalpha;
std::cout << "Contains 'care': " << trie.contains("care") << "\n";      // true
std::cout << "Starts with 'ca': " << trie.starts_with("ca") << "\n";     // true

// Autocompletion with optional count limit
auto suggestions = trie.autocomplete("care", 3);
for (const auto& word : suggestions) {
    std::cout << "  - " << word << "\n"; // "care", "careful", "careless"
}

// Longest common prefix lookup
std::cout << trie.longest_common_prefix("caregiver") << "\n"; // "care"
```

- Edge split and child transfer handle arbitrary prefix collisions without data duplication.
- Deletion automatically performs reverse path compression when intermediate non-terminal nodes are left with a single child.

---

### 4. Disjoint Set Union (`stl::disjoint_set`)

A high-performance Union-Find container implementing two-pass Path Compression and Union by Rank.

```cpp
#include <stl/dsu.hpp>
#include <iostream>

stl::disjoint_set dsu(6); // Sets: {0}, {1}, {2}, {3}, {4}, {5}
dsu.unite(0, 1);
dsu.unite(1, 2);
dsu.unite(3, 4);

std::cout << "0 connected to 2? " << dsu.connected(0, 2) << "\n"; // true
std::cout << "0 connected to 3? " << dsu.connected(0, 3) << "\n"; // false
std::cout << "Number of disjoint sets: " << dsu.num_sets() << "\n"; // 3
std::cout << "Set size containing 0: " << dsu.set_size(0) << "\n";   // 3

// Retrieve all grouped equivalence classes
auto groups = dsu.sets();
```

---

## Complexity Analysis ($\mathcal{O}$ Notation)

| Container | Operation | Time Complexity (Average) | Time Complexity (Worst Case) | Space Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **`stl::binomial_heap`** | `push` / `emplace` | $\mathcal{O}(1)$ amortized | $\mathcal{O}(\log n)$ | $\mathcal{O}(n)$ |
| | `top` | $\mathcal{O}(\log n)$ / $\mathcal{O}(1)$ cached | $\mathcal{O}(\log n)$ | $\mathcal{O}(1)$ |
| | `pop` | $\mathcal{O}(\log n)$ | $\mathcal{O}(\log n)$ | $\mathcal{O}(1)$ auxiliary |
| | `merge` | $\mathcal{O}(\log n)$ | $\mathcal{O}(\log n)$ | $\mathcal{O}(1)$ auxiliary |
| | `update_key` | $\mathcal{O}(\log n)$ | $\mathcal{O}(\log n)$ | $\mathcal{O}(1)$ auxiliary |
| **`stl::fibonacci_heap`** | `push` / `emplace` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | $\mathcal{O}(n)$ |
| | `top` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ |
| | `pop` | $\mathcal{O}(\log n)$ amortized | $\mathcal{O}(n)$ | $\mathcal{O}(\log n)$ auxiliary |
| | `merge` | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ | $\mathcal{O}(1)$ |
| | `update_key` | $\mathcal{O}(1)$ amortized | $\mathcal{O}(\log n)$ | $\mathcal{O}(1)$ auxiliary |
| | `erase` | $\mathcal{O}(\log n)$ amortized | $\mathcal{O}(n)$ | $\mathcal{O}(\log n)$ auxiliary |
| **`stl::compressed_trie`** | `insert` | $\mathcal{O}(k)$ | $\mathcal{O}(k \cdot |\Sigma|)$ | $\mathcal{O}(N \cdot k)$ compressed |
| | `contains` | $\mathcal{O}(k)$ | $\mathcal{O}(k \cdot |\Sigma|)$ | $\mathcal{O}(1)$ |
| | `erase` | $\mathcal{O}(k)$ | $\mathcal{O}(k \cdot |\Sigma|)$ | $\mathcal{O}(1)$ auxiliary |
| | `autocomplete` | $\mathcal{O}(k + m)$ | $\mathcal{O}(k + m)$ | $\mathcal{O}(m)$ output |
| **`stl::disjoint_set`** | `find` | $\mathcal{O}(\alpha(n))$ | $\mathcal{O}(\alpha(n))$ | $\mathcal{O}(n)$ |
| | `unite` | $\mathcal{O}(\alpha(n))$ | $\mathcal{O}(\alpha(n))$ | $\mathcal{O}(1)$ auxiliary |
| | `connected` | $\mathcal{O}(\alpha(n))$ | $\mathcal{O}(\alpha(n))$ | $\mathcal{O}(1)$ auxiliary |

*Note: $k$ denotes key string length, $m$ the number of matched descendant characters, and $\alpha(n) \le 4$ is the inverse Ackermann function.*

---

## Empirical Benchmark Results

Measured on Apple Silicon (M-series, 64-bit ARM) with AppleClang 16.0.0 and `-O3` optimization:

```
+-----------------------------+-----------------------------+-------------+---------------+-------------------+
| Container                   | Benchmark Workload          |  Operations |     Time (ms) | Throughput (ops/s)|
+-----------------------------+-----------------------------+-------------+---------------+-------------------+
| std::priority_queue         | Heavy Push (200k items)     |      200000 |       4.09 ms |     48,929,160.85 |
| stl::binomial_heap          | Heavy Push (200k items)     |      200000 |      17.63 ms |     11,344,299.49 |
| stl::fibonacci_heap         | Heavy Push (200k items)     |      200000 |       8.89 ms |     22,505,309.00 |
+-----------------------------+-----------------------------+-------------+---------------+-------------------+
| std::priority_queue         | Push + Extract (100k items) |      200000 |       7.39 ms |     27,076,573.50 |
| stl::binomial_heap          | Push + Extract (100k items) |      200000 |      44.35 ms |      4,509,171.90 |
| stl::fibonacci_heap         | Push + Extract (100k items) |      200000 |      54.27 ms |      3,685,424.48 |
+-----------------------------+-----------------------------+-------------+---------------+-------------------+
| std::priority_queue (copy)  | Merge 50 Heaps (100k items) |      100000 |       5.34 ms |     18,735,071.66 |
| stl::binomial_heap (O(logN))| Merge 50 Heaps (100k items) |      100000 |       6.06 ms |     16,506,531.30 |
| stl::fibonacci_heap (O(1))  | Merge 50 Heaps (100k items) |      100000 |       3.65 ms |     27,422,300.97 |
+-----------------------------+-----------------------------+-------------+---------------+-------------------+
| stl::fibonacci_heap         | update_key (50k items)      |       50000 |       0.16 ms |    307,849,547.77 |
+-----------------------------+-----------------------------+-------------+---------------+-------------------+
```

### Key Insights:
1. **Merge Advantage**: `stl::fibonacci_heap` executes 50-heap merging at **27.4M ops/s (3.65 ms)**, decisively outperforming `std::priority_queue` copying and rebuilding.
2. **Decrease-Key Speed**: With direct node handles and cascading cuts, `stl::fibonacci_heap` updates 50,000 keys in **0.16 ms (>307 million operations/second)**.
