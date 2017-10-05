#ifndef __OP_DELETE_H
#define __OP_DELETE_H

#include "op.h"
#include "../../graph/node.h"
#include "../../graph/edge.h"
#include "../../resultset/resultset.h"
#include "../../util/triemap/triemap.h"
/* Delets entities specified within the DELETE clause. */

typedef struct {
    OpBase op;
    RedisModuleCtx *ctx;
    int request_refresh;
    const char *graph_name;

    Node ***nodes_to_delete;
    size_t node_count;

    Edge ***edges_to_delete;
    size_t edge_count;

    TrieMap *deleted_nodes;
    TrieMap *deleted_edges;
    ResultSet *result_set;
} OpDelete;

OpBase* NewDeleteOp(RedisModuleCtx *ctx, AST_DeleteNode *ast_delete_node, Graph *graph, const char *graph_name, ResultSet *result_set);
OpDelete* _NewDeleteOp(RedisModuleCtx *ctx, AST_DeleteNode *ast_delete_node, Graph *graph, const char *graph_name, ResultSet *result_set);

OpResult OpDeleteConsume(OpBase *opBase, Graph* graph);
OpResult OpDeleteReset(OpBase *ctx);
void OpDeleteFree(OpBase *ctx);

#endif