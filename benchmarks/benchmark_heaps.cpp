#include <stl/binomial_heap.hpp>
#include <stl/fibonacci_heap.hpp>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <queue>
#include <random>
#include <string>
#include <vector>

namespace {

using Clock = std::chrono::high_resolution_clock;

template <typename Func>
double measure_ms(Func&& f) {
    auto start = Clock::now();
    f();
    auto end = Clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void print_row(const std::string& container, const std::string& benchmark,
               std::size_t n, double time_ms) {
    double ops_sec = (n / (time_ms / 1000.0));
    std::cout << "| " << std::left << std::setw(22) << container
              << "| " << std::setw(28) << benchmark
              << "| " << std::right << std::setw(12) << n
              << "| " << std::fixed << std::setprecision(2) << std::setw(11) << time_ms << " ms "
              << "| " << std::fixed << std::setprecision(2) << std::setw(14) << ops_sec
              << " |\n";
}

void print_separator() {
    std::cout << "+" << std::string(23, '-')
              << "+" << std::string(29, '-')
              << "+" << std::string(13, '-')
              << "+" << std::string(15, '-')
              << "+" << std::string(17, '-')
              << "+\n";
}

} // namespace

int main() {
    std::cout << "\n=========================================================================================\n";
    std::cout << "                 STL HEAPS PERFORMANCE BENCHMARKS (C++20, -O3)                          \n";
    std::cout << "=========================================================================================\n\n";

    constexpr std::size_t N_PUSH = 200'000;
    constexpr std::size_t N_MIXED = 100'000;
    constexpr std::size_t NUM_HEAPS = 50;
    constexpr std::size_t PER_HEAP_SIZE = 2'000;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(-1'000'000, 1'000'000);

    std::vector<int> random_data(N_PUSH);
    for (auto& x : random_data) {
        x = dist(rng);
    }

    print_separator();
    std::cout << "| " << std::left << std::setw(22) << "Container"
              << "| " << std::setw(28) << "Benchmark"
              << "| " << std::right << std::setw(12) << "Operations"
              << "| " << std::setw(14) << "Time (ms)"
              << "| " << std::setw(16) << "Throughput (ops/s)"
              << " |\n";
    print_separator();

    // =========================================================================
    // 1. Heavy Push Benchmark
    // =========================================================================
    {
        double ms_std = measure_ms([&]() {
            std::priority_queue<int> pq;
            for (int val : random_data) {
                pq.push(val);
            }
        });
        print_row("std::priority_queue", "Heavy Push (200k items)", N_PUSH, ms_std);

        double ms_bin = measure_ms([&]() {
            stl::binomial_heap<int> bh;
            for (int val : random_data) {
                bh.push(val);
            }
        });
        print_row("stl::binomial_heap", "Heavy Push (200k items)", N_PUSH, ms_bin);

        double ms_fib = measure_ms([&]() {
            stl::fibonacci_heap<int> fh;
            for (int val : random_data) {
                fh.push(val);
            }
        });
        print_row("stl::fibonacci_heap", "Heavy Push (200k items)", N_PUSH, ms_fib);
    }
    print_separator();

    // =========================================================================
    // 2. Push + Full Extract Benchmark
    // =========================================================================
    {
        double ms_std = measure_ms([&]() {
            std::priority_queue<int> pq;
            for (std::size_t i = 0; i < N_MIXED; ++i) {
                pq.push(random_data[i]);
            }
            while (!pq.empty()) {
                pq.pop();
            }
        });
        print_row("std::priority_queue", "Push + Extract (100k items)", N_MIXED * 2, ms_std);

        double ms_bin = measure_ms([&]() {
            stl::binomial_heap<int> bh;
            for (std::size_t i = 0; i < N_MIXED; ++i) {
                bh.push(random_data[i]);
            }
            while (!bh.empty()) {
                bh.pop();
            }
        });
        print_row("stl::binomial_heap", "Push + Extract (100k items)", N_MIXED * 2, ms_bin);

        double ms_fib = measure_ms([&]() {
            stl::fibonacci_heap<int> fh;
            for (std::size_t i = 0; i < N_MIXED; ++i) {
                fh.push(random_data[i]);
            }
            while (!fh.empty()) {
                fh.pop();
            }
        });
        print_row("stl::fibonacci_heap", "Push + Extract (100k items)", N_MIXED * 2, ms_fib);
    }
    print_separator();

    // =========================================================================
    // 3. Heap Merge Benchmark (50 heaps of 2,000 items = 100,000 total items)
    // =========================================================================
    {
        // std::priority_queue has no merge() member function; requires copying elements
        double ms_std = measure_ms([&]() {
            std::vector<std::priority_queue<int>> heaps(NUM_HEAPS);
            for (std::size_t h = 0; h < NUM_HEAPS; ++h) {
                for (std::size_t i = 0; i < PER_HEAP_SIZE; ++i) {
                    heaps[h].push(random_data[(h * PER_HEAP_SIZE + i) % N_PUSH]);
                }
            }
            // Merge all heaps into heaps[0]
            for (std::size_t h = 1; h < NUM_HEAPS; ++h) {
                while (!heaps[h].empty()) {
                    heaps[0].push(heaps[h].top());
                    heaps[h].pop();
                }
            }
        });
        print_row("std::priority_queue (copy)", "Merge 50 Heaps (100k items)", NUM_HEAPS * PER_HEAP_SIZE, ms_std);

        double ms_bin = measure_ms([&]() {
            std::vector<stl::binomial_heap<int>> heaps(NUM_HEAPS);
            for (std::size_t h = 0; h < NUM_HEAPS; ++h) {
                for (std::size_t i = 0; i < PER_HEAP_SIZE; ++i) {
                    heaps[h].push(random_data[(h * PER_HEAP_SIZE + i) % N_PUSH]);
                }
            }
            for (std::size_t h = 1; h < NUM_HEAPS; ++h) {
                heaps[0].merge(heaps[h]);
            }
        });
        print_row("stl::binomial_heap (O(log N))", "Merge 50 Heaps (100k items)", NUM_HEAPS * PER_HEAP_SIZE, ms_bin);

        double ms_fib = measure_ms([&]() {
            std::vector<stl::fibonacci_heap<int>> heaps(NUM_HEAPS);
            for (std::size_t h = 0; h < NUM_HEAPS; ++h) {
                for (std::size_t i = 0; i < PER_HEAP_SIZE; ++i) {
                    heaps[h].push(random_data[(h * PER_HEAP_SIZE + i) % N_PUSH]);
                }
            }
            for (std::size_t h = 1; h < NUM_HEAPS; ++h) {
                heaps[0].merge(heaps[h]);
            }
        });
        print_row("stl::fibonacci_heap (O(1))", "Merge 50 Heaps (100k items)", NUM_HEAPS * PER_HEAP_SIZE, ms_fib);
    }
    print_separator();

    // =========================================================================
    // 4. Decrease-Key / Priority Update Workload
    // =========================================================================
    {
        constexpr std::size_t N_DECREASE = 50'000;
        stl::fibonacci_heap<int> fh;
        std::vector<stl::fibonacci_heap<int>::handle> handles;
        handles.reserve(N_DECREASE);

        for (std::size_t i = 0; i < N_DECREASE; ++i) {
            handles.push_back(fh.push(static_cast<int>(i)));
        }

        double ms_fib_update = measure_ms([&]() {
            for (std::size_t i = 0; i < N_DECREASE; ++i) {
                fh.update_key(handles[i], static_cast<int>(i + 1'000'000));
            }
        });
        print_row("stl::fibonacci_heap", "update_key (50k items)", N_DECREASE, ms_fib_update);
    }
    print_separator();

    std::cout << "\nBenchmark complete.\n\n";
    return 0;
}
