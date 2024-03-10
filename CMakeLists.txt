cmake_minimum_required(VERSION 3.26)

project(graph-mining)

set(CMAKE_CXX_STANDARD 20)
add_executable(main
        src/main.cpp src/graph/graph.cpp src/graph/kplex.cpp
)
target_include_directories(main PRIVATE src)
target_precompile_headers(main PRIVATE src/pch.hpp)