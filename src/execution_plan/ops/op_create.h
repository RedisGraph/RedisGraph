/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef __OP_CREATE_H
#define __OP_CREATE_H

#include "op.h"
#include "../../graph/entities/node.h"
#include "../../graph/entities/edge.h"
#include "../../resultset/resultset.h"
/* Creates new entities according to the CREATE clause. */

typedef struct {
    Edge *edge;
    const cypher_astnode_t *ast_entity;
    int src_node_rec_idx;
    int dest_node_rec_idx;
    int edge_rec_idx;
} EdgeCreateCtx;

typedef struct {
    Node *node;
    const cypher_astnode_t *ast_entity;
    int node_rec_idx;
} NodeCreateCtx;

typedef struct {
    OpBase op;
    GraphContext *gc;
    NEWAST *ast;
    QueryGraph *qg;
    Record *records;

    NodeCreateCtx *nodes_to_create;
    size_t node_count;

    EdgeCreateCtx *edges_to_create;
    size_t edge_count;

    Node **created_nodes;
    Edge **created_edges;
    ResultSet *result_set;
} OpCreate;

OpBase* NewCreateOp(RedisModuleCtx *ctx, QueryGraph *qg, ResultSet *result_set);

Record OpCreateConsume(OpBase *opBase);
OpResult OpCreateReset(OpBase *ctx);
void OpCreateFree(OpBase *ctx);

#endif
