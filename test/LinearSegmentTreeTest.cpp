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
            q.push(2*n);
            q.push(2*n + 1);
        }
        q.pop();
    }

}

TEST(LinearSegmentTreeTestSuite, UpdateTest) {
    std::vector a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    inflate::linear_segment_tree<int> tree(a.begin(), a.end());
    tree.add_operation([](int a, int b, size_t begin_pos, size_t end_pos) {return a + b * (end_pos - begin_pos);});
    tree.update(0, 10, 0, 3);
    auto root = reinterpret_cast<const inflate::segment_tree_node<int, int>*>(& (tree.root_node()));
    ASSERT_TRUE(root->_tag);
    ASSERT_EQ(root->sum, 85);
    tree.update(4, 6, 0, 2);
    ASSERT_FALSE(root[1]._tag);
    ASSERT_EQ(root -> sum, 89);
}

TEST(LinearSegmentTreeTestSuite, ComprehensiveTest) {
    std::vector a = {1, 5, 4, 2, 3};
    inflate::linear_segment_tree<int> tree(a.begin(), a.end());
    tree.add_operation([](int a, int b, size_t begin_pos, size_t end_pos) {return a + b * (end_pos - begin_pos);});
    ASSERT_EQ(tree.query(0, 5), 15);
    ASSERT_EQ(tree.query(1, 4), 11);
    tree.update(1, 3, 0, 2);
    ASSERT_EQ(tree.query(2, 4), 8);
    tree.update(0, 5, 0, 1);
    ASSERT_EQ(tree.query(0, 4), 20);
}