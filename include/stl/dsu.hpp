#pragma once

#include <numeric>
#include <stdexcept>
#include <utility>
#include <vector>

namespace stl {

/**
 * @brief Disjoint Set Union (DSU / Union-Find) data structure.
 *
 * Implements Union by Rank and Path Compression, achieving nearly constant
 * O(alpha(n)) amortized time per operation, where alpha is the inverse Ackermann function.
 *
 * Provides tracking of component sizes and total number of disjoint sets,
 * dynamic element addition, and grouping queries.
 */
class disjoint_set {
public:
    using size_type = std::size_t;

    // =========================================================================
    // Constructors, Assignment
    // =========================================================================

    disjoint_set() = default;

    /**
     * @brief Constructs a disjoint set with 'n' elements {0, 1, ..., n - 1}.
     */
    explicit disjoint_set(size_type n)
        : parent_(n), rank_(n, 0), set_size_(n, 1), num_sets_(n) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    // =========================================================================
    // Capacity & Status
    // =========================================================================

    [[nodiscard]] size_type size() const noexcept {
        return parent_.size();
    }

    [[nodiscard]] bool empty() const noexcept {
        return parent_.empty();
    }

    /**
     * @brief Returns the current number of disjoint sets (connected components).
     */
    [[nodiscard]] size_type num_sets() const noexcept {
        return num_sets_;
    }

    /**
     * @brief Returns the number of elements in the set containing 'x'.
     */
    [[nodiscard]] size_type set_size(size_type x) {
        return set_size_[find(x)];
    }

    // =========================================================================
    // Core Operations
    // =========================================================================

    /**
     * @brief Finds the representative of the set containing 'x' with path compression.
     * Amortized O(alpha(n)).
     */
    [[nodiscard]] size_type find(size_type x) {
        validate_element(x);
        size_type root = x;
        while (root != parent_[root]) {
            root = parent_[root];
        }
        // Path compression
        size_type curr = x;
        while (curr != root) {
            size_type next = parent_[curr];
            parent_[curr] = root;
            curr = next;
        }
        return root;
    }

    /**
     * @brief Merges the set containing 'x' with the set containing 'y' using Union by Rank.
     * @return true if the sets were merged; false if they were already in the same set.
     * Amortized O(alpha(n)).
     */
    bool unite(size_type x, size_type y) {
        size_type root_x = find(x);
        size_type root_y = find(y);

        if (root_x == root_y) {
            return false;
        }

        // Union by rank
        if (rank_[root_x] < rank_[root_y]) {
            std::swap(root_x, root_y);
        }

        parent_[root_y] = root_x;
        set_size_[root_x] += set_size_[root_y];

        if (rank_[root_x] == rank_[root_y]) {
            ++rank_[root_x];
        }

        --num_sets_;
        return true;
    }

    /**
     * @brief Checks if 'x' and 'y' belong to the same set.
     */
    [[nodiscard]] bool connected(size_type x, size_type y) {
        return find(x) == find(y);
    }

    /**
     * @brief Dynamically adds a new element to the disjoint set.
     * @return The index assigned to the new element.
     */
    size_type add_element() {
        size_type new_id = parent_.size();
        parent_.push_back(new_id);
        rank_.push_back(0);
        set_size_.push_back(1);
        ++num_sets_;
        return new_id;
    }

    /**
     * @brief Resets all elements into disjoint singleton sets.
     */
    void reset() {
        std::iota(parent_.begin(), parent_.end(), 0);
        std::fill(rank_.begin(), rank_.end(), 0);
        std::fill(set_size_.begin(), set_size_.end(), 1);
        num_sets_ = parent_.size();
    }

    /**
     * @brief Groups and returns all disjoint sets as lists of element indices.
     */
    [[nodiscard]] std::vector<std::vector<size_type>> sets() {
        std::vector<std::vector<size_type>> root_to_elements(parent_.size());
        for (size_type i = 0; i < parent_.size(); ++i) {
            root_to_elements[find(i)].push_back(i);
        }

        std::vector<std::vector<size_type>> result;
        result.reserve(num_sets_);
        for (auto& group : root_to_elements) {
            if (!group.empty()) {
                result.push_back(std::move(group));
            }
        }
        return result;
    }

    void swap(disjoint_set& other) noexcept {
        using std::swap;
        swap(parent_, other.parent_);
        swap(rank_, other.rank_);
        swap(set_size_, other.set_size_);
        swap(num_sets_, other.num_sets_);
    }

private:
    std::vector<size_type> parent_;
    std::vector<size_type> rank_;
    std::vector<size_type> set_size_;
    size_type num_sets_{0};

    void validate_element(size_type x) const {
        if (x >= parent_.size()) {
            throw std::out_of_range("disjoint_set: index out of bounds");
        }
    }
};

inline void swap(disjoint_set& lhs, disjoint_set& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace stl
