#include "test_runner.hpp"
#include <stl/binomial_heap.hpp>
#include <stl/fibonacci_heap.hpp>
#include <queue>
#include <random>
#include <vector>

TEST_CASE(BinomialHeapTest, BasicPushPopTop) {
    stl::binomial_heap<int> bh;
    ASSERT_TRUE(bh.empty());
    ASSERT_EQ(bh.size(), 0);

    bh.push(10);
    bh.push(30);
    bh.push(20);
    bh.push(5);

    ASSERT_FALSE(bh.empty());
    ASSERT_EQ(bh.size(), 4);
    ASSERT_EQ(bh.top(), 30);

    bh.pop();
    ASSERT_EQ(bh.size(), 3);
    ASSERT_EQ(bh.top(), 20);

    bh.pop();
    ASSERT_EQ(bh.top(), 10);

    bh.pop();
    ASSERT_EQ(bh.top(), 5);

    bh.pop();
    ASSERT_TRUE(bh.empty());
    ASSERT_EQ(bh.size(), 0);
}

TEST_CASE(BinomialHeapTest, MinHeapComparator) {
    stl::binomial_heap<int, std::greater<int>> min_bh;
    min_bh.push(15);
    min_bh.push(5);
    min_bh.push(25);
    min_bh.push(1);

    ASSERT_EQ(min_bh.top(), 1);
    min_bh.pop();
    ASSERT_EQ(min_bh.top(), 5);
    min_bh.pop();
    ASSERT_EQ(min_bh.top(), 15);
    min_bh.pop();
    ASSERT_EQ(min_bh.top(), 25);
    min_bh.pop();
    ASSERT_TRUE(min_bh.empty());
}

TEST_CASE(BinomialHeapTest, Merge) {
    stl::binomial_heap<int> h1;
    stl::binomial_heap<int> h2;

    h1.push(10);
    h1.push(50);
    h1.push(20);

    h2.push(35);
    h2.push(5);
    h2.push(70);

    h1.merge(h2);

    ASSERT_EQ(h1.size(), 6);
    ASSERT_TRUE(h2.empty());
    ASSERT_EQ(h1.top(), 70);

    std::vector<int> extracted;
    while (!h1.empty()) {
        extracted.push_back(h1.top());
        h1.pop();
    }
    std::vector<int> expected = {70, 50, 35, 20, 10, 5};
    ASSERT_EQ(extracted.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_EQ(expected[i], extracted[i]);
    }
}

TEST_CASE(BinomialHeapTest, CopyAndMove) {
    stl::binomial_heap<int> h1;
    for (int i = 1; i <= 10; ++i) {
        h1.push(i);
    }

    stl::binomial_heap<int> h2 = h1; // Copy
    ASSERT_EQ(h2.size(), 10);
    ASSERT_EQ(h2.top(), 10);

    stl::binomial_heap<int> h3 = std::move(h1); // Move
    ASSERT_EQ(h3.size(), 10);
    ASSERT_TRUE(h1.empty());

    while (!h2.empty()) {
        ASSERT_EQ(h2.top(), h3.top());
        h2.pop();
        h3.pop();
    }
}

TEST_CASE(BinomialHeapTest, UpdateKey) {
    stl::binomial_heap<int> bh;
    auto h1 = bh.push(10);
    auto h2 = bh.push(20);
    bh.push(30);

    ASSERT_EQ(bh.top(), 30);
    bh.update_key(h1, 50);
    ASSERT_EQ(bh.top(), 50);

    ASSERT_THROWS(bh.update_key(h2, 5), std::invalid_argument);
}

TEST_CASE(BinomialHeapTest, DifferentialStressTest) {
    stl::binomial_heap<int> custom_heap;
    std::priority_queue<int> std_heap;

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(-10000, 10000);

    for (int i = 0; i < 5000; ++i) {
        int val = dist(rng);
        custom_heap.push(val);
        std_heap.push(val);
        ASSERT_EQ(custom_heap.top(), std_heap.top());
        ASSERT_EQ(custom_heap.size(), std_heap.size());
    }

    while (!std_heap.empty()) {
        ASSERT_EQ(custom_heap.top(), std_heap.top());
        custom_heap.pop();
        std_heap.pop();
        ASSERT_EQ(custom_heap.size(), std_heap.size());
    }
    ASSERT_TRUE(custom_heap.empty());
}

TEST_CASE(FibonacciHeapTest, BasicPushPopTop) {
    stl::fibonacci_heap<int> fh;
    ASSERT_TRUE(fh.empty());
    ASSERT_EQ(fh.size(), 0);

    fh.push(10);
    fh.push(30);
    fh.push(20);
    fh.push(5);

    ASSERT_FALSE(fh.empty());
    ASSERT_EQ(fh.size(), 4);
    ASSERT_EQ(fh.top(), 30);

    fh.pop();
    ASSERT_EQ(fh.size(), 3);
    ASSERT_EQ(fh.top(), 20);

    fh.pop();
    ASSERT_EQ(fh.top(), 10);

    fh.pop();
    ASSERT_EQ(fh.top(), 5);

    fh.pop();
    ASSERT_TRUE(fh.empty());
    ASSERT_EQ(fh.size(), 0);
}

TEST_CASE(FibonacciHeapTest, MinHeapComparator) {
    stl::fibonacci_heap<int, std::greater<int>> min_fh;
    min_fh.push(15);
    min_fh.push(5);
    min_fh.push(25);
    min_fh.push(1);

    ASSERT_EQ(min_fh.top(), 1);
    min_fh.pop();
    ASSERT_EQ(min_fh.top(), 5);
    min_fh.pop();
    ASSERT_EQ(min_fh.top(), 15);
    min_fh.pop();
    ASSERT_EQ(min_fh.top(), 25);
    min_fh.pop();
    ASSERT_TRUE(min_fh.empty());
}

TEST_CASE(FibonacciHeapTest, Merge) {
    stl::fibonacci_heap<int> h1;
    stl::fibonacci_heap<int> h2;

    h1.push(10);
    h1.push(50);
    h1.push(20);

    h2.push(35);
    h2.push(5);
    h2.push(70);

    h1.merge(h2);

    ASSERT_EQ(h1.size(), 6);
    ASSERT_TRUE(h2.empty());
    ASSERT_EQ(h1.top(), 70);

    std::vector<int> extracted;
    while (!h1.empty()) {
        extracted.push_back(h1.top());
        h1.pop();
    }
    std::vector<int> expected = {70, 50, 35, 20, 10, 5};
    ASSERT_EQ(extracted.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_EQ(expected[i], extracted[i]);
    }
}

TEST_CASE(FibonacciHeapTest, UpdateKeyAndErase) {
    stl::fibonacci_heap<int> fh;
    auto h1 = fh.push(10);
    auto h2 = fh.push(20);
    auto h3 = fh.push(30);
    fh.push(40);

    ASSERT_EQ(fh.top(), 40);
    fh.update_key(h1, 100);
    ASSERT_EQ(fh.top(), 100);

    ASSERT_THROWS(fh.update_key(h2, 5), std::invalid_argument);

    fh.erase(h3);
    ASSERT_EQ(fh.size(), 3);
    ASSERT_EQ(fh.top(), 100);

    fh.pop(); // pops 100
    ASSERT_EQ(fh.top(), 40);
    fh.pop(); // pops 40
    ASSERT_EQ(fh.top(), 20);
    fh.pop();
    ASSERT_TRUE(fh.empty());
}

TEST_CASE(FibonacciHeapTest, DifferentialStressTest) {
    stl::fibonacci_heap<int> custom_heap;
    std::priority_queue<int> std_heap;

    std::mt19937 rng(1337);
    std::uniform_int_distribution<int> dist(-10000, 10000);

    for (int i = 0; i < 5000; ++i) {
        int val = dist(rng);
        custom_heap.push(val);
        std_heap.push(val);
        ASSERT_EQ(custom_heap.top(), std_heap.top());
        ASSERT_EQ(custom_heap.size(), std_heap.size());
    }

    while (!std_heap.empty()) {
        ASSERT_EQ(custom_heap.top(), std_heap.top());
        custom_heap.pop();
        std_heap.pop();
        ASSERT_EQ(custom_heap.size(), std_heap.size());
    }
    ASSERT_TRUE(custom_heap.empty());
}

int main() {
    return RUN_ALL_TESTS();
}
