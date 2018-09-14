/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_DELETE_H
#define __OP_DELETE_H

#include "op.h"
#include "../../graph/node.h"
#include "../../resultset/resultset.h"
#include "../../util/triemap/triemap.h"
#include "../../index/index_updates.h"

/* Deletes entities specified within the DELETE clause. */

typedef struct {
    OpBase op;
    RedisModuleCtx *ctx;
    Graph *g;
    const char *graph_name;
    QueryGraph *qg;
    size_t node_count;
    Node ***nodes_to_delete;
    size_t edge_count;
    Edge ***edges_to_delete;
    NodeID *deleted_nodes;              // Array of nodes to be removed.    
    EdgeID *deleted_edges;              // Array of edges to be removed.

    ResultSet *result_set;
} OpDelete;

OpBase* NewDeleteOp(RedisModuleCtx *ctx, AST_DeleteNode *ast_delete_node, QueryGraph *qg, Graph *g, const char *graph_name, ResultSet *result_set);
OpResult OpDeleteConsume(OpBase *opBase, QueryGraph* graph);
OpResult OpDeleteReset(OpBase *ctx);
void OpDeleteFree(OpBase *ctx);

#endif
