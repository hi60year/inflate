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


    template <
            class T,
            class Alloc = std::allocator<T>,
            class Compare = std::less<T>,
            class Plus = std::plus<T>,
            class OperationOperandType = T>
    concept linear_segment_tree_requirement = std::copyable<T>
                                              && std::same_as<T, typename Alloc::value_type>
                                              && std::is_default_constructible_v<Compare>
                                              && std::is_default_constructible_v<Plus>
                                              && std::is_invocable_r_v<bool, Compare, const T&, const T&>
                                              && std::is_invocable_r_v<T, Plus, const T&, const T&>
                                              && std::copyable<T>
                                              && std::copyable<OperationOperandType>
                                              && requires(Alloc allocator, std::size_t size, T* p) {
                                                  {allocator.allocate(size)} -> std::convertible_to<T*>;
                                                  {allocator.deallocate(p, size)};
                                              };

    template <
        class T,
        class Alloc = std::allocator<T>,
        class Compare = std::less<T>,
        class Plus = std::plus<T>,
        class OperationOperandType = T>
            requires linear_segment_tree_requirement<T, Alloc, Compare, Plus, OperationOperandType>

    class linear_segment_tree {
    public:
        using value_type = T;
        using reference = value_type&;
        using size_type = std::size_t;
        using allocator_type = Alloc;
        struct segment_tree_node;
    protected:
        Alloc allocator;
        size_type _size;
        std::add_pointer_t<segment_tree_node> root;

        template<std::input_iterator Iter>
        Iter buildTree(size_type pos, size_type l, size_type r, Iter begin, Iter end);

    public:
        struct segment_tree_node {

        protected:
            struct segment_tree_node_tag {
                OperationOperandType val;
                std::function<bool(T, OperationOperandType)> operation;
            };

            std::optional<segment_tree_node_tag> _tag;

        public:

            T max;
            T min;
            T sum;
            size_type begin_pos;
            size_type end_pos;

            [[nodiscard]] constexpr bool is_leaf() noexcept {
                return begin_pos == end_pos-1;
            }

            template<class Call> requires std::copyable<Call> && std::is_invocable_r_v<T, Call(T, OperationOperandType)>
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

            // TODO: complete construct parent node.
        };

        [[nodiscard]] constexpr allocator_type get_allocator() noexcept {
            return allocator;
        }

        [[nodiscard]] constexpr size_type size() noexcept {
            return _size;
        }

        [[nodiscard]] constexpr size_type allocation_size() noexcept {
            return _size*4;
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
    };

    template<class T, class Alloc, class Compare, class Plus, class OperationOperandType>
    requires linear_segment_tree_requirement<T, Alloc, Compare, Plus, OperationOperandType>
    template<std::input_iterator Iter>
    Iter linear_segment_tree<T, Alloc, Compare, Plus, OperationOperandType>
            ::buildTree(size_type pos, size_type l, size_type r, Iter begin, Iter end) {
        if (l == r - 1) {
            std::construct_at(root + pos - 1, *begin++, l);
            return begin;
        } else {
            size_type mid = std::midpoint(l, r);
            begin = buildTree(pos * 2, l, mid, begin, end);
            begin = buildTree(pos * 2 + 1, mid, r, begin, end);

            // TODO: complete tree build process.
            // std::construct_at()
            return begin;
        }
    }


} // inflate

#endif //INFLATE_SEGMENT_TREE_HPP
