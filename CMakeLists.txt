cmake_minimum_required(VERSION 3.26)
project(inflate)

set(CMAKE_CXX_STANDARD 23)

add_executable(inflate main.cpp
        ds/segment_tree.hpp)
