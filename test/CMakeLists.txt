enable_testing()

add_executable(test_edge test_edge.c ${graph_files})
add_test(test_edge test_edge)

add_executable(test_node test_node.c ${graph_files})
add_test(test_node test_node)

add_executable(test_triplet test_triplet.c ${graph_files})
add_test(test_triplet test_triplet)

add_executable(test_graph test_graph.c ${graph_files})
add_test(test_graph test_graph)

add_executable(test_agg_sum test_agg_sum.c ${graph_files})
add_test(test_agg_sum test_agg_sum)

add_executable(test_filter_tree test_filter_tree.c ${graph_files})
add_test(test_filter_tree test_filter_tree)

add_executable(test_hexastore test_hexastore.c ${graph_files})
add_test(test_hexastore test_hexastore)