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
#include "../../parser/ast.h"
#include "../../resultset/resultset.h"
/* Creates new entities according to the CREATE clause. */

typedef struct {
	Edge *edge;
	int src_node_rec_idx;
	int dest_node_rec_idx;
	int edge_rec_idx;
} EdgeCreateCtx;

typedef struct {
	Node *node;
	int node_rec_idx;
} NodeCreateCtx;

typedef struct {
	OpBase op;
	AST *ast;
	QueryGraph *qg;
	GraphContext *gc;
	Record *records;
	size_t node_count;
	size_t edge_count;
	Node **created_nodes;
	Edge **created_edges;
	ResultSet *result_set;
	NodeCreateCtx *nodes_to_create;
	EdgeCreateCtx *edges_to_create;
} OpCreate;

OpBase *NewCreateOp(RedisModuleCtx *ctx, AST *ast, QueryGraph *qg,
                    ResultSet *result_set);

Record OpCreateConsume(OpBase *opBase);
OpResult OpCreateReset(OpBase *ctx);
void OpCreateFree(OpBase *ctx);

#endif
