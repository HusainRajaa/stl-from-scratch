#include "test_runner.hpp"
#include <stl/dsu.hpp>
#include <numeric>
#include <vector>

TEST_CASE(DSUTest, InitialState) {
    stl::disjoint_set dsu(5);
    ASSERT_EQ(dsu.size(), 5);
    ASSERT_EQ(dsu.num_sets(), 5);

    for (size_t i = 0; i < 5; ++i) {
        ASSERT_EQ(dsu.find(i), i);
        ASSERT_EQ(dsu.set_size(i), 1);
    }
}

TEST_CASE(DSUTest, UniteAndFind) {
    stl::disjoint_set dsu(10);

    // Merge 0-1, 2-3, 4-5
    ASSERT_TRUE(dsu.unite(0, 1));
    ASSERT_TRUE(dsu.unite(2, 3));
    ASSERT_TRUE(dsu.unite(4, 5));

    // Redundant unite
    ASSERT_FALSE(dsu.unite(0, 1));
    ASSERT_FALSE(dsu.unite(1, 0));

    ASSERT_EQ(dsu.num_sets(), 7);
    ASSERT_TRUE(dsu.connected(0, 1));
    ASSERT_TRUE(dsu.connected(2, 3));
    ASSERT_TRUE(dsu.connected(4, 5));
    ASSERT_FALSE(dsu.connected(0, 2));

    ASSERT_EQ(dsu.set_size(0), 2);
    ASSERT_EQ(dsu.set_size(1), 2);
    ASSERT_EQ(dsu.set_size(2), 2);

    // Merge sets {0, 1} and {2, 3}
    ASSERT_TRUE(dsu.unite(1, 2));
    ASSERT_EQ(dsu.num_sets(), 6);
    ASSERT_TRUE(dsu.connected(0, 3));
    ASSERT_EQ(dsu.set_size(0), 4);
    ASSERT_EQ(dsu.set_size(3), 4);
}

TEST_CASE(DSUTest, DynamicAddElement) {
    stl::disjoint_set dsu(3);
    ASSERT_EQ(dsu.size(), 3);

    size_t id1 = dsu.add_element();
    ASSERT_EQ(id1, 3);
    ASSERT_EQ(dsu.size(), 4);
    ASSERT_EQ(dsu.num_sets(), 4);

    size_t id2 = dsu.add_element();
    ASSERT_EQ(id2, 4);
    ASSERT_EQ(dsu.size(), 5);

    dsu.unite(3, 4);
    ASSERT_TRUE(dsu.connected(3, 4));
    ASSERT_EQ(dsu.set_size(3), 2);
}

TEST_CASE(DSUTest, SetsGrouping) {
    stl::disjoint_set dsu(6);
    dsu.unite(0, 1);
    dsu.unite(1, 2);
    dsu.unite(3, 4);
    // Element 5 is standalone

    auto groups = dsu.sets();
    ASSERT_EQ(groups.size(), 3);

    size_t total_elements = 0;
    for (const auto& group : groups) {
        total_elements += group.size();
    }
    ASSERT_EQ(total_elements, 6);
}

TEST_CASE(DSUTest, ResetAndOutOfBounds) {
    stl::disjoint_set dsu(4);
    dsu.unite(0, 1);
    dsu.unite(2, 3);
    ASSERT_EQ(dsu.num_sets(), 2);

    dsu.reset();
    ASSERT_EQ(dsu.num_sets(), 4);
    ASSERT_FALSE(dsu.connected(0, 1));

    ASSERT_THROWS(dsu.find(10), std::out_of_range);
    ASSERT_THROWS(dsu.unite(0, 10), std::out_of_range);
}

int main() {
    return RUN_ALL_TESTS();
}
