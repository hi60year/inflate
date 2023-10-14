//
// Created by conko on 23-10-12.
//

#ifndef INFLATE_SEGMENT_TREE_HPP
#define INFLATE_SEGMENT_TREE_HPP

#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <numeric>

namespace inflate {

    template <class T, class OperationOperandType = T>
    struct segment_tree_node {

        struct segment_tree_node_tag {
            OperationOperandType val;
            std::function<T(const T&, const OperationOperandType&)> operation;
        };

        std::optional<segment_tree_node_tag> _tag;

        using size_type = std::size_t;

        T max;
        T min;
        T sum;
        size_type begin_pos;
        size_type end_pos;

        [[nodiscard]] constexpr bool is_leaf() const noexcept {
            return begin_pos == end_pos-1;
        }

        template<class Call> requires std::copyable<Call> && std::is_invocable_r_v<T, Call, const T&, const OperationOperandType&>
        void set_tag(OperationOperandType opr, Call operation) {
            _tag = segment_tree_node_tag {opr, operation};
        }

        void clear_tag() {
            _tag.reset();
        }

        // construct leaf node
        segment_tree_node(const T& val, size_type pos):
                _tag(),
                max(val),
                min(val),
                sum(val),
                begin_pos(pos),
                end_pos(pos + 1) {}

        segment_tree_node(const T& _max, const T& _min, const T& _sum, size_type _begin_pos, size_type _end_pos) :
            _tag(),
            min(_min),
            max(_max),
            sum(_sum),
            begin_pos(_begin_pos),
            end_pos(_end_pos) {}



    };


    template <
            class T,
            class Compare = std::less<T>,
            class Plus = std::plus<T>,
            class OperationOperandType = T,
            class Alloc = std::allocator<segment_tree_node<T, OperationOperandType>>
                    >
    concept linear_segment_tree_requirement = std::copyable<T>
                                              && std::same_as<segment_tree_node<T, OperationOperandType>, typename Alloc::value_type>
                                              && std::is_default_constructible_v<Compare>
                                              && std::is_default_constructible_v<Plus>
                                              && std::is_invocable_r_v<bool, Compare, const T&, const T&>
                                              && std::is_invocable_r_v<T, Plus, const T&, const T&>
                                              && std::copyable<T>
                                              && std::copyable<OperationOperandType>
                                              && requires(Alloc allocator, std::size_t size, std::add_pointer_t<T> p, segment_tree_node<T, OperationOperandType>* p2) {
                                                  {allocator.allocate(size)} -> std::convertible_to<segment_tree_node<T, OperationOperandType>*>;
                                                  {allocator.deallocate(p2, size)};
                                              };

    template <
            class T,
            class Compare = std::less<T>,
            class Plus = std::plus<T>,
            class OperationOperandType = T,
            class Alloc = std::allocator<segment_tree_node<T, OperationOperandType>>
                    >
            requires linear_segment_tree_requirement<T, Compare, Plus, OperationOperandType, Alloc>

    class linear_segment_tree {
    public:
        using value_type = T;
        using reference = value_type&;
        using size_type = std::size_t;
        using allocator_type = Alloc;
        using node_type = segment_tree_node<T, OperationOperandType>;
    protected:
        Alloc allocator;
        size_type _size;
        node_type* root;

        template<std::input_iterator Iter>
        Iter buildTree(size_type pos, size_type l, size_type r, Iter begin, Iter end);

        static constexpr node_type generate_parent(const node_type& l_child, const node_type& r_child, Compare compare = Compare(), Plus plus = Plus()) noexcept {
            return node_type (
                    std::invoke(compare, l_child.max, r_child.max) ? r_child.max : l_child.max,
                    std::invoke(compare, l_child.min, r_child.min) ? l_child.min : r_child.min,
                    std::invoke(plus, l_child.sum, r_child.sum),
                    l_child.begin_pos,
                    r_child.end_pos
            );
        }

    public:


        [[nodiscard]] constexpr allocator_type get_allocator() const noexcept {
            return allocator;
        }

        [[nodiscard]] constexpr size_type size() const noexcept {
            return _size;
        }

        [[nodiscard]] constexpr size_type allocation_size() const noexcept {
            return _size*4;
        }

        [[nodiscard]] constexpr const node_type& root_node () const noexcept {
            return *root;
        }

        template<std::input_iterator Iter>
        constexpr linear_segment_tree(Iter begin, Iter end, const Alloc& alloc = Alloc()):
        _size(std::distance(begin, end)), allocator(alloc){
            root = allocator.allocate(allocation_size());
            buildTree(1, 0, _size, begin, end);
        };

        constexpr linear_segment_tree(const linear_segment_tree& other):
            _size(other._size),
            allocator(std::allocator_traits<allocator_type>::select_on_container_copy_construction(
                    other.get_allocator()
                    )) {
            root = allocator.allocate(allocation_size());
            std::uninitialized_copy_n(other.root, other.allocation_size(), root);
        }

        constexpr ~linear_segment_tree() {
            if (this -> _size != 0) {
                allocator.deallocate(root, allocation_size());
            }
        }

    private:
        template<class Op>
        void _apply_op(node_type& node, const Op& operation, const OperationOperandType& val) {

            for (size_type i = node.begin_pos; i != node.end_pos; i++) {
                node.sum = std::invoke(operation, node.sum, val);
            }

            node.max = std::invoke(operation, node.max, val);
            node.min = std::invoke(operation, node.min, val);
        }
    public:

        void push_down_tag(size_type pos) {

            auto& node = root[pos - 1];

            if (not node._tag.has_value()) {
                return;
            }

            const auto& tag = node._tag.value();

            if (not node.is_leaf()) {
                push_down_tag(pos * 2);
                push_down_tag(pos * 2 + 1);
                _apply_op(root[pos * 2 - 1], tag.operation, tag.val);
                _apply_op(root[pos * 2], tag.operation, tag.val);
                root[pos * 2 - 1]._tag = tag;
                root[pos * 2]._tag = tag;
            }

            node._tag.reset();
        }

    private:

        template<class Op>
        void _update(size_type pos, size_type begin, size_type end, const Op& operation, const OperationOperandType& val) {
            push_down_tag(pos);
            auto& node = root[pos - 1];
            if (node.begin_pos < begin) {
                _update(pos * 2 + 1, begin, end, operation, val);
            }
            if (node.end_pos > end) {
                _update(pos * 2, begin, end, operation, val);
            } else if (node.begin_pos >= begin) {
                _apply_op(node, operation, val);
                node.set_tag(val, operation);
                return;
            }

            node = generate_parent(root[pos * 2 - 1], root[pos * 2]);

        }

    public:
        template<class Op>
        void update(size_type begin, size_type end, const Op& operation, const OperationOperandType& val) {
            _update(1, begin, end, operation, val);
        }
    };

    template<class T, class Compare, class Plus, class OperationOperandType, class Alloc>
    requires linear_segment_tree_requirement<T, Compare, Plus, OperationOperandType, Alloc>
    template<std::input_iterator Iter>
    Iter linear_segment_tree<T, Compare, Plus, OperationOperandType, Alloc>
            ::buildTree(size_type pos, size_type l, size_type r, Iter begin, Iter end) {
        if (l == r - 1) {
            std::construct_at(root + pos - 1, *begin++, l);
            return begin;
        } else {
            size_type mid = std::midpoint(l, r);
            begin = buildTree(pos * 2, l, mid, begin, end);
            begin = buildTree(pos * 2 + 1, mid, r, begin, end);

            std::construct_at(root + pos - 1, generate_parent(root[pos * 2 - 1], root[pos * 2]));

            return begin;
        }
    }


} // inflate

#endif //INFLATE_SEGMENT_TREE_HPP
