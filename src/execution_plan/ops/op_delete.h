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
    Graph *g;
    QueryGraph *qg;
    size_t node_count;
    size_t edge_count;
    char **nodes_to_delete;
    char **edges_to_delete;
    NodeID *deleted_nodes;              // Array of nodes to be removed.    
    EdgeID *deleted_edges;              // Array of edges to be removed.

    ResultSet *result_set;
} OpDelete;

OpBase* NewDeleteOp(AST_DeleteNode *ast_delete_node, QueryGraph *qg, Graph *g, ResultSet *result_set);
OpResult OpDeleteConsume(OpBase *opBase, Record *r);
OpResult OpDeleteReset(OpBase *ctx);
void OpDeleteFree(OpBase *ctx);

#endif
