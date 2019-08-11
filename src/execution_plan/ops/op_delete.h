/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "op.h"
#include "../../graph/entities/node.h"
#include "../../resultset/resultset_statistics.h"
#include "../../util/triemap/triemap.h"
/* Deletes entities specified within the DELETE clause. */


typedef struct {
	OpBase op;
	GraphContext *gc;
	uint node_count;
	uint edge_count;
	uint *nodes_to_delete;
	uint *edges_to_delete;
	Node *deleted_nodes;    // Array of nodes to be removed.
	Edge *deleted_edges;    // Array of edges to be removed.

	ResultSetStatistics *stats;
} OpDelete;

OpBase *NewDeleteOp(QueryGraph *qg, char **deleted_entities, ResultSetStatistics *stats);
Record OpDeleteConsume(OpBase *opBase);
OpResult OpDeleteInit(OpBase *opBase);
OpResult OpDeleteReset(OpBase *ctx);
void OpDeleteFree(OpBase *ctx);
