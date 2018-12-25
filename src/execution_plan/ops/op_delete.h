/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef __OP_DELETE_H
#define __OP_DELETE_H

#include "op.h"
#include "../../graph/entities/node.h"
#include "../../resultset/resultset.h"
#include "../../util/triemap/triemap.h"
/* Delets entities specified within the DELETE clause. */


typedef struct {
    OpBase op;
    GraphContext *gc;
    QueryGraph *qg;
    size_t node_count;
    size_t edge_count;
    int *nodes_to_delete;
    int *edges_to_delete;
    Node *deleted_nodes;    // Array of nodes to be removed.    
    Edge *deleted_edges;    // Array of edges to be removed.

    ResultSet *result_set;
} OpDelete;

OpBase* NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, GraphContext *gc, ResultSet *result_set);
Record OpDeleteConsume(OpBase *opBase);
OpResult OpDeleteReset(OpBase *ctx);
void OpDeleteFree(OpBase *ctx);

#endif
