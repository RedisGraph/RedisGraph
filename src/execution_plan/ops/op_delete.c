/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./op_delete.h"
#include "../../arithmetic/arithmetic_expression.h"
#include "../../util/arr.h"
#include <assert.h>

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

OpBase *NewDeleteOp(QueryGraph *qg, char **deleted_entities, ResultSetStatistics *stats) {
	OpDelete *op_delete = malloc(sizeof(OpDelete));

	op_delete->gc = GraphContext_GetFromTLS();
	op_delete->node_count = 0;
	op_delete->edge_count = 0;
	op_delete->deleted_nodes = array_new(Node, 32);
	op_delete->deleted_edges = array_new(Edge, 32);
	op_delete->stats = stats;

	// Set our Op operations
	OpBase_Init(&op_delete->op);
	op_delete->op.name = "Delete";
	op_delete->op.type = OPType_DELETE;
	op_delete->op.consume = OpDeleteConsume;
	op_delete->op.init = OpDeleteInit;
	op_delete->op.reset = OpDeleteReset;
	op_delete->op.free = OpDeleteFree;

	// Update modifies array to include all deleted nodes
	for(uint i = 0; i < op_delete->node_count; i ++) {
		const char *alias = deleted_entities[i];
		if(QueryGraph_GetEntityTypeByAlias(qg, alias) == ENTITY_NODE) {
			op_delete->nodes_to_delete[op_delete->node_count++] = OpBase_Modifies((OpBase *)op_delete, alias);
		} else {
			op_delete->edges_to_delete[op_delete->edge_count++] = OpBase_Modifies((OpBase *)op_delete, alias);
		}
	}

	return (OpBase *)op_delete;
}

OpResult OpDeleteInit(OpBase *opBase) {
	return OP_OK;
}

Record OpDeleteConsume(OpBase *opBase) {
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

OpResult OpDeleteReset(OpBase *ctx) {
	return OP_OK;
}

void OpDeleteFree(OpBase *ctx) {
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
