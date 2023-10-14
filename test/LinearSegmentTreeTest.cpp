//
// Created by Administrator on 10/13/2023.
//

#include "../ds/segment_tree.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <ranges>
#include <queue>

class LinearSegmentTreeTestSuite : public inflate::linear_segment_tree<int>, public ::testing::Test{

};

TEST(LinearSegmentTreeTestSuite, ConstructionFromIteratorRangeTest) {
    std::vector a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    inflate::linear_segment_tree<int> tree(a.begin(), a.end());
    ASSERT_EQ(tree.size(), a.size());
    ASSERT_EQ(tree.allocation_size(), a.size()*4);
    ASSERT_EQ(tree.root_node().sum, 55);
    auto node = reinterpret_cast<const inflate::segment_tree_node<int, int>*>(& (tree.root_node()));
    ASSERT_EQ(node[1].sum, 15);
    ASSERT_EQ(node[2].sum, 40);
    ASSERT_EQ(node[3].sum, 3);
    ASSERT_EQ(node[4].sum, 12);
    std::queue<int> q;
    q.push(1);
    while (!q.empty()) {
        const auto& n = q.front();
        if (!node[n-1].is_leaf()) {
            ASSERT_EQ(node[n - 1].sum, node[2 * n - 1].sum + node[2 * n].sum);
            ASSERT_EQ(node[n - 1].max, std::max(node[2 * n - 1].max, node[2 * n].max));
            ASSERT_EQ(node[n - 1].min, std::min(node[2 * n - 1].min, node[2 * n].min));
            q.push(2*n);
            q.push(2*n + 1);
        }
        q.pop();
    }

}

TEST(LinearSegmentTreeTestSuite, UpdateTest) {
    std::vector a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    inflate::linear_segment_tree<int> tree(a.begin(), a.end());
    tree.update(0, 10, [](int a, int b) {return a + b;}, 3);
    auto root = reinterpret_cast<const inflate::segment_tree_node<int, int>*>(& (tree.root_node()));
    ASSERT_TRUE(root->_tag);
    ASSERT_EQ(root->sum, 85);
    tree.update(4, 6, [](int a, int b) {return a+b;}, 2);
    ASSERT_FALSE(root[1]._tag);
    ASSERT_EQ(root -> sum, 89);
}