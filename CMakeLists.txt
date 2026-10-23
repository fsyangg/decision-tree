cmake_minimum_required(VERSION 3.20)
project(tree-race-starter LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(benchmark REQUIRED)
find_package(GTest REQUIRED)

option(TRACY_ENABLE "" OFF)
option(TRACY_ON_DEMAND "" OFF)
option(TRACY_NO_EXIT "" ON) # for short progs
add_subdirectory(third_party/tracy)

option(TREE_SIMD "" ON)

add_executable(tree_exe
    src/main.cpp
    src/loaders.h
    src/flat_tree.h
)
target_compile_options(tree_exe PRIVATE -O2 -g -DNDEBUG)

add_executable(tree_benchmark
    bench/cppbench/tree_benchmark.cpp
    src/loaders.h
    src/flat_tree.h
)
target_compile_options(tree_benchmark PRIVATE -O3 -DNDEBUG)

add_executable(tree_tests
    tst/tree_tests.cpp
    src/loaders.h
    src/flat_tree.h
)
target_compile_options(tree_tests PRIVATE -O3 -DNDEBUG)

foreach(target tree_exe tree_benchmark tree_tests)
    target_include_directories(${target} PRIVATE src)
    target_compile_definitions(${target} PRIVATE TREE_RACE_SOURCE_DIR="${CMAKE_SOURCE_DIR}" TREE_SIMD)
endforeach()

target_link_libraries(tree_exe PUBLIC Tracy::TracyClient)
target_link_libraries(tree_benchmark PRIVATE benchmark::benchmark_main Tracy::TracyClient)
target_link_libraries(tree_tests PRIVATE GTest::gtest_main Tracy::TracyClient)

enable_testing()
add_test(NAME tree_tests COMMAND tree_tests)
