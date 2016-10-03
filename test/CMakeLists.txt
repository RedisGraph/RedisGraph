add_executable(test_edge test_edge.c ${graph_files})
add_test(test_edge test_edge)

add_executable(test_node test_node.c ${graph_files})
add_test(test_node test_node)

add_executable(test_triplet test_triplet.c ${graph_files})
add_test(test_triplet test_triplet)