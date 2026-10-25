
Advanced Data Structures: "STL Implementation from Scratch" in C++
1-Day Execution Plan & Commit Schedule (Starting at 3:00 PM)
Team Role Division
Krishna: Core Infrastructure, Self-Balancing Trees (Red-Black Tree, Splay Tree), Range Query Containers (Segment Tree with Lazy Propagation).
Husain: Advanced Heaps (Binomial Heap, Fibonacci Heap), String Containers (Compressed Trie), Graph Containers (Disjoint Set Union with Path Compression & Rank), Testing Framework.
Detailed Hour-by-Hour Timeline
Phase 1: Project Setup & Core STL Boilerplate (03:00 PM – 04:15 PM)
Objective: Establish the build environment, template base definitions, custom memory allocator wrappers, and standard iterator traits.

Krishna:

Setup folder structure: include/stl/, src/, tests/, benchmarks/.
Write stl_iterator.hpp defining bidirectional_iterator_tag, custom iterator_traits, and node base types.
Commit: chore: setup project structure and template iterator traits
Husain:

Configure CMakeLists.txt targeting C++20 with -Wall -Wextra -O3 optimization flags.
Write lightweight header-only test driver (test_runner.hpp) with timing measurement and assertion macros.
Commit: build: add CMake C++20 build target and test harness
Phase 2: Self-Balancing Trees & Advanced Priority Queues (04:15 PM – 07:00 PM)
Objective: Implement core STL-like container implementations featuring logarithmic guarantees and custom iterator access.

Krishna:

Implement rb_tree.hpp (stl::red_black_tree<T, Compare>) with left/right rotations, color balancing, insert, erase, find, begin(), and end().
Implement splay_tree.hpp (stl::splay_tree<T></t>) supporting splay operations (zig, zig-zig, zig-zag) on lookups.
Commits:
feat(trees): implement template Red-Black Tree with bidirectional iterators
feat(trees): implement Splay Tree container with self-adjusting access
Husain:

Implement binomial_heap.hpp (stl::binomial_heap<T, Compare>) supporting $O(\log n)$ key merge and extraction.
Implement fibonacci_heap.hpp (stl::fibonacci_heap<T, Compare>) with amortized $O(1)$ insertion and decrease_key.
Commits:
feat(heap): implement Binomial Heap priority queue
feat(heap): implement Fibonacci Heap container with lazy consolidation
Phase 3: Specialized Range Queries, String DS & Graph DS (07:00 PM – 09:30 PM)
Objective: Address non-standard advanced containers in the syllabus (segment trees, tries, and disjoint sets).

Krishna:

Implement segment_tree.hpp (stl::segment_tree<T, MergeOp>) supporting range queries, point updates, and lazy propagation for range updates.
Commit: feat(range): implement Segment Tree container with lazy propagation
Husain:

Implement compressed_trie.hpp (stl::compressed_trie) supporting string insertion, prefix lookup, and autocompletion routines.
Implement dsu.hpp (stl::disjoint_set) supporting path compression and union by rank.
Commits:
feat(string): implement Compressed Trie (Patricia Trie) container
feat(graph): implement Disjoint Set Union with path compression
Phase 4: Unit Testing & Performance Benchmarks (09:30 PM – 11:15 PM)
Objective: Validate exception safety, memory leaks (via AddressSanitizer), correctness, and benchmark custom containers against standard std::map and std::priority_queue.

Krishna & Husain (Joint Work):

Krishna: Write test_trees.cpp and test_segment_tree.cpp to verify invariant maintenance after dynamic insertions/deletions.
Husain: Write benchmark_heaps.cpp comparing stl::fibonacci_heap against std::priority_queue under heavy workloads.
Commits:
test: add unit test suite for trees and range containers
bench: add execution time benchmarks comparing custom heaps to std::priority_queue
Phase 5: Documentation & Submission Freeze (11:15 PM – 12:00 AM)
Objective: Complete documentation, compile final report details, and prepare code presentation.

Krishna & Husain:

Write README.md detailing time/space complexities ($\mathcal{O}$ notation), standard compliance, and execution instructions.
Commit: docs: finalize README with complexity analysis and usage documentation
Commit: chore: tagged release v1.0.0
Header File Map
File	Responsible	Syllabus Target
include/stl/stl_iterator.hpp	Krishna	Standard STL Iterator Interfaces
include/stl/rb_tree.hpp	Krishna	Balanced BST (AVL/Red-Black Tree)
include/stl/splay_tree.hpp	Krishna	Self-adjusting BST
include/stl/segment_tree.hpp	Krishna	Multidimensional & Range Query Trees
include/stl/binomial_heap.hpp	Husain	Advanced Heap Structures
include/stl/fibonacci_heap.hpp	Husain	Advanced Priority Queue
include/stl/compressed_trie.hpp	Husain	String Data Structures
include/stl/dsu.hpp	Husain	Advanced Graph Structures
