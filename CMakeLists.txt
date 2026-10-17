cmake_minimum_required(VERSION 3.20)
project(tree-race-starter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(benchmark REQUIRED)
find_package(GTest REQUIRED)

add_executable(tree_exe
    src/main.cpp
    src/csv.h
    src/node_tree.h
)
add_executable(tree_benchmark
    bench/cppbench/tree_benchmark.cpp
    src/csv.h
    src/node_tree.h
)
add_executable(tree_tests
    tst/tree_tests.cpp
    tst/unit_tests.cpp
    src/csv.h
    src/node_tree.h
)

foreach(target tree_exe tree_benchmark tree_tests)
    target_include_directories(${target} PRIVATE src)
    target_compile_definitions(${target} PRIVATE TREE_RACE_SOURCE_DIR="${CMAKE_SOURCE_DIR}")
    target_compile_options(${target} PRIVATE -O3 -DNDEBUG -g)
endforeach()

target_link_libraries(tree_benchmark PRIVATE benchmark::benchmark_main)
target_link_libraries(tree_tests PRIVATE GTest::gtest_main)

enable_testing()
add_test(NAME tree_tests COMMAND tree_tests)
