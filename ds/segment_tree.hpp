//
// Created by conko on 23-10-12.
//

#ifndef INFLATE_SEGMENT_TREE_HPP
#define INFLATE_SEGMENT_TREE_HPP

#include <concepts>
#include <functional>
#include <memory>
#include <optional>

namespace inflate {

    template <
            class T,
            class Alloc = std::allocator<T>,
            class Compare = std::less<T>,
            class Plus = std::plus<T>,
            class OperationOperandType = T>

            requires std::copyable<T>
                    && std::same_as<T, typename Alloc::value_type>
                    && std::is_default_constructible_v<Compare>
                    && std::is_invocable_r_v<bool, Compare(const T&, const T&)>
                    && std::is_invocable_r_v<T, Plus(const T&, const T&)>
                    && std::copyable<T>
                    && std::copyable<OperationOperandType>
    class segment_tree {
    public:
        using value_type = T;
        using reference = value_type&;
        using size_type = std::size_t;
        using difference_type = std::ptrdiff_t;
        using allocator_type = Alloc;
        using pointer = std::allocator_traits<Alloc>::pointer;
        using const_pointer = std::allocator_traits<Alloc>::const_pointer;

        struct segment_tree_node {

        protected:
            struct segment_tree_node_tag {
                OperationOperandType val;
                std::function<bool(T, OperationOperandType)> operation;
            };

            std::optional<segment_tree_node_tag> tag;

        public:

            T max;
            T min;
            T sum;
            size_type begin_pos;
            size_type end_pos;

            [[nodiscard]] bool is_leaf() {
                return begin_pos == end_pos-1;
            }

            template<class Call> requires std::copyable<Call> && std::is_invocable_r_v<T, Call(T, OperationOperandType)>
            void set_tag(OperationOperandType opr, Call operation) {
                tag = segment_tree_node_tag {opr, operation};
            }

            void clear_tag() {
                tag.reset();
            }

            // construct leaf node
            segment_tree_node(const T& val, size_type pos):
                tag(),
                max(val),
                min(val),
                sum(val),
                begin_pos(pos),
                end_pos(pos) {}

        };


        segment_tree();
    };

} // inflate

#endif //INFLATE_SEGMENT_TREE_HPP
