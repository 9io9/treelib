cmake_minimum_required(VERSION 3.25.1)

project(treelib VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/include/ec.h 
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/tools/pec/pec.py ${CMAKE_CURRENT_SOURCE_DIR}/.ec ${CMAKE_CURRENT_SOURCE_DIR}/include/ec.h _LJW_TREELIB_EC_H_
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/.ec ${CMAKE_CURRENT_SOURCE_DIR}/tools/pec/pec.py
    COMMENT "convert .ec to error code header file"
)

add_custom_target(treegech DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/include/ec.h)

add_library(treelib STATIC src/basic_tree.c src/rb_tree.c)
add_dependencies(treelib treegech)

target_include_directories(treelib PRIVATE include lib)
target_link_libraries(treelib aLocas)

add_subdirectory(lib/aLocas)

enable_testing()
add_subdirectory(test)