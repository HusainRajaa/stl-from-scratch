#pragma once

#include <algorithm>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

namespace stl {

/**
 * @brief Binomial Heap priority queue implementation.
 *
 * Implements a priority queue backed by a collection of binomial trees.
 * By default, uses std::less<T>, meaning the maximum element is at the top,
 * identical to std::priority_queue.
 *
 * Supports O(log n) worst-case merge, insert, and extract-top.
 *
 * @tparam T Element type stored in the heap.
 * @tparam Compare Binary predicate providing strict weak ordering.
 */
template <typename T, typename Compare = std::less<T>>
class binomial_heap {
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
        node_type* parent{nullptr};
        node_type* child{nullptr};
        node_type* sibling{nullptr};

        template <typename... Args>
        explicit node_type(Args&&... args)
            : value(std::forward<Args>(args)...) {}
    };

    using handle = node_type*;

    // =========================================================================
    // Constructors, Destructor, Assignment
    // =========================================================================

    binomial_heap() = default;

    explicit binomial_heap(const Compare& comp)
        : comp_(comp) {}

    template <typename InputIt>
    binomial_heap(InputIt first, InputIt last, const Compare& comp = Compare())
        : comp_(comp) {
        for (; first != last; ++first) {
            push(*first);
        }
    }

    binomial_heap(std::initializer_list<T> init, const Compare& comp = Compare())
        : binomial_heap(init.begin(), init.end(), comp) {}

    ~binomial_heap() {
        clear();
    }

    binomial_heap(const binomial_heap& other)
        : comp_(other.comp_) {
        head_ = clone_tree_list(other.head_, nullptr);
        size_ = other.size_;
    }

    binomial_heap(binomial_heap&& other) noexcept
        : head_(std::exchange(other.head_, nullptr)),
          size_(std::exchange(other.size_, 0)),
          comp_(std::move(other.comp_)) {}

    binomial_heap& operator=(const binomial_heap& other) {
        if (this != &other) {
            binomial_heap tmp(other);
            swap(tmp);
        }
        return *this;
    }

    binomial_heap& operator=(binomial_heap&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = std::exchange(other.head_, nullptr);
            size_ = std::exchange(other.size_, 0);
            comp_ = std::move(other.comp_);
        }
        return *this;
    }

    // =========================================================================
    // Capacity & Status
    // =========================================================================

    [[nodiscard]] bool empty() const noexcept {
        return head_ == nullptr;
    }

    [[nodiscard]] size_type size() const noexcept {
        return size_;
    }

    void clear() noexcept {
        destroy_tree_list(head_);
        head_ = nullptr;
        size_ = 0;
    }

    // =========================================================================
    // Element Access
    // =========================================================================

    /**
     * @brief Returns a reference to the highest priority element in the heap.
     * Throws std::underflow_error if the heap is empty.
     */
    [[nodiscard]] const_reference top() const {
        if (empty()) {
            throw std::underflow_error("binomial_heap::top(): heap is empty");
        }
        const node_type* best = find_extreme_node();
        return best->value;
    }

    // =========================================================================
    // Modifiers
    // =========================================================================

    /**
     * @brief Inserts an element into the heap. O(log n) worst-case.
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
        node_type* new_node = new node_type(std::forward<Args>(args)...);
        binomial_heap single_heap(comp_);
        single_heap.head_ = new_node;
        single_heap.size_ = 1;

        merge(single_heap);
        return new_node;
    }

    /**
     * @brief Removes the top element from the heap. O(log n).
     */
    void pop() {
        if (empty()) {
            throw std::underflow_error("binomial_heap::pop(): heap is empty");
        }

        // Find root with extreme priority and its predecessor in the root list
        node_type* prev_best = nullptr;
        node_type* best = head_;
        node_type* prev = head_;
        node_type* curr = head_->sibling;

        while (curr != nullptr) {
            if (has_higher_priority(curr->value, best->value)) {
                best = curr;
                prev_best = prev;
            }
            prev = curr;
            curr = curr->sibling;
        }

        // Remove best from root list
        if (prev_best == nullptr) {
            head_ = best->sibling;
        } else {
            prev_best->sibling = best->sibling;
        }

        // Reverse best's child list to get increasing order of degrees
        node_type* child_head = nullptr;
        node_type* child_curr = best->child;
        size_type removed_children_count = 0;

        while (child_curr != nullptr) {
            node_type* next = child_curr->sibling;
            child_curr->parent = nullptr;
            child_curr->sibling = child_head;
            child_head = child_curr;
            child_curr = next;
            removed_children_count += (1ULL << child_head->degree);
        }

        delete best;
        --size_;

        // Merge children into this heap (children were already counted in size_)
        binomial_heap child_heap(comp_);
        child_heap.head_ = child_head;
        child_heap.size_ = 0;

        merge_internal(child_heap);
    }

    /**
     * @brief Merges other into *this. other becomes empty. O(log n).
     */
    void merge(binomial_heap& other) {
        if (this == &other || other.empty()) {
            return;
        }
        if (empty()) {
            head_ = other.head_;
            size_ = other.size_;
            other.head_ = nullptr;
            other.size_ = 0;
            return;
        }

        merge_internal(other);
    }

    /**
     * @brief Updates the key of a node with higher priority. O(log n).
     */
    void update_key(handle h, const T& new_value) {
        if (!h) return;
        if (has_higher_priority(h->value, new_value)) {
            throw std::invalid_argument("binomial_heap::update_key: new value has lower priority");
        }
        h->value = new_value;
        bubble_up(h);
    }

    /**
     * @brief Updates the key of a node with higher priority (rvalue).
     */
    void update_key(handle h, T&& new_value) {
        if (!h) return;
        if (has_higher_priority(h->value, new_value)) {
            throw std::invalid_argument("binomial_heap::update_key: new value has lower priority");
        }
        h->value = std::move(new_value);
        bubble_up(h);
    }

    void swap(binomial_heap& other) noexcept {
        using std::swap;
        swap(head_, other.head_);
        swap(size_, other.size_);
        swap(comp_, other.comp_);
    }

    [[nodiscard]] const Compare& value_comp() const noexcept {
        return comp_;
    }

private:
    node_type* head_{nullptr};
    size_type size_{0};
    Compare comp_{};

    /**
     * @brief Returns true if 'a' has strictly higher priority than 'b'.
     * Since default comp_ is std::less (max-heap), comp_(b, a) is true when a > b.
     */
    bool has_higher_priority(const T& a, const T& b) const {
        return comp_(b, a);
    }

    const node_type* find_extreme_node() const noexcept {
        const node_type* best = head_;
        const node_type* curr = head_->sibling;
        while (curr != nullptr) {
            if (has_higher_priority(curr->value, best->value)) {
                best = curr;
            }
            curr = curr->sibling;
        }
        return best;
    }

    void bubble_up(node_type* node) {
        node_type* curr = node;
        node_type* p = curr->parent;
        while (p != nullptr && has_higher_priority(curr->value, p->value)) {
            std::swap(curr->value, p->value);
            curr = p;
            p = curr->parent;
        }
    }

    /**
     * @brief Links tree with root y as child of tree with root z.
     * Precondition: z has higher or equal priority than y, both have same degree.
     */
    static void link_trees(node_type* y, node_type* z) {
        y->parent = z;
        y->sibling = z->child;
        z->child = y;
        ++z->degree;
    }

    /**
     * @brief Merges root lists of h1 and h2 sorted by degree.
     */
    static node_type* merge_root_lists(node_type* h1, node_type* h2) {
        if (!h1) return h2;
        if (!h2) return h1;

        node_type* head = nullptr;
        node_type** tail = &head;

        while (h1 && h2) {
            if (h1->degree <= h2->degree) {
                *tail = h1;
                h1 = h1->sibling;
            } else {
                *tail = h2;
                h2 = h2->sibling;
            }
            tail = &((*tail)->sibling);
        }

        *tail = h1 ? h1 : h2;
        return head;
    }

    void merge_internal(binomial_heap& other) {
        size_ += other.size_;
        head_ = merge_root_lists(head_, other.head_);
        other.head_ = nullptr;
        other.size_ = 0;

        if (!head_) return;

        node_type* prev = nullptr;
        node_type* curr = head_;
        node_type* next = curr->sibling;

        while (next != nullptr) {
            if ((curr->degree != next->degree) ||
                (next->sibling != nullptr && next->sibling->degree == curr->degree)) {
                // Cases 1 and 2: degrees differ, or three consecutive trees have same degree
                prev = curr;
                curr = next;
            } else if (!has_higher_priority(next->value, curr->value)) {
                // Case 3: curr has higher or equal priority -> next becomes child of curr
                curr->sibling = next->sibling;
                link_trees(next, curr);
            } else {
                // Case 4: next has higher priority -> curr becomes child of next
                if (prev == nullptr) {
                    head_ = next;
                } else {
                    prev->sibling = next;
                }
                link_trees(curr, next);
                curr = next;
            }
            next = curr->sibling;
        }
    }

    static void destroy_tree_list(node_type* root) noexcept {
        while (root != nullptr) {
            node_type* next = root->sibling;
            destroy_tree_list(root->child);
            delete root;
            root = next;
        }
    }

    static node_type* clone_tree_list(const node_type* src, node_type* parent) {
        if (!src) return nullptr;

        node_type* first = nullptr;
        node_type* prev = nullptr;

        for (const node_type* curr = src; curr != nullptr; curr = curr->sibling) {
            node_type* cloned = new node_type(curr->value);
            cloned->degree = curr->degree;
            cloned->parent = parent;
            cloned->child = clone_tree_list(curr->child, cloned);

            if (!first) {
                first = cloned;
            } else {
                prev->sibling = cloned;
            }
            prev = cloned;
        }

        return first;
    }
};

template <typename T, typename Compare>
void swap(binomial_heap<T, Compare>& lhs, binomial_heap<T, Compare>& rhs) noexcept {
    lhs.swap(rhs);
}

} // namespace stl
