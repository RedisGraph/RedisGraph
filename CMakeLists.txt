cmake_minimum_required(VERSION 3.0.0)
project(graph VERSION 0.0.1)

include(CTest)
enable_testing()

add_subdirectory(src)
add_subdirectory(test)