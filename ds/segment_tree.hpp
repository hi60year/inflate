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
            std::reference_wrapper<const std::function<T(const T&, const OperationOperandType&, size_t begin_pos, size_t end_pos)>> operation;
        };

        std::optional<segment_tree_node_tag> _tag;

        using size_type = std::size_t;

        T sum;
        size_type begin_pos;
        size_type end_pos;

        [[nodiscard]] constexpr bool is_leaf() const noexcept {
            return begin_pos == end_pos-1;
        }

        template<class Call> requires std::is_invocable_r_v<T, Call, const T&, const OperationOperandType&, size_t, size_t>
        void set_tag(OperationOperandType opr, const Call& operation) {
            _tag = segment_tree_node_tag {opr, std::cref(operation)};
        }

        void clear_tag() {
            _tag.reset();
        }

        // construct leaf node
        segment_tree_node(const T& val, size_type pos):
                _tag(),
                sum(val),
                begin_pos(pos),
                end_pos(pos + 1) {}

        segment_tree_node(const T& _sum, size_type _begin_pos, size_type _end_pos) :
            _tag(),
            sum(_sum),
            begin_pos(_begin_pos),
            end_pos(_end_pos) {}
    };


    template <
            class T,
            class Plus = std::plus<T>,
            class OperationOperandType = T,
            class Alloc = std::allocator<segment_tree_node<T, OperationOperandType>>
                    >
    concept linear_segment_tree_requirement = std::copyable<T>
                                              && std::same_as<segment_tree_node<T, OperationOperandType>, typename Alloc::value_type>
                                              && std::is_default_constructible_v<Plus>
                                              && std::is_invocable_r_v<T, Plus, const T&, const T&>
                                              && std::copyable<T>
                                              && std::copyable<OperationOperandType>
                                              && requires(Alloc allocator, std::size_t size, std::add_pointer_t<T> p, segment_tree_node<T, OperationOperandType>* p2) {
                                                  {allocator.allocate(size)} -> std::convertible_to<segment_tree_node<T, OperationOperandType>*>;
                                                  {allocator.deallocate(p2, size)};
                                              };

    template <
            class T,
            class Plus = std::plus<T>,
            class OperationOperandType = T,
            class Alloc = std::allocator<segment_tree_node<T, OperationOperandType>>
                    >
            requires linear_segment_tree_requirement<T, Plus, OperationOperandType, Alloc>

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
        std::vector<std::function<T(const T&, const OperationOperandType&, size_t begin_pos, size_t end_pos)>> operations;

        template<std::input_iterator Iter>
        Iter buildTree(size_type pos, size_type l, size_type r, Iter begin, Iter end);

        static constexpr node_type generate_parent(const node_type& l_child, const node_type& r_child, Plus plus = Plus()) noexcept {
            return node_type (
                    std::invoke(plus, l_child.sum, r_child.sum),
                    l_child.begin_pos,
                    r_child.end_pos
            );
        }

        void destroy_tree(size_type pos = 1) noexcept {
            auto& node = root[pos - 1];
            if (not node.is_leaf()) {
                destroy_tree(pos * 2);
                destroy_tree(pos * 2 + 1);
            }
            std::destroy_at(root + pos - 1);
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

        constexpr ~linear_segment_tree() noexcept {
            destroy_tree();
            if (this -> _size != 0) {
                allocator.deallocate(root, allocation_size());
            }
        }

    private:
        template<class Op>
        void _apply_op(node_type& node, const Op& operation, const OperationOperandType& val) {
            node.sum = std::invoke(operation, node.sum, val, node.begin_pos, node.end_pos);
        }
    public:

        template<class Call> requires std::copyable<Call> && std::is_invocable_r_v<T, Call, const T&, const OperationOperandType&, size_t, size_t>
        void add_operation(Call operation) {
            operations.push_back(operation);
        }

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

        void _update(size_type pos, size_type begin, size_type end, size_type operation_num, const OperationOperandType& val) {
            push_down_tag(pos);
            auto& node = root[pos - 1];
            const auto& operation = operations[operation_num];

            if (node.begin_pos >= begin && node.end_pos <= end) {
                _apply_op(node, operation, val);
                node.set_tag(val, operation);
                return;
            } else if (size_type mid = std::midpoint(node.begin_pos, node.end_pos); mid <= begin) {
                _update(pos * 2 + 1, begin, end, operation_num, val);
            } else if (mid >= end) {
                _update(pos * 2, begin, end, operation_num, val);
            } else {
                _update(pos * 2, begin, end, operation_num, val);
                _update(pos * 2 + 1, begin, end, operation_num, val);
            }

            node = generate_parent(root[pos * 2 - 1], root[pos * 2]);

        }

        T _query(size_type pos, size_type begin_pos, size_type end_pos, const Plus& plus = Plus()) {
            push_down_tag(pos);
            auto& node = root[pos - 1];

            if (node.begin_pos >= begin_pos && node.end_pos <= end_pos) {
                return node.sum;
            } else if (size_type mid = std::midpoint(node.begin_pos, node.end_pos); mid <= begin_pos) {
                return _query(pos * 2 + 1, begin_pos, end_pos, plus);
            } else if (mid >= end_pos) {
                return _query(pos * 2, begin_pos, end_pos, plus);
            } else {
                return std::invoke(plus,
                                   _query(pos * 2, begin_pos, end_pos, plus),
                                   _query(pos * 2 + 1, begin_pos, end_pos, plus));
            }
        }

    public:
        void update(size_type begin, size_type end, int operation_num, const OperationOperandType& val) {
            _update(1, begin, end, operation_num, val);
        }

        T query(size_type begin_pos, size_type end_pos, const Plus& plus = Plus()) {
            return _query(1, begin_pos, end_pos, plus);
        }
    };

    template<class T, class Plus, class OperationOperandType, class Alloc>
    requires linear_segment_tree_requirement<T, Plus, OperationOperandType, Alloc>
    template<std::input_iterator Iter>
    Iter linear_segment_tree<T, Plus, OperationOperandType, Alloc>
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
