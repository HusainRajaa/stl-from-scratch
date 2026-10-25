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

`src/` and `benchmarks/` are reserved for future non-header code and benchmarks.
Husain's CMake configuration, test harness, heaps, trie, DSU, and heap benchmarks
are intentionally outside this branch's implementation scope.

## Build and test

A C++20 compiler, GNU Make, and a POSIX shell are required. Run from the
repository root in Git Bash, MSYS2, or a POSIX shell:

```sh
make
```

This builds and runs every test suite, stopping on a build error or failed test.
On Windows, `mingw32-make` can be used instead of `make`; Git Bash/MSYS2's `sh`,
`mkdir`, and `rm` must be on `PATH`.

| Command                           | Action                                                          |
| --------------------------------- | --------------------------------------------------------------- |
| `make` / `make test` / `make run` | Build and run all tests                                         |
| `make build`                      | Compile without running tests                                   |
| `make debug`                      | Build and run with debug symbols and GCC/libstdc++ debug checks |
| `make run-test_trees`             | Build and run only the tree suite                               |
| `make run-test_memory`            | Build and run only allocation tests                             |
| `make clean`                      | Remove generated files under `build/`                           |
| `make -j4`                        | Build and run suites in parallel                                |

The Makefile discovers standalone `tests/*.cpp` executables automatically. Each
file must have its own `main()`. New tests require no manual source-list update;
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
