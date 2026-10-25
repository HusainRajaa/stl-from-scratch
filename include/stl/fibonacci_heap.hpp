#pragma once

#include <algorithm>
#include <cmath>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace stl {

/**
 * @brief Fibonacci Heap priority queue implementation.
 *
 * Implements a priority queue with amortized O(1) push, merge, and decrease_key / update_key,
 * and amortized O(log n) pop and erase.
 *
 * By default, uses std::less<T>, matching std::priority_queue semantics (max-heap).
 *
 * @tparam T Element type stored in the heap.
 * @tparam Compare Binary predicate defining strict weak ordering.
 */
template <typename T, typename Compare = std::less<T>>
class fibonacci_heap {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using value_compare = Compare;

    struct node_type {
        T value;
        size_type degree{0};
        bool marked{false};
        node_type* parent{nullptr};
        node_type* child{nullptr};
        node_type* left{this};
        node_type* right{this};

        template <typename... Args>
        explicit node_type(Args&&... args)
            : value(std::forward<Args>(args)...), left(this), right(this) {}
    };

    using handle = node_type*;

    // =========================================================================
    // Constructors, Destructor, Assignment
    // =========================================================================

    fibonacci_heap() = default;

    explicit fibonacci_heap(const Compare& comp)
        : comp_(comp) {}

    template <typename InputIt>
    fibonacci_heap(InputIt first, InputIt last, const Compare& comp = Compare())
        : comp_(comp) {
        for (; first != last; ++first) {
            push(*first);
        }
    }

    fibonacci_heap(std::initializer_list<T> init, const Compare& comp = Compare())
        : fibonacci_heap(init.begin(), init.end(), comp) {}

    ~fibonacci_heap() {
        clear();
    }

    fibonacci_heap(const fibonacci_heap& other)
        : comp_(other.comp_) {
        clone_from(other);
    }

    fibonacci_heap(fibonacci_heap&& other) noexcept
        : extreme_(std::exchange(other.extreme_, nullptr)),
          size_(std::exchange(other.size_, 0)),
          comp_(std::move(other.comp_)) {}

    fibonacci_heap& operator=(const fibonacci_heap& other) {
        if (this != &other) {
            fibonacci_heap tmp(other);
            swap(tmp);
        }
        return *this;
    }

    fibonacci_heap& operator=(fibonacci_heap&& other) noexcept {
        if (this != &other) {
            clear();
            extreme_ = std::exchange(other.extreme_, nullptr);
            size_ = std::exchange(other.size_, 0);
            comp_ = std::move(other.comp_);
        }
        return *this;
    }

    // =========================================================================
    // Capacity & Status
    // =========================================================================

    [[nodiscard]] bool empty() const noexcept {
        return extreme_ == nullptr;
    }

    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }

    void clear() noexcept {
        if (extreme_ != nullptr) {
            destroy_circular_list(extreme_);
            extreme_ = nullptr;
            size_ = 0;
        }
    }

    // =========================================================================
    // Element Access
    // =========================================================================

    /**
     * @brief Access highest priority element. O(1) worst-case.
     */
    [[nodiscard]] const_reference top() const {
        if (empty()) {
            throw std::underflow_error("fibonacci_heap::top(): heap is empty");
        }
        return extreme_->value;
    }

    // =========================================================================
    // Modifiers
    // =========================================================================

    /**
     * @brief Inserts an element. Amortized O(1).
     * Returns a handle to the newly inserted node.
     */
    handle push(const T& value) {
        return emplace(value);
    }

    handle push(T&& value) {
        return emplace(std::move(value));
    }

    template <typename... Args>
    handle emplace(Args&&... args) {
        node_type* node = new node_type(std::forward<Args>(args)...);
        if (extreme_ == nullptr) {
            extreme_ = node;
        } else {
            insert_into_circular_list(extreme_, node);
            if (has_higher_priority(node->value, extreme_->value)) {
                extreme_ = node;
            }
        }
        ++size_;
        return node;
    }

    /**
     * @brief Removes the highest priority element. Amortized O(log n).
     */
    void pop() {
        if (empty()) {
            throw std::underflow_error("fibonacci_heap::pop(): heap is empty");
        }

        node_type* z = extreme_;

        // Add all children of z to the root list
        if (z->child != nullptr) {
            node_type* child = z->child;
            node_type* curr = child;
            do {
                curr->parent = nullptr;
                curr = curr->right;
            } while (curr != child);

            // Splice children into root list
            splice_into_root_list(child);
            z->child = nullptr;
        }

        // Remove z from the root list
        if (z->right == z) {
            extreme_ = nullptr;
        } else {
            extreme_ = z->right;
            remove_from_circular_list(z);
            consolidate();
        }

        delete z;
        --size_;
    }

    /**
     * @brief Merges other into *this. O(1) worst-case. other becomes empty.
     */
    void merge(fibonacci_heap& other) {
        if (this == &other || other.empty()) {
            return;
        }
        if (empty()) {
            extreme_ = other.extreme_;
            size_ = other.size_;
            other.extreme_ = nullptr;
            other.size_ = 0;
            return;
        }

        // Concatenate root lists
        splice_into_root_list(other.extreme_);

        if (has_higher_priority(other.extreme_->value, extreme_->value)) {
            extreme_ = other.extreme_;
        }

        size_ += other.size_;
        other.extreme_ = nullptr;
        other.size_ = 0;
    }

    /**
     * @brief Increases priority of element pointed by handle. Amortized O(1).
     */
    void update_key(handle h, const T& new_value) {
        if (!h) return;
        if (has_higher_priority(h->value, new_value)) {
            throw std::invalid_argument("fibonacci_heap::update_key: new value has lower priority");
        }
        h->value = new_value;
        node_type* p = h->parent;
        if (p != nullptr && has_higher_priority(h->value, p->value)) {
            cut(h, p);
            cascading_cut(p);
        }
        if (has_higher_priority(h->value, extreme_->value)) {
            extreme_ = h;
        }
    }

    void update_key(handle h, T&& new_value) {
        if (!h) return;
        if (has_higher_priority(h->value, new_value)) {
            throw std::invalid_argument("fibonacci_heap::update_key: new value has lower priority");
        }
        h->value = std::move(new_value);
        node_type* p = h->parent;
        if (p != nullptr && has_higher_priority(h->value, p->value)) {
            cut(h, p);
            cascading_cut(p);
        }
        if (has_higher_priority(h->value, extreme_->value)) {
            extreme_ = h;
        }
    }

    /**
     * @brief Erases element pointed by handle. Amortized O(log n).
     */
    void erase(handle h) {
        if (!h) return;
        if (h == extreme_) {
            pop();
            return;
        }

        node_type* p = h->parent;
        if (p != nullptr) {
            cut(h, p);
            cascading_cut(p);
        }

        // Add children of h to root list
        if (h->child != nullptr) {
            node_type* child = h->child;
            node_type* curr = child;
            do {
                curr->parent = nullptr;
                curr = curr->right;
            } while (curr != child);
            splice_into_root_list(child);
            h->child = nullptr;
        }

        remove_from_circular_list(h);
        delete h;
        --size_;
    }

    void swap(fibonacci_heap& other) noexcept {
        using std::swap;
        swap(extreme_, other.extreme_);
        swap(size_, other.size_);
        swap(comp_, other.comp_);
    }

    [[nodiscard]] const Compare& value_comp() const noexcept {
        return comp_;
    }

private:
    node_type* extreme_{nullptr};
    size_type size_{0};
    Compare comp_{};

    bool has_higher_priority(const T& a, const T& b) const {
        return comp_(b, a);
    }

    static void insert_into_circular_list(node_type* pos, node_type* node) {
        node->left = pos;
        node->right = pos->right;
        pos->right->left = node;
        pos->right = node;
    }

    static void remove_from_circular_list(node_type* node) {
        node->left->right = node->right;
        node->right->left = node->left;
        node->left = node;
        node->right = node;
    }

    void splice_into_root_list(node_type* other_root) {
        node_type* l1 = extreme_;
        node_type* r1 = extreme_->right;
        node_type* l2 = other_root->left;
        node_type* r2 = other_root;

        l1->right = r2;
        r2->left = l1;
        l2->right = r1;
        r1->left = l2;
    }

    void link_trees(node_type* y, node_type* x) {
        remove_from_circular_list(y);
        y->parent = x;
        y->marked = false;

        if (x->child == nullptr) {
            x->child = y;
            y->left = y;
            y->right = y;
        } else {
            insert_into_circular_list(x->child, y);
        }
        ++x->degree;
    }

    void cut(node_type* x, node_type* y) {
        // Remove x from child list of y
        if (x->right == x) {
            y->child = nullptr;
        } else {
            if (y->child == x) {
                y->child = x->right;
            }
            remove_from_circular_list(x);
        }
        --y->degree;

        // Add x to root list
        insert_into_circular_list(extreme_, x);
        x->parent = nullptr;
        x->marked = false;
    }

    void cascading_cut(node_type* y) {
        node_type* z = y->parent;
        if (z != nullptr) {
            if (!y->marked) {
                y->marked = true;
            } else {
                cut(y, z);
                cascading_cut(z);
            }
        }
    }

    void consolidate() {
        // Degree bound: ~1.4404 * log2(n) + 2
        constexpr size_type MAX_DEGREE = 128;
        std::vector<node_type*> table(MAX_DEGREE, nullptr);

        // Collect all roots currently in root list
        std::vector<node_type*> roots;
        roots.reserve(MAX_DEGREE);

        node_type* curr = extreme_;
        if (curr != nullptr) {
            do {
                roots.push_back(curr);
                curr = curr->right;
            } while (curr != extreme_);
        }

        for (node_type* x : roots) {
            size_type d = x->degree;
            while (d < MAX_DEGREE && table[d] != nullptr) {
                node_type* y = table[d];
                if (has_higher_priority(y->value, x->value)) {
                    std::swap(x, y);
                }
                link_trees(y, x);
                table[d] = nullptr;
                ++d;
            }
            if (d < MAX_DEGREE) {
                table[d] = x;
            }
        }

        // Reconstruct root list from table
        extreme_ = nullptr;
        for (node_type* node : table) {
            if (node != nullptr) {
                node->left = node;
                node->right = node;
                node->parent = nullptr;

                if (extreme_ == nullptr) {
                    extreme_ = node;
                } else {
                    insert_into_circular_list(extreme_, node);
                    if (has_higher_priority(node->value, extreme_->value)) {
                        extreme_ = node;
                    }
                }
            }
        }
    }

    static void destroy_circular_list(node_type* start) noexcept {
        if (!start) return;

        node_type* curr = start;
        do {
            node_type* next = curr->right;
            if (curr->child != nullptr) {
                destroy_circular_list(curr->child);
            }
            delete curr;
            curr = next;
        } while (curr != start);
    }

    void clone_from(const fibonacci_heap& other) {
        if (other.empty()) {
            extreme_ = nullptr;
            size_ = 0;
            return;
        }

        size_ = other.size_;
        extreme_ = clone_circular_list(other.extreme_, nullptr);

        // Find the extreme node in the cloned root list matching other.extreme_->value
        node_type* curr = extreme_;
        node_type* best = extreme_;
        do {
            if (has_higher_priority(curr->value, best->value)) {
                best = curr;
            }
            curr = curr->right;
        } while (curr != extreme_);

        extreme_ = best;
    }

    static node_type* clone_circular_list(const node_type* src_start, node_type* parent) {
        if (!src_start) return nullptr;

        node_type* first_cloned = nullptr;
        node_type* prev_cloned = nullptr;
        const node_type* curr = src_start;

        do {
            node_type* cloned = new node_type(curr->value);
            cloned->degree = curr->degree;
            cloned->marked = curr->marked;
            cloned->parent = parent;

            if (curr->child != nullptr) {
                cloned->child = clone_circular_list(curr->child, cloned);
            }

            if (!first_cloned) {
                first_cloned = cloned;
                cloned->left = cloned;
                cloned->right = cloned;
            } else {
                insert_into_circular_list(first_cloned, cloned);
            }
            prev_cloned = cloned;
            curr = curr->right;
        } while (curr != src_start);

        return first_cloned;
    }
};

template <typename T, typename Compare>
void swap(fibonacci_heap<T, Compare>& lhs, fibonacci_heap<T, Compare>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace stl
