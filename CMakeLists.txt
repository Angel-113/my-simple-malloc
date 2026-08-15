cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(MyMalloc C)

add_library(malloc_core
    src/core/base.c
    src/api/allocator.c
    src/core/header.c
    src/core/rb_tree.c
    src/api/allocator_helper.c
)

target_include_directories(malloc_core PUBLIC include)

add_executable(run_tests
    test/utils/test.c
    test/utils/rand.c
    test/suites/rb_tree_test.c
    test/suites/allocator_test.c
    test/utils/rand_test.c
    test/utils/logger.c
)

target_link_libraries(run_tests PRIVATE malloc_core)

# Testing
set(NODES_TO_INSERT 750000 CACHE STRING "Number of nodes to insert in tree tests")
set(NODES_TO_DELETE 500000 CACHE STRING "Number of nodes to delete in tree tests")
set(MAX_NODES 1000000 CACHE STRING "Max number of nodes in the tree tests")

target_compile_definitions(run_tests PRIVATE
    NODES_TO_INSERT=${NODES_TO_INSERT}
    NODES_TO_DELETE=${NODES_TO_DELETE}
)

enable_testing()

# tree testing
add_test(NAME tree_general COMMAND run_tests "tree general")
add_test(NAME tree_insertion COMMAND run_tests "tree insertion")
add_test(NAME tree_deletion COMMAND run_tests "tree deletion")
add_test(NAME tree_performance COMMAND run_tests "tree performance")

# Allocator examples test
set(test 1)
set(max_tests 5)
while (test LESS_EQUAL max_tests)
    add_test(NAME "allocator_example${test}" COMMAND run_tests "allocator examples ${test}")
    math(EXPR test "${test} + 1")
endwhile()
