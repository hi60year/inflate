//
// Created by Administrator on 10/13/2023.
//

#include "../ds/segment_tree.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <print>

class LinearSegmentTreeTestSuite : ::testing::Test {

};

TEST(LinearSegmentTreeTestSuite, ConstructionFromIteratorRangeTest) {
    std::vector a = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    inflate::linear_segment_tree<int> tree(a.begin(), a.end());
    ASSERT_EQ(tree.size(), a.size());
    ASSERT_EQ(tree.allocation_size(), a.size()*4);
}