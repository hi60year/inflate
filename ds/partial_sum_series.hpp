//
// Created by Administrator on 10/15/2023.
//

#ifndef INFLATE_PARTIAL_SUM_SERIES_HPP
#define INFLATE_PARTIAL_SUM_SERIES_HPP

#include <algorithm>
#include <vector>
#include <functional>
#include <concepts>
#include <iterator>
#include <numeric>

namespace inflate {
    template<class T, class Plus = std::plus<T>, class Minus = std::minus<T>>
    class partial_sum_series {
    protected:
        Plus plus;
        Minus minus;
    public:
        std::vector<T> underlying_container;
        using size_type = std::size_t;

        template<std::input_iterator Iter>
        explicit partial_sum_series(Iter begin, Iter end, const Plus& _plus = Plus(), const Minus& _minus = Minus())
            : plus(_plus), minus(_minus) {
            if constexpr (std::random_access_iterator<Iter>) {
                underlying_container.reserve(std::distance(begin, end));
            }
            std::partial_sum(begin, end, std::back_inserter(underlying_container), plus);
        }

        constexpr decltype(auto) begin() noexcept {
            return underlying_container.begin();
        }

        constexpr decltype(auto) end() noexcept {
            return underlying_container.end();
        }

        constexpr decltype(auto) cbegin() const noexcept {
            return underlying_container.cbegin();
        }

        constexpr decltype(auto) cend() const noexcept {
            return underlying_container.cend();
        }

        constexpr decltype(auto) rbegin() noexcept {
            return underlying_container.rbegin();
        }

        constexpr decltype(auto) rend() noexcept {
            return underlying_container.rend();
        }

        constexpr decltype(auto) crbegin() const noexcept {
            return underlying_container.crbegin();
        }

        constexpr decltype(auto) crend() const noexcept {
            return underlying_container.crend();
        }


        constexpr T query(decltype(underlying_container)::const_iterator begin,
                          decltype(underlying_container)::const_iterator end) const {
            return std::invoke(minus, *end, *minus);
        }

        constexpr T query_n(decltype(underlying_container)::const_iterator begin,
                            size_type n) const {
            return std::invoke(minus, begin[n], *begin);
        }
    };
}

#endif //INFLATE_PARTIAL_SUM_SERIES_HPP
