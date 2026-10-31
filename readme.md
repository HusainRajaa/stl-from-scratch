# STL from Scratch: Advanced Heaps, Trie, Graph DSU & Benchmarks

## Run the C++ demos

From the project root, build and run all demos with:

```sh
make cpp
```

`make run` remains available as an equivalent command.

Each file is a standalone C++20 program with its own `main()`:

| Source                      | Command                      |
| --------------------------- | ---------------------------- |
| `src/demo_trees.cpp`        | `make run-demo_trees`        |
| `src/demo_segment_tree.cpp` | `make run-demo_segment_tree` |
| `src/demo_heaps.cpp`        | `make run-demo_heaps`        |
| `src/demo_trie.cpp`         | `make run-demo_trie`         |
| `src/demo_dsu.cpp`          | `make run-demo_dsu`          |

The demos print operations and results; they do not require interactive input.
Use `make test` for tests only, or `make` for tests and demos together.

In WSL, keep Linux binaries separate from Windows builds:

```sh
cd /mnt/c/Users/DELL/OneDrive/Desktop/stl-from-scratch
make BUILD_DIR=build/wsl cpp
```

You can also compile any demo directly:

```sh
mkdir -p build/examples
g++ -std=c++20 -Wall -Wextra -Wpedantic -Iinclude src/demo_dsu.cpp -o build/examples/demo_dsu.exe
./build/examples/demo_dsu.exe
```

CMake builds the same demos as targets named `demo_trees`, `demo_segment_tree`,
`demo_heaps`, `demo_trie`, and `demo_dsu`. CTest also runs them as smoke tests.

Advanced Data Structures coursework: custom header-only C++20 standard template library containers implemented from scratch. This branch (`dev_husain`) implements Husain's component portfolio: Advanced Heaps, String Containers, Graph Disjoint Sets, C++20 Test Harness, and Performance Benchmarks.

---

## Portfolio Division (Husain's Implementation)

| Container / Module     | Header File                       | Key Data Structure & Algorithm                                               | Syllabus Target                  |
| :--------------------- | :-------------------------------- | :--------------------------------------------------------------------------- | :------------------------------- |
| **Binomial Heap**      | `include/stl/binomial_heap.hpp`   | Collection of binomial trees ($B_k$), binary addition union                  | Advanced Priority Queues         |
| **Fibonacci Heap**     | `include/stl/fibonacci_heap.hpp`  | Circular doubly-linked root lists, lazy consolidation, cascading cuts        | Amortized Optimal Priority Queue |
| **Compressed Trie**    | `include/stl/compressed_trie.hpp` | Radix / Patricia Trie with edge label compression and dynamic recompression  | String Data Structures           |
| **Disjoint Set Union** | `include/stl/dsu.hpp`             | Union by Rank with Path Compression ($\mathcal{O}(\alpha(n))$)               | Advanced Graph Containers        |
| **Test Runner**        | `tests/test_runner.hpp`           | Header-only test harness with timing measurement, C++20 type-safe assertions | Testing Framework                |
| **Heap Benchmarks**    | `benchmarks/benchmark_heaps.cpp`  | Comprehensive microbenchmarks comparing against `std::priority_queue`        | Empirical Complexity Evaluation  |

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

| Container                  | Operation          | Time Complexity (Average)                       | Time Complexity (Worst Case) | Space Complexity                |
| :------------------------- | :----------------- | :---------------------------------------------- | :--------------------------- | :------------------------------ |
| **`stl::binomial_heap`**   | `push` / `emplace` | $\mathcal{O}(1)$ amortized                      | $\mathcal{O}(\log n)$        | $\mathcal{O}(n)$                |
|                            | `top`              | $\mathcal{O}(\log n)$ / $\mathcal{O}(1)$ cached | $\mathcal{O}(\log n)$        | $\mathcal{O}(1)$                |
|                            | `pop`              | $\mathcal{O}(\log n)$                           | $\mathcal{O}(\log n)$        | $\mathcal{O}(1)$ auxiliary      |
|                            | `merge`            | $\mathcal{O}(\log n)$                           | $\mathcal{O}(\log n)$        | $\mathcal{O}(1)$ auxiliary      |
|                            | `update_key`       | $\mathcal{O}(\log n)$                           | $\mathcal{O}(\log n)$        | $\mathcal{O}(1)$ auxiliary      |
| **`stl::fibonacci_heap`**  | `push` / `emplace` | $\mathcal{O}(1)$                                | $\mathcal{O}(1)$             | $\mathcal{O}(n)$                |
|                            | `top`              | $\mathcal{O}(1)$                                | $\mathcal{O}(1)$             | $\mathcal{O}(1)$                |
|                            | `pop`              | $\mathcal{O}(\log n)$ amortized                 | $\mathcal{O}(n)$             | $\mathcal{O}(\log n)$ auxiliary |
|                            | `merge`            | $\mathcal{O}(1)$                                | $\mathcal{O}(1)$             | $\mathcal{O}(1)$                |
|                            | `update_key`       | $\mathcal{O}(1)$ amortized                      | $\mathcal{O}(\log n)$        | $\mathcal{O}(1)$ auxiliary      |
|                            | `erase`            | $\mathcal{O}(\log n)$ amortized                 | $\mathcal{O}(n)$             | $\mathcal{O}(\log n)$ auxiliary |
| **`stl::compressed_trie`** | `insert`           | $\mathcal{O}(k)$                                | $\mathcal{O}(k \cdot         | \Sigma                          | )$  | $\mathcal{O}(N \cdot k)$ compressed |
|                            | `contains`         | $\mathcal{O}(k)$                                | $\mathcal{O}(k \cdot         | \Sigma                          | )$  | $\mathcal{O}(1)$                    |
|                            | `erase`            | $\mathcal{O}(k)$                                | $\mathcal{O}(k \cdot         | \Sigma                          | )$  | $\mathcal{O}(1)$ auxiliary          |
|                            | `autocomplete`     | $\mathcal{O}(k + m)$                            | $\mathcal{O}(k + m)$         | $\mathcal{O}(m)$ output         |
| **`stl::disjoint_set`**    | `find`             | $\mathcal{O}(\alpha(n))$                        | $\mathcal{O}(\alpha(n))$     | $\mathcal{O}(n)$                |
|                            | `unite`            | $\mathcal{O}(\alpha(n))$                        | $\mathcal{O}(\alpha(n))$     | $\mathcal{O}(1)$ auxiliary      |
|                            | `connected`        | $\mathcal{O}(\alpha(n))$                        | $\mathcal{O}(\alpha(n))$     | $\mathcal{O}(1)$ auxiliary      |

_Note: $k$ denotes key string length, $m$ the number of matched descendant characters, and $\alpha(n) \le 4$ is the inverse Ackermann function._

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
   \=======

# STL from Scratch

Advanced Data Structures coursework: header-only C++20 containers implemented
without using standard-library containers as substitutes for the tree algorithms.
Standard allocation, traits, and contiguous storage utilities are used where
appropriate. This is an educational STL-like library, not a drop-in standard
library replacement.

## Team scope

Krishna's implementation on `dev_krishna`:

- `include/stl/stl_iterator.hpp`: standard-compatible iterator tags, traits
  (including raw pointers), and a key-free tree node base.
- `include/stl/stl_memory.hpp`: allocator-aware object construction and cleanup,
  shared by both trees.
- `include/stl/rb_tree.hpp`: red-black tree with insertion/deletion balancing,
  allocator-backed node construction, bidirectional iteration, and validation.
- `include/stl/splay_tree.hpp`: zig, zig-zig, and zig-zag self-adjusting tree;
  iterative copying and destruction handle deeply skewed trees.
- `include/stl/segment_tree.hpp`: configurable aggregation and lazy range updates.
- `tests/test_iterator.cpp`, `tests/test_memory.cpp`, `tests/test_trees.cpp`, and
  `tests/test_segment_tree.cpp`: standalone correctness tests.

`src/` contains runnable demos for the merged containers. `benchmarks/` contains
Husain's heap performance benchmark. The merged project also includes his test
harness, CMake configuration, heaps, trie, and DSU.

## Build and test

A C++20 compiler, GNU Make, and a POSIX shell are required. Run from the
repository root in Git Bash, MSYS2, or a POSIX shell:

```sh
make
```

This builds and runs every test suite and demo, stopping on a build error or
nonzero exit status.
On Windows, `mingw32-make` can be used instead of `make`; Git Bash/MSYS2's `sh`,
`mkdir`, and `rm` must be on `PATH`.

| Command                | Action                                                          |
| ---------------------- | --------------------------------------------------------------- |
| `make`                 | Build and run all tests and demos                               |
| `make test`            | Build and run all tests                                         |
| `make run`             | Build and run all demos                                         |
| `make build`           | Compile tests and demos without running them                    |
| `make debug`           | Build and run with debug symbols and GCC/libstdc++ debug checks |
| `make run-test_trees`  | Build and run only the tree suite                               |
| `make run-test_memory` | Build and run only allocation tests                             |
| `make clean`           | Remove generated files under `build/`                           |
| `make -j4`             | Build and run suites in parallel                                |

The Makefile discovers standalone `tests/*.cpp` and `src/demo_*.cpp` executables
automatically. Each file must have its own `main()`. New tests and demos require
no manual source-list update;
compiler-generated dependencies rebuild affected tests when headers change.
Keep the Makefile updated when adding other executable types, shared `.cpp`
implementation files, or new build requirements.

Release and debug outputs are separate under `build/release/` and `build/debug/`.
Override the compiler with `make CXX=clang++`; extra libraries can be supplied via
`LDLIBS`. When changing flags within the same build directory, run `make clean`
first. Test checks remain active with `NDEBUG` defined.

On a GCC/Clang installation with sanitizer runtimes, additionally run:

```sh
g++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/test_trees.cpp -o build/test_trees_sanitized.exe
g++ -std=c++20 -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer -Iinclude tests/test_segment_tree.cpp -o build/test_segment_tree_sanitized.exe
./build/test_trees_sanitized.exe
./build/test_segment_tree_sanitized.exe
```

Local validation: GCC 15.2/MinGW passes all four suites in optimized and debug
builds with warnings treated as errors. Sanitizer validation is **not completed**:
the installed MinGW toolchain cannot link `libasan` or `libubsan`.

## Allocation helpers

`include/stl/stl_memory.hpp` provides `stl::allocate_object(allocator, args...)`
and `stl::deallocate_object(allocator, pointer)`. The helpers use
`std::allocator_traits`, forward constructor arguments, and release the allocated
slot if construction throws. They support over-aligned types and allocator
pointer types through `std::to_address`.

Deallocation accepts a null pointer as a no-op. Otherwise, pass a live object
returned by `allocate_object` and the original allocator or an equivalent copy.
Object destruction and allocator deallocation must not throw. These are wrappers
around an allocator, not a replacement memory pool. Both trees use them with
`std::allocator` internally; their public template parameters remain unchanged.

## Trees

```cpp
#include <stl/rb_tree.hpp>
#include <stl/splay_tree.hpp>

stl::red_black_tree<int> ordered;
auto [position, inserted] = ordered.insert(8);
ordered.insert(3);
ordered.insert(12);
ordered.erase(8);                       // Returns number erased: 0 or 1.
auto found = ordered.find(12);          // end() if absent.
if (found != ordered.end()) {
    found = ordered.erase(found);      // Returns the successor.
}
for (const int value : ordered) { /* Sorted, unique keys. */ }

stl::splay_tree<int> adaptive;
adaptive.insert(4);
adaptive.insert(9);
adaptive.find(4);                      // Moves 4 to the root.
```

Both trees accept an optional `Compare` type and comparator constructor argument,
using `std::less<T>` by default. Keys are unique under comparator equivalence and
must not be modified through iterators. Keys need not be default-constructible.
They support `size`, `empty`, `clear`, `contains`, const iteration, `--end()` on
nonempty trees, independent copying, moving, and `swap`. `validate()` checks
ordering, parent links, and node count; red-black validation also checks coloring
and equal black heights. Comparators must impose a strict weak ordering.

A non-const splay lookup splays the matching node, or the last visited node on a
miss. Const lookups do not modify the tree and do not gain amortized lookup
bounds from splaying.

Insertion and rotations preserve existing element iterators; erasure invalidates
only the erased element. Red-black element iterators survive move/swap, but
past-the-end iterators do not transfer. Treat all splay iterators as invalidated
by move/swap because they retain an owning-container pointer. As with STL
iterators, do not dereference `end()`, decrement `begin()`, or erase an iterator
from a different container. `erase(iterator)` requires a dereferenceable iterator.

Tree insertion performs comparisons and construction before linking a new node;
failed insertion leaves existing contents unchanged. Failed key copying cleans
up partially cloned nodes. Strong copy-assignment guarantees require nonthrowing
comparator swap; throwing comparator moves/swaps retain the comparator's own
exception-safety limitations. Destructors of stored values must not throw.
The containers do not expose a user-supplied allocator template parameter.

## Lazy segment tree

All ranges are **half-open `[first, last)`**. Point updates assign a value;
default range updates add a delta to every element.

```cpp
#include <stl/segment_tree.hpp>
#include <vector>

std::vector<long long> values{1, 2, 3, 4};
stl::segment_tree<long long> sums(values, 0LL); // Explicit sum identity.
sums.range_update(1, 4, 5);                    // Values: 1, 7, 8, 9.
sums.point_update(2, 10);                      // Values: 1, 7, 10, 9.
auto total = sums.query(0, 4);                // 27.
auto middle = sums.query(1, 3);               // 17.
```

Construction also accepts `(first_iterator, last_sentinel, identity, ...)`.
Empty queries return the supplied identity, empty updates do nothing, and
invalid/inverted ranges or invalid point indices throw `std::out_of_range`.
Queries are const and do not push lazy tags or mutate the tree. Copies are
independent; moved-from trees are empty and may be reassigned or destroyed.

For custom aggregation, use
`segment_tree<T, MergeOp, ApplyOp, ComposeOp>` and provide matching policies:

- `merge(left, right)` is associative with the supplied two-sided identity.
- `apply(aggregate, delta, length)` updates an aggregate for `length` elements.
- `compose(old_delta, new_delta)` represents applying **old first, new second**.
- Applying an update must distribute over merging adjacent segments.

Values and update payloads share type `T`. Policies must be const-callable.
Pending tags use `std::optional<T>` so no update identity is assumed. Defaults
implement sums with additive updates; changing only `MergeOp` does **not** make
those default lazy policies correct for arbitrary aggregation. For minimum
queries with additive updates, for example:

```cpp
#include <algorithm>
#include <limits>

struct minimum {
    int operator()(int a, int b) const { return std::min(a, b); }
};
struct add_to_minimum {
    int operator()(int aggregate, int delta, std::size_t) const {
        return aggregate + delta;
    }
};
stl::segment_tree<int, minimum, add_to_minimum> minima(
    std::vector<int>{8, 2, 6}, std::numeric_limits<int>::max());
minima.range_update(0, 2, 3);
auto smallest = minima.query(0, 3); // 5.
```

Use nonthrowing value assignments and update/merge policies for reliable mutable
segment-tree operations: exceptions during an update or assignment do not have
a transactional rollback guarantee. Numeric overflow is the caller's
responsibility. Structural range errors are checked before changing the tree.

## Complexity

Bounds assume constant-time comparisons, policy calls, and value operations.
`n` is the number of stored elements and `h` is a tree's current height.

| Operation                                 | Red-black tree              | Splay tree                            | Segment tree                    |
| ----------------------------------------- | --------------------------- | ------------------------------------- | ------------------------------- |
| Insert / erase by key                     | O(log n) worst case         | O(log n) amortized; O(n) worst case   | Not supported                   |
| Non-const lookup                          | O(log n)                    | O(log n) amortized; O(n) worst case   | Not applicable                  |
| Const lookup                              | O(log n)                    | O(h), up to O(n)                      | Not applicable                  |
| Range query / point update / range update | Not supported               | Not supported                         | O(log n), O(1) for empty ranges |
| Construction from n values                | Repeated insert: O(n log n) | Repeated insert: O(n log n) amortized | O(n)                            |
| Copy / clear                              | O(n)                        | O(n)                                  | Copy: O(n)                      |
| Full traversal / validation               | O(n)                        | O(n)                                  | Not applicable                  |
| Storage                                   | O(n)                        | O(n)                                  | O(n)                            |

Red-black `begin()` and `--end()` are O(1) via cached extrema. Splay `begin()`
and `--end()` are O(h). Individual iterator steps may take O(h), while a full
traversal is O(n). `size()` and `empty()` are O(1) for all containers.

## Test coverage

- Iterator concepts, read-only key references, and custom traits.
- Allocator construction failure cleanup, failed allocations, move-only constructor
  arguments, over-aligned storage, null deallocation, and live-object tracking.
- Seeded differential tree tests against `std::set`, ascending/descending
  comparators, duplicate keys, bidirectional traversal, iterator erasure,
  copies, moves, and non-default-constructible keys.
- Copy-failure injection with tracked live keys and 60,000-node skew stress.
- Splay hit/miss behavior and red-black invariants after dynamic mutations.
- Seeded segment-tree comparisons against vector sums, point assignments after
  pending updates, range bounds, empty/singleton inputs, custom minimum merges,
  chronological noncommutative update composition, and lazy-operation counts.
