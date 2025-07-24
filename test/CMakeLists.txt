add_executable(fsalloc allocator/fsalloc.c)

target_link_libraries(fsalloc allocator)
target_include_directories(fsalloc PRIVATE ../include)


add_test(NAME fsalloc COMMAND fsalloc 10 5 -1 1)