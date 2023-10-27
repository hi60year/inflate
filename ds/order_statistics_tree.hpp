//
// Created by Administrator on 10/17/2023.
//

#ifndef INFLATE_ORDER_STATISTICS_TREE_HPP
#define INFLATE_ORDER_STATISTICS_TREE_HPP

#include <iostream>
#include <functional>
#include <memory>
#include <optional>
#include <numeric>
#include <vector>
#include <algorithm>
#include <concepts>

namespace inflate {

    enum class Color { RED, BLACK };

    template <typename T>
    struct rb_tree_node {
        T value;
        rb_tree_node<T>* parent;
        rb_tree_node<T>* left;
        rb_tree_node<T>* right;
        Color color;
        int size;

        explicit rb_tree_node(T val, Color col = Color::RED)
                : value(val), parent(nullptr), left(nullptr), right(nullptr), color(col), size(1) {}
    };

    template <std::copyable T>
    class order_statistics_tree {
    public:
        using value_type = T;

        order_statistics_tree() : root(nullptr) {}

        ~order_statistics_tree() {
            destroyTree(root);
        }

        void insert(const T& value) {
            auto* new_node = new rb_tree_node<T>(value);
            BSTInsert(new_node);
            fixViolation(new_node);
        }

        void remove(const T& value) {
            auto node = findNode(value);
            if (node) {
                BSTRemove(node);
                delete node;
            }
        }

        rb_tree_node<T>* find(const T& value) {
            return findNode(value);
        }

        T kthSmallest(int k) const {
            if (k <= 0 || k > getSize())
                throw std::out_of_range("Invalid k value!");

            return kthSmallestHelper(root, k);
        }

        [[nodiscard]] int getSize() const noexcept {
            if (root == nullptr)
                return 0;

            return root->size;
        }

    private:
        rb_tree_node<T>* root;

        rb_tree_node<T>* findNode(const T& value) const {
            rb_tree_node<T>* curr = root;
            while (curr != nullptr) {
                if (value < curr->value)
                    curr = curr->left;
                else if (value > curr->value)
                    curr = curr->right;
                else
                    return curr;
            }
            return nullptr;
        }

        rb_tree_node<T>* minimum(rb_tree_node<T>* node) const {
            while (node->left != nullptr)
                node = node->left;
            return node;
        }

        int calculateSize(rb_tree_node<T>* node) const {
            if (node == nullptr)
                return 0;

            return 1 + calculateSize(node->left) + calculateSize(node->right);
        }

        void fixViolation(rb_tree_node<T>* node) {
            rb_tree_node<T>* parent = nullptr;
            rb_tree_node<T>* grand_parent = nullptr;
            while (node != root && node->color == Color::RED && node->parent->color == Color::RED) {
                parent = node->parent;
                grand_parent = parent->parent;
                if (parent == grand_parent->left) {
                    rb_tree_node<T>* uncle = grand_parent->right;
                    if (uncle != nullptr && uncle->color == Color::RED) {
                        grand_parent->color = Color::RED;
                        parent->color = Color::BLACK;
                        uncle->color = Color::BLACK;
                        node = grand_parent;
                    } else {
                        if (node == parent->right) {
                            rotateLeft(parent);
                            node = parent;
                            parent = node->parent;
                        }
                        rotateRight(grand_parent);
                        std::swap(parent->color, grand_parent->color);
                        node = parent;
                    }
                } else {
                    rb_tree_node<T>* uncle = grand_parent->left;
                    if (uncle != nullptr && uncle->color == Color::RED) {
                        grand_parent->color = Color::RED;
                        parent->color = Color::BLACK;
                        uncle->color = Color::BLACK;
                        node = grand_parent;
                    } else {
                        if (node == parent->left) {
                            rotateRight(parent);
                            node = parent;
                            parent = node->parent;
                        }
                        rotateLeft(grand_parent);
                        std::swap(parent->color, grand_parent->color);
                        node = parent;
                    }
                }
            }
            root->color = Color::BLACK;
        }

        void rotateLeft(rb_tree_node<T>* node) {
            rb_tree_node<T>* temp = node->right;
            node->right = temp->left;
            if (temp->left != nullptr)
                temp->left->parent = node;
            temp->parent = node->parent;
            if (node->parent == nullptr)
                root = temp;
            else if (node == node->parent->left)
                node->parent->left = temp;
            else
                node->parent->right = temp;
            temp->left = node;
            node->parent = temp;

            // Update the size values
            temp->size = node->size;
            node->size = calculateSize(node->left) + calculateSize(node->right) + 1;
        }

        void rotateRight(rb_tree_node<T>* node) {
            rb_tree_node<T>* temp = node->left;
            node->left = temp->right;
            if (temp->right != nullptr)
                temp->right->parent = node;
            temp->parent = node->parent;
            if (node->parent == nullptr)
                root = temp;
            else if (node == node->parent->left)
                node->parent->left = temp;
            else
                node->parent->right = temp;
            temp->right = node;
            node->parent = temp;

            // Update the size values
            temp->size = node->size;
            node->size = calculateSize(node->left) + calculateSize(node->right) + 1;
        }

        void BSTInsert(rb_tree_node<T>* node) {
            rb_tree_node<T>* x = root;
            rb_tree_node<T>* y = nullptr;
            while (x != nullptr) {
                y = x;
                if (node->value < x->value) {
                    x->size++;
                    x = x->left;
                } else if (node->value > x->value) {
                    x->size++;
                    x = x->right;
                } else {
                    return; // Duplicate value, do nothing
                }
            }
            node->parent = y;
            if (y == nullptr)
                root = node;
            else if (node->value < y->value) {
                y->left = node;
                fixSizeIncrement(y->right);
            } else {
                y->right = node;
                fixSizeIncrement(y->left);
            }
        }

        void fixSizeIncrement(rb_tree_node<T>* node) {
            while (node != nullptr) {
                node->size++;
                node = node->parent;
            }
        }

        void BSTRemove(rb_tree_node<T>* node) {
            rb_tree_node<T>* y = node;
            rb_tree_node<T>* x = nullptr;
            Color y_original_color = y->color;
            if (node->left == nullptr) {
                x = node->right;
                transplant(node, node->right);
            } else if (node->right == nullptr) {
                x = node->left;
                transplant(node, node->left);
            } else {
                y = minimum(node->right);
                y_original_color = y->color;
                x = y->right;
                if (y->parent == node && x != nullptr) {
                    x->parent = y;
                    fixSizeDecrement(y->right);
                } else {
                    transplant(y, y->right);
                    y->right = node->right;
                    if (y->right != nullptr)
                        y->right->parent = y;
                }
                transplant(node, y);
                y->left = node->left;
                y->left->parent = y;
                y->color = node->color;
                fixSizeDecrement(y->left);
            }
            if (y_original_color == Color::BLACK)
                fixViolationRemove(x);
        }

        void fixViolationRemove(rb_tree_node<T>* node) {
            while (node != root && (node == nullptr || node->color == Color::BLACK)) {
                if (node == node->parent->left) {
                    rb_tree_node<T>* sibling = node->parent->right;
                    if (sibling->color == Color::RED) {
                        sibling->color = Color::BLACK;
                        node->parent->color = Color::RED;
                        rotateLeft(node->parent);
                        sibling = node->parent->right;
                    }
                    if ((sibling->left == nullptr || sibling->left->color == Color::BLACK) &&
                        (sibling->right == nullptr || sibling->right->color == Color::BLACK)) {
                        sibling->color = Color::RED;
                        node = node->parent;
                    } else {
                        if (sibling->right == nullptr || sibling->right->color == Color::BLACK) {
                            sibling->left->color = Color::BLACK;
                            sibling->color = Color::RED;
                            rotateRight(sibling);
                            sibling = node->parent->right;
                        }
                        sibling->color = node->parent->color;
                        node->parent->color = Color::BLACK;
                        sibling->right->color = Color::BLACK;
                        rotateLeft(node->parent);
                        node = root;
                    }
                } else {
                    rb_tree_node<T>* sibling = node->parent->left;
                    if (sibling->color == Color::RED) {
                        sibling->color = Color::BLACK;
                        node->parent->color = Color::RED;
                        rotateRight(node->parent);
                        sibling = node->parent->left;
                    }
                    if ((sibling->right == nullptr || sibling->right->color == Color::BLACK) &&
                        (sibling->left == nullptr || sibling->left->color == Color::BLACK)) {
                        sibling->color = Color::RED;
                        node = node->parent;
                    } else {
                        if (sibling->left == nullptr || sibling->left->color == Color::BLACK) {
                            sibling->right->color = Color::BLACK;
                            sibling->color = Color::RED;
                            rotateLeft(sibling);
                            sibling = node->parent->left;
                        }
                        sibling->color = node->parent->color;
                        node->parent->color = Color::BLACK;
                        sibling->left->color = Color::BLACK;
                        rotateRight(node->parent);
                        node = root;
                    }
                }
            }
            if (node != nullptr)
                node->color = Color::BLACK;
        }

        void transplant(rb_tree_node<T>* u, rb_tree_node<T>* v) {
            if (u->parent == nullptr)
                root = v;
            else if (u == u->parent->left)
                u->parent->left = v;
            else
                u->parent->right = v;

            if (v != nullptr)
                v->parent = u->parent;
        }

        T kthSmallestHelper(rb_tree_node<T>* node, int k) const {
            int leftSize = calculateSize(node->left) + 1;
            if (k == leftSize)
                return node->value;
            else if (k < leftSize)
                return kthSmallestHelper(node->left, k);
            else
                return kthSmallestHelper(node->right, k - leftSize);
        }

        void destroyTree(rb_tree_node<T>* node) {
            if (node == nullptr)
                return;

            destroyTree(node->left);
            destroyTree(node->right);
            delete node;
        }
    };

}  // namespace inflate

#endif //INFLATE_ORDER_STATISTICS_TREE_HPP
