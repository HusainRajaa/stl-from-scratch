#pragma once

#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace stl {

/**
 * @brief Compressed Trie (Patricia Trie / Radix Tree) for string storage and prefix queries.
 *
 * Each edge stores a string label. Nodes with single children and no terminal status
 * are compressed, ensuring compact representation and fast prefix matching.
 *
 * Supports string insertion, erasure with dynamic path re-compression, exact lookup,
 * prefix testing, autocompletion in lexicographical order, and longest prefix matching.
 */
class compressed_trie {
private:
    struct Node {
        std::string edge; // Label on the incoming edge from parent
        bool is_terminal{false};
        std::map<char, std::unique_ptr<Node>> children;

        explicit Node(std::string edge_label = "", bool terminal = false)
            : edge(std::move(edge_label)), is_terminal(terminal) {}

        [[nodiscard]] bool is_leaf() const noexcept {
            return children.empty();
        }
    };

    std::unique_ptr<Node> root_;
    std::size_t size_{0};

    static std::size_t common_prefix_length(std::string_view a, std::string_view b) noexcept {
        std::size_t len = 0;
        const std::size_t max_len = std::min(a.size(), b.size());
        while (len < max_len && a[len] == b[len]) {
            ++len;
        }
        return len;
    }

    static std::unique_ptr<Node> clone_subtree(const Node* src) {
        if (!src) return nullptr;
        auto cloned = std::make_unique<Node>(src->edge, src->is_terminal);
        for (const auto& [ch, child] : src->children) {
            cloned->children[ch] = clone_subtree(child.get());
        }
        return cloned;
    }

public:
    using size_type = std::size_t;

    // =========================================================================
    // Constructors, Destructor, Assignment
    // =========================================================================

    compressed_trie() : root_(std::make_unique<Node>("", false)), size_(0) {}

    template <typename InputIt>
    compressed_trie(InputIt first, InputIt last) : compressed_trie() {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    compressed_trie(std::initializer_list<std::string_view> init) : compressed_trie() {
        for (const auto& str : init) {
            insert(str);
        }
    }

    ~compressed_trie() = default;

    compressed_trie(const compressed_trie& other)
        : root_(clone_subtree(other.root_.get())), size_(other.size_) {}

    compressed_trie(compressed_trie&& other) noexcept
        : root_(std::move(other.root_)), size_(std::exchange(other.size_, 0)) {
        if (!root_) {
            root_ = std::make_unique<Node>("", false);
        }
    }

    compressed_trie& operator=(const compressed_trie& other) {
        if (this != &other) {
            root_ = clone_subtree(other.root_.get());
            size_ = other.size_;
        }
        return *this;
    }

    compressed_trie& operator=(compressed_trie&& other) noexcept {
        if (this != &other) {
            root_ = std::move(other.root_);
            size_ = std::exchange(other.size_, 0);
            if (!root_) {
                root_ = std::make_unique<Node>("", false);
            }
        }
        return *this;
    }

    // =========================================================================
    // Capacity & Status
    // =========================================================================

    [[nodiscard]] bool empty() const noexcept {
        return size_ == 0;
    }

    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }

    void clear() noexcept {
        root_ = std::make_unique<Node>("", false);
        size_ = 0;
    }

    // =========================================================================
    // Modifiers
    // =========================================================================

    /**
     * @brief Inserts a key into the compressed trie.
     * @return true if inserted, false if the key already existed.
     */
    bool insert(std::string_view key) {
        if (key.empty()) {
            if (!root_->is_terminal) {
                root_->is_terminal = true;
                ++size_;
                return true;
            }
            return false;
        }

        Node* curr = root_.get();
        std::string_view rem = key;

        while (true) {
            char first_ch = rem[0];
            auto it = curr->children.find(first_ch);

            if (it == curr->children.end()) {
                // No child with this character; create a new leaf branch
                curr->children[first_ch] = std::make_unique<Node>(std::string(rem), true);
                ++size_;
                return true;
            }

            Node* child = it->second.get();
            std::size_t cpl = common_prefix_length(child->edge, rem);

            if (cpl < child->edge.size()) {
                // Edge needs to be split
                std::string split_edge = child->edge.substr(0, cpl);
                std::string child_rem_edge = child->edge.substr(cpl);

                auto split_node = std::make_unique<Node>(split_edge, false);
                char old_child_lead = child_rem_edge[0];

                // Update child's incoming edge
                child->edge = child_rem_edge;

                // Transfer child to split_node
                split_node->children[old_child_lead] = std::move(it->second);

                if (cpl == rem.size()) {
                    // The inserted key ends exactly at the split point
                    split_node->is_terminal = true;
                } else {
                    // New branch for the rest of rem
                    std::string new_branch_edge = std::string(rem.substr(cpl));
                    char new_branch_lead = new_branch_edge[0];
                    split_node->children[new_branch_lead] =
                        std::make_unique<Node>(new_branch_edge, true);
                }

                curr->children[first_ch] = std::move(split_node);
                ++size_;
                return true;
            }

            // child->edge is a full prefix of rem
            if (cpl == rem.size()) {
                // Exact match of key with child node
                if (!child->is_terminal) {
                    child->is_terminal = true;
                    ++size_;
                    return true;
                }
                return false; // Key already present
            }

            // Advance down to child
            rem = rem.substr(cpl);
            curr = child;
        }
    }

    /**
     * @brief Erases a key from the trie.
     * Re-compresses nodes if a non-terminal node is left with a single child.
     * @return true if erased, false if the key was not found.
     */
    bool erase(std::string_view key) {
        if (key.empty()) {
            if (root_->is_terminal) {
                root_->is_terminal = false;
                --size_;
                return true;
            }
            return false;
        }

        bool removed = false;
        erase_helper(root_.get(), key, removed);
        if (removed) {
            --size_;
        }
        return removed;
    }

    // =========================================================================
    // Lookups & Autocompletion
    // =========================================================================

    /**
     * @brief Checks if the exact key exists in the trie.
     */
    [[nodiscard]] bool contains(std::string_view key) const noexcept {
        if (key.empty()) {
            return root_->is_terminal;
        }

        const Node* curr = root_.get();
        std::string_view rem = key;

        while (true) {
            char first_ch = rem[0];
            auto it = curr->children.find(first_ch);
            if (it == curr->children.end()) {
                return false;
            }

            const Node* child = it->second.get();
            std::size_t cpl = common_prefix_length(child->edge, rem);

            if (cpl < child->edge.size()) {
                return false;
            }

            if (cpl == rem.size()) {
                return child->is_terminal;
            }

            rem = rem.substr(cpl);
            curr = child;
        }
    }

    /**
     * @brief Checks if any stored key starts with the given prefix.
     */
    [[nodiscard]] bool starts_with(std::string_view prefix) const noexcept {
        if (prefix.empty()) {
            return size_ > 0;
        }

        const Node* curr = root_.get();
        std::string_view rem = prefix;

        while (true) {
            char first_ch = rem[0];
            auto it = curr->children.find(first_ch);
            if (it == curr->children.end()) {
                return false;
            }

            const Node* child = it->second.get();
            std::size_t cpl = common_prefix_length(child->edge, rem);

            if (cpl == rem.size()) {
                // Prefix completely consumed within this edge
                return true;
            }

            if (cpl < child->edge.size()) {
                // Mismatch before edge ends
                return false;
            }

            rem = rem.substr(cpl);
            curr = child;
        }
    }

    /**
     * @brief Finds the longest prefix of 'str' that is present as a key in the trie.
     */
    [[nodiscard]] std::string longest_common_prefix(std::string_view str) const {
        std::string longest;
        if (root_->is_terminal) {
            longest = "";
        }

        const Node* curr = root_.get();
        std::string current_prefix;
        std::string_view rem = str;

        while (!rem.empty()) {
            char first_ch = rem[0];
            auto it = curr->children.find(first_ch);
            if (it == curr->children.end()) {
                break;
            }

            const Node* child = it->second.get();
            std::size_t cpl = common_prefix_length(child->edge, rem);

            if (cpl < child->edge.size()) {
                // Edge matched partially; no full node reached
                break;
            }

            current_prefix += child->edge;
            if (child->is_terminal) {
                longest = current_prefix;
            }

            rem = rem.substr(cpl);
            curr = child;
        }

        return longest;
    }

    /**
     * @brief Returns all stored keys starting with 'prefix' in lexicographical order.
     * @param max_results Maximum number of results to return (0 = no limit).
     */
    [[nodiscard]] std::vector<std::string> autocomplete(
        std::string_view prefix, std::size_t max_results = 0) const {
        std::vector<std::string> results;

        if (prefix.empty()) {
            std::string current;
            if (root_->is_terminal) {
                results.push_back("");
            }
            collect_all(root_.get(), current, results, max_results);
            return results;
        }

        const Node* curr = root_.get();
        std::string_view rem = prefix;
        std::string accumulated;

        while (true) {
            char first_ch = rem[0];
            auto it = curr->children.find(first_ch);
            if (it == curr->children.end()) {
                return results; // No words match prefix
            }

            const Node* child = it->second.get();
            std::size_t cpl = common_prefix_length(child->edge, rem);

            if (cpl == rem.size()) {
                // Found node where prefix ends (could be partway through child->edge)
                accumulated += child->edge;
                if (child->is_terminal) {
                    results.push_back(accumulated);
                }
                collect_all(child, accumulated, results, max_results);
                return results;
            }

            if (cpl < child->edge.size()) {
                return results; // Mismatch
            }

            accumulated += child->edge;
            rem = rem.substr(cpl);
            curr = child;
        }
    }

    /**
     * @brief Returns all keys in lexicographical order.
     */
    [[nodiscard]] std::vector<std::string> words() const {
        return autocomplete("");
    }

    void swap(compressed_trie& other) noexcept {
        using std::swap;
        swap(root_, other.root_);
        swap(size_, other.size_);
    }

private:
    static void collect_all(const Node* node, std::string& current,
                            std::vector<std::string>& results, std::size_t max_results) {
        if (!node) return;

        for (const auto& [ch, child] : node->children) {
            if (max_results > 0 && results.size() >= max_results) {
                return;
            }

            std::size_t prev_len = current.size();
            current += child->edge;

            if (child->is_terminal) {
                results.push_back(current);
            }

            collect_all(child.get(), current, results, max_results);
            current.resize(prev_len);
        }
    }

    static void erase_helper(Node* curr, std::string_view rem, bool& removed) {
        if (rem.empty()) return;

        char first_ch = rem[0];
        auto it = curr->children.find(first_ch);
        if (it == curr->children.end()) {
            return;
        }

        Node* child = it->second.get();
        std::size_t cpl = common_prefix_length(child->edge, rem);

        if (cpl < child->edge.size()) {
            return; // Not found
        }

        if (cpl == rem.size()) {
            // Found the target node
            if (!child->is_terminal) {
                return; // Key wasn't marked terminal
            }
            child->is_terminal = false;
            removed = true;

            // If child has no children of its own, remove it from curr
            if (child->is_leaf()) {
                curr->children.erase(it);
            } else if (child->children.size() == 1) {
                // Compress child with its only child
                auto sole_child_it = child->children.begin();
                Node* sole_child = sole_child_it->second.get();
                child->edge += sole_child->edge;
                child->is_terminal = sole_child->is_terminal;
                auto moved_children = std::move(sole_child->children);
                child->children = std::move(moved_children);
            }
        } else {
            // Recurse deeper
            erase_helper(child, rem.substr(cpl), removed);

            // After recursion, check if child can be compressed
            if (removed && !child->is_terminal) {
                if (child->is_leaf()) {
                    curr->children.erase(it);
                } else if (child->children.size() == 1) {
                    auto sole_child_it = child->children.begin();
                    Node* sole_child = sole_child_it->second.get();
                    child->edge += sole_child->edge;
                    child->is_terminal = sole_child->is_terminal;
                    auto moved_children = std::move(sole_child->children);
                    child->children = std::move(moved_children);
                }
            }
        }

        // If curr itself (when not root) became eligible for compression with its single child
        // (Note: handled by curr's caller)
    }
};

inline void swap(compressed_trie& lhs, compressed_trie& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace stl
