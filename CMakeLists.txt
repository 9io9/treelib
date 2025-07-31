cmake_minimum_required(VERSION 3.25.1)

project(treelib VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(lib/aLocas)

add_library(treelib STATIC src/basic_tree.c)

target_include_directories(treelib PRIVATE include lib)
target_link_libraries(treelib aLocas)

# enable_testing()
# add_subdirectory(test)