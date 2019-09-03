/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../util/arr.h"
#include <assert.h>

/* Forward declarations */
static void Free(OpBase *ctx);
static OpResult Reset(OpBase *ctx);
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);

void _DeleteEntities(OpDelete *op) {
	Graph *g = op->gc->g;
	uint node_deleted = 0;
	uint relationships_deleted = 0;
	uint node_count = array_len(op->deleted_nodes);
	uint edge_count = array_len(op->deleted_edges);

	/* Nothing to delete, quickly return. */
	if((node_count + edge_count) == 0) return;

	/* Lock everything. */
	Graph_AcquireWriteLock(g);

	if(GraphContext_HasIndices(op->gc)) {
		for(int i = 0; i < node_count; i++) {
			Node *n = op->deleted_nodes + i;
			GraphContext_DeleteNodeFromIndices(op->gc, n);
		}
	}

	Graph_BulkDelete(g, op->deleted_nodes, node_count, op->deleted_edges,
					 edge_count, &node_deleted, &relationships_deleted);

	/* Release lock. */
	Graph_ReleaseLock(g);

	if(op->stats) op->stats->nodes_deleted += node_deleted;
	if(op->stats) op->stats->relationships_deleted += relationships_deleted;
}

OpBase *NewDeleteOp(const ExecutionPlan *plan, QueryGraph *qg, char **deleted_entities,
					ResultSetStatistics *stats) {
	OpDelete *op = malloc(sizeof(OpDelete));

	op->gc = GraphContext_GetFromTLS();
	op->node_count = 0;
	op->edge_count = 0;
	op->deleted_nodes = array_new(Node, 32);
	op->deleted_edges = array_new(Edge, 32);
	op->stats = stats;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_DELETE, "Delete", Init, Consume, Reset, NULL, Free, plan);

	// Update modifies array to include all deleted nodes
	for(uint i = 0; i < op->node_count; i ++) {
		const char *alias = deleted_entities[i];
		if(QueryGraph_GetEntityTypeByAlias(qg, alias) == ENTITY_NODE) {
			op->nodes_to_delete[op->node_count++] = OpBase_Modifies((OpBase *)op, alias);
		} else {
			op->edges_to_delete[op->edge_count++] = OpBase_Modifies((OpBase *)op, alias);
		}
	}

	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	return OP_OK;
}

static Record Consume(OpBase *opBase) {
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

static OpResult Reset(OpBase *ctx) {
	return OP_OK;
}

static void Free(OpBase *ctx) {
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
