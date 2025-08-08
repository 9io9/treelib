add_executable(bst basic_tree.c)
add_executable(rbt rb_tree.c)

target_include_directories(bst PRIVATE ../include ../lib)
target_link_libraries(bst treelib)
target_include_directories(rbt PRIVATE ../include ../lib)
target_link_libraries(rbt treelib)

add_test(NAME basic_tree COMMAND bst 88 11 33 46 79 23 55 1 2 5 10 80 83 92 66 77)
add_test(NAME rb_tree COMMAND rbt 88 11 33 46 79 23 55 1 2 5 10 80 83 92 66 77)