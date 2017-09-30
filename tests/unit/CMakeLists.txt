enable_testing()

set(test_graph_files
      ../../src/value.c
      ../../src/module.c
      ../../src/value_cmp.c
      ../../src/query_executor.c

      ../../src/hexastore/hexastore.c
      ../../src/hexastore/triplet.c

      ../../src/filter_tree/filter_tree.c

      ../../src/stores/store.c

      ../../src/graph/graph_entity.c
      ../../src/graph/edge.c
      ../../src/graph/node.c
      ../../src/graph/graph.c

      ../../src/parser/ast.c
      ../../src/parser/lex.yy.c
      ../../src/parser/grammar.c

      ../../src/aggregate/aggregate.c
      ../../src/aggregate/agg_funcs.c
      ../../src/aggregate/repository.c

      ../../src/grouping/group.c
      ../../src/grouping/group_cache.c

      ../../src/util/heap.c
      ../../src/util/sha1.c
      ../../src/util/prng.c
      ../../src/util/snowflake.c
      ../../src/util/triemap/triemap.c
      ../../src/util/triemap/triemap_type.c

      ../../src/resultset/record.c
      ../../src/resultset/resultset.c

      ../../src/execution_plan/ops/op_node_by_label_scan.c
      ../../src/execution_plan/ops/op_all_node_scan.c
      ../../src/execution_plan/ops/op_expand_all.c
      ../../src/execution_plan/ops/op_expand_into.c
      ../../src/execution_plan/ops/op_produce_results.c
      ../../src/execution_plan/ops/op_filter.c
      ../../src/execution_plan/ops/op_aggregate.c
      ../../src/execution_plan/ops/op_create.c

      ../../src/execution_plan/execution_plan.c

      ../../src/rmutil/sds.c
      ../../src/rmutil/util.c
      ../../src/rmutil/vector.c
      ../../src/rmutil/strings.c
      )

add_executable(test_edge test_edge.c ${test_graph_files})
add_test(test_edge test_edge)

add_executable(test_node test_node.c ${test_graph_files})
add_test(test_node test_node)

add_executable(test_triplet test_triplet.c ${test_graph_files})
add_test(test_triplet test_triplet)

add_executable(test_graph test_graph.c ${test_graph_files})
add_test(test_graph test_graph)

add_executable(test_agg_sum test_agg_sum.c ${test_graph_files})
add_test(test_agg_sum test_agg_sum)

add_executable(test_filter_tree test_filter_tree.c ${test_graph_files})
add_test(test_filter_tree test_filter_tree)

add_executable(test_hexastore test_hexastore.c ${test_graph_files})
add_test(test_hexastore test_hexastore)

add_executable(test_resultset_record test_resultset_record.c ${test_graph_files})
add_test(test_resultset_record test_resultset_record)

add_executable(test_value test_value.c ${test_graph_files})
add_test(test_value test_value)
