/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../arithmetic/arithmetic_expression.h"
#include <assert.h>

/* Forward declarations. */
static Record DeleteConsume(OpBase *opBase);
static void DeleteFree(OpBase *opBase);

void _DeleteEntities(OpDelete *op) {
	Graph *g = op->gc->g;
	uint node_deleted = 0;
	uint relationships_deleted = 0;
	uint node_count = array_len(op->deleted_nodes);
	uint edge_count = array_len(op->deleted_edges);

	/* Nothing to delete, quickly return. */
	if((node_count + edge_count) == 0) return;

	/* Lock everything. */
	QueryCtx_LockForCommit();

	if(GraphContext_HasIndices(op->gc)) {
		for(int i = 0; i < node_count; i++) {
			Node *n = op->deleted_nodes + i;
			GraphContext_DeleteNodeFromIndices(op->gc, n);
		}
	}

	Graph_BulkDelete(g, op->deleted_nodes, node_count, op->deleted_edges,
					 edge_count, &node_deleted, &relationships_deleted);

	/* Release lock. */
	QueryCtx_UnlockCommit(&op->op);

	if(op->stats) op->stats->nodes_deleted += node_deleted;
	if(op->stats) op->stats->relationships_deleted += relationships_deleted;
}

OpBase *NewDeleteOp(const ExecutionPlan *plan, const char **nodes_ref, const char **edges_ref,
					ResultSetStatistics *stats) {
	OpDelete *op = malloc(sizeof(OpDelete));

	op->gc = QueryCtx_GetGraphCtx();

	op->nodes_to_delete = array_new(int, array_len(nodes_ref));
	op->edges_to_delete = array_new(int, array_len(edges_ref));

	op->deleted_nodes = array_new(Node, 32);
	op->deleted_edges = array_new(Edge, 32);
	op->stats = stats;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_DELETE, "Delete", NULL, DeleteConsume,
				NULL, NULL, DeleteFree, plan);

	// Set nodes/edges to be deleted record indices.
	int idx;
	int node_count = array_len(nodes_ref);
	for(int i = 0; i < node_count; i++) {
		assert(OpBase_Aware((OpBase *)op, nodes_ref[i], &idx));
		op->nodes_to_delete = array_append(op->nodes_to_delete, idx);
	}

	int edge_count = array_len(edges_ref);
	for(int i = 0; i < edge_count; i++) {
		assert(OpBase_Aware((OpBase *)op, edges_ref[i], &idx));
		op->edges_to_delete = array_append(op->edges_to_delete, idx);
	}

	op->node_count = array_len(op->nodes_to_delete);
	op->edge_count = array_len(op->edges_to_delete);

	return (OpBase *)op;
}

static Record DeleteConsume(OpBase *opBase) {
	OpDelete *op = (OpDelete *)opBase;
	OpBase *child = op->op.children[0];

	Record r = OpBase_Consume(child);
	if(!r) return NULL;

	/* Enqueue entities for deletion. */
	for(int i = 0; i < op->node_count; i++) {
		Node *n = Record_GetNode(r, op->nodes_to_delete[i]);
		op->deleted_nodes = array_append(op->deleted_nodes, *n);
	}

	for(int i = 0; i < op->edge_count; i++) {
		Edge *e = Record_GetEdge(r, op->edges_to_delete[i]);
		op->deleted_edges = array_append(op->deleted_edges, *e);
	}

	return r;
}

static void DeleteFree(OpBase *ctx) {
	OpDelete *op = (OpDelete *)ctx;

	if(op->deleted_nodes || op->deleted_edges) _DeleteEntities(op);

	if(op->nodes_to_delete) {
		array_free(op->nodes_to_delete);
		op->nodes_to_delete = NULL;
	}

	if(op->edges_to_delete) {
		array_free(op->edges_to_delete);
		op->edges_to_delete = NULL;
	}

	if(op->deleted_nodes) {
		array_free(op->deleted_nodes);
		op->deleted_nodes = NULL;
	}

	if(op->deleted_edges) {
		array_free(op->deleted_edges);
		op->deleted_edges = NULL;
	}
}

