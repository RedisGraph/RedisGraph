# The file list for building the graph database library.
# They can be used directly for unit tests.
# the ../src part is so that they can be accessed from outside the src tree
set(_graph_files
      ../src/value.c
      ../src/module.c
      ../src/value_cmp.c
      ../src/query_executor.c

      ../src/hexastore/hexastore.c
      ../src/hexastore/triplet.c

      ../src/filter_tree/filter_tree.c

      ../src/stores/store.c

      ../src/graph/edge.c
      ../src/graph/node.c
      ../src/graph/graph.c

      ../src/parser/ast.c
      ../src/parser/lex.yy.c
      ../src/parser/grammar.c

      ../src/aggregate/aggregate.c
      ../src/aggregate/functions.c
      ../src/aggregate/repository.c

      ../src/grouping/group.c
      ../src/grouping/group_cache.c

      ../src/util/heap.c
      ../src/util/sha1.c
      ../src/util/prng.c
      ../src/util/triemap/triemap.c
      ../src/util/triemap/triemap_type.c

      ../src/resultset/record.c
      ../src/resultset/resultset.c

      ../src/execution_plan/ops/op_node_by_label_scan.c
      ../src/execution_plan/ops/op_all_node_scan.c
      ../src/execution_plan/ops/op_expand_all.c
      ../src/execution_plan/ops/op_expand_into.c
      ../src/execution_plan/ops/op_produce_results.c
      ../src/execution_plan/ops/op_filter.c
      ../src/execution_plan/ops/op_aggregate.c

      ../src/execution_plan/execution_plan.c

      ../src/rmutil/sds.c
      ../src/rmutil/util.c
      ../src/rmutil/vector.c
      ../src/rmutil/strings.c
      )


set (graph_files ${_graph_files} PARENT_SCOPE)

# Libgraph target - a static library for graph database
add_library(Llibgraph STATIC  ${_graph_files})

target_compile_options(Llibgraph PUBLIC "-fPIC" "-DREDIS_MODULE_TARGET" "-I${CMAKE_CURRENT_LIST_DIR}")


# build the parser source code before building Llibgraph
#add_custom_command(
#                TARGET Llibgraph
#                PRE_BUILD
#                COMMAND lex -i lexer.l
#                COMMAND lemon -s grammar.y
#                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/parser)



add_library(module MODULE
    module.c
    rmutil/util.c
    rmutil/strings.c
)

add_library(librmutil STATIC IMPORTED ../../src/rmutil/librmutil.a)

target_compile_options(module PUBLIC "-DREDIS_MODULE_TARGET")

target_link_libraries(module Llibgraph)
target_link_libraries(module m)