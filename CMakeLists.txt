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

add_library(treelib STATIC src/basic_tree.c src/rb_tree.c src/btree.c)
add_dependencies(treelib treegech)

find_package(cmth REQUIRED HINTS "${CMAKE_SOURCE_DIR}/lib/cmth/lib/cmake/cmth")
find_package(aLocas REQUIRED HINTS "${CMAKE_SOURCE_DIR}/lib/aLocas/lib/cmake/aLocas")
find_package(seqlib REQUIRED HINTS "${CMAKE_SOURCE_DIR}/lib/seqlib/lib/cmake/seqlib")

target_include_directories(treelib PRIVATE include lib)
target_link_libraries(treelib aLocas::aLocas seqlib::seqlib cmth::cmth)

# Library
set_target_properties(treelib PROPERTIES EXPORT_NAME treelib)

# Install library and headers
install(
    TARGETS treelib
    EXPORT treelibTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include DESTINATION .)

# Export package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    treelibConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)
configure_file(treelibConfig.cmake.in treelibConfig.cmake @ONLY)
install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/treelibConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/treelibConfigVersion.cmake"
    DESTINATION lib/cmake/treelib
)
install(
    EXPORT treelibTargets
    FILE treelibTargets.cmake
    NAMESPACE treelib::
    DESTINATION lib/cmake/treelib
)

enable_testing()
add_subdirectory(test)