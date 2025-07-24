cmake_minimum_required(VERSION 3.25.1)

project(treelib VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_library(treelib STATIC src/basic_tree.c)
add_library(allocator STATIC src/allocator/fsalloc.c)

add_executable(fsalloc test/allocator/fsalloc.c)

target_include_directories(allocator PRIVATE include)
target_include_directories(treelib PRIVATE include)
target_include_directories(fsalloc PRIVATE include)
target_link_libraries(treelib allocator)
target_link_libraries(fsalloc allocator)

enable_testing()

add_test(NAME fsalloc COMMAND fsalloc 10 5 -1 1)