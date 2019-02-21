/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_DELETE_H
#define __OP_DELETE_H

#include "op.h"
#include "../../graph/entities/node.h"
#include "../../resultset/resultset.h"
#include "../../parser/ast.h"
#include "../../util/triemap/triemap.h"
/* Deletes entities specified within the DELETE clause. */


typedef struct {
    OpBase op;
    GraphContext *gc;
    AST *ast;
    size_t node_count;
    size_t edge_count;
    int *nodes_to_delete;
    int *edges_to_delete;
    Node *deleted_nodes;    // Array of nodes to be removed.    
    Edge *deleted_edges;    // Array of edges to be removed.

    ResultSet *result_set;
} OpDelete;

OpBase* NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, GraphContext *gc, ResultSet *result_set, AST *ast);
Record OpDeleteConsume(OpBase *opBase);
OpResult OpDeleteReset(OpBase *ctx);
void OpDeleteFree(OpBase *ctx);

#endif
