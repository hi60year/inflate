cmake_minimum_required(VERSION 3.26)
project(inflate)

set(CMAKE_CXX_STANDARD 23)

add_library(inflate STATIC main.cpp
        ds/segment_tree.hpp
        ds/partial_sum_series.hpp
        ds/order_statistics_tree.hpp)

add_subdirectory(test)