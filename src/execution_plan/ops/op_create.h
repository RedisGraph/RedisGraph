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
#include "../../parser/ast.h"
#include "../../parser/ast_build_op_contexts.h"

/* Creates new entities according to the CREATE clause. */

typedef struct {
    Edge *edge;
    PropertyMap *properties;
    int src_node_rec_idx;
    int dest_node_rec_idx;
    int edge_rec_idx;
} EdgeCreateCtx;

typedef struct {
    Node *node;
    PropertyMap *properties;
    int node_rec_idx;
} NodeCreateCtx;

typedef struct {
    OpBase op;
    GraphContext *gc;
    AST *ast;
    QueryGraph *qg;
    Record *records;

    NodeCreateCtx *nodes_to_create;
    size_t node_count;

    EdgeCreateCtx *edges_to_create;
    size_t edge_count;

    // TODO tmp, improve
    PropertyMap **node_properties;
    PropertyMap **edge_properties;

    Node **created_nodes;
    Edge **created_edges;
    ResultSet *result_set;
} OpCreate;

OpBase* NewCreateOp(RedisModuleCtx *ctx, QueryGraph *qg, ResultSet *result_set);

Record OpCreateConsume(OpBase *opBase);
OpResult OpCreateReset(OpBase *ctx);
void OpCreateFree(OpBase *ctx);

#endif
