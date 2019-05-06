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
#include "../../resultset/resultset_statistics.h"
#include "../../parser/ast_shared.h"

/* Creates new entities according to the CREATE clause. */

typedef struct {
    OpBase op;
    GraphContext *gc;
    QueryGraph *qg;
    Record *records;
    uint record_len;

    NodeCreateCtx *nodes_to_create;
    EdgeCreateCtx *edges_to_create;

    // TODO tmp, improve
    PropertyMap **node_properties;
    PropertyMap **edge_properties;

    Node **created_nodes;
    Edge **created_edges;
    ResultSetStatistics *stats;
} OpCreate;

OpBase* NewCreateOp(ResultSetStatistics *stats, NodeCreateCtx *nodes, EdgeCreateCtx *edges, uint record_len);

Record OpCreateConsume(OpBase *opBase);
OpResult OpCreateReset(OpBase *ctx);
void OpCreateFree(OpBase *ctx);

#endif
