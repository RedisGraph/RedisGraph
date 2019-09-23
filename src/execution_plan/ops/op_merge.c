/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge.h"

#include "../../query_ctx.h"
#include "../../schema/schema.h"
#include "../../arithmetic/arithmetic_expression.h"
#include <assert.h>

/* Forward declarations. */
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);
static OpResult Reset(OpBase *opBase);
static void Free(OpBase *opBase);

static void _AddProperties(OpMerge *op, Record r, GraphEntity *ge, PropertyMap *props) {
	for(int i = 0; i < props->property_count; i++) {
		SIValue val = AR_EXP_Evaluate(props->values[i], r);
		GraphEntity_AddProperty(ge, props->keys[i], val);
		SIValue_Free(&val);
	}

	if(op->stats) op->stats->properties_set += props->property_count;
}

/* Saves every entity within the query graph into the actual graph.
 * update statistics regarding the number of entities create and properties set. */
static void _CommitNodes(OpMerge *op, Record r) {
	int labelID;
	Graph *g = op->gc->g;

	uint node_count = array_len(op->nodes_to_merge);

	// Start by creating nodes.
	Graph_AllocateNodes(g, node_count);

	for(uint i = 0; i < node_count; i ++) {
		NodeCreateCtx *node_ctx = &op->nodes_to_merge[i];
		// Get blueprint of node to create
		QGNode *node_blueprint = node_ctx->node;

		// Newly created node will be placed within given record.
		Node *created_node = Record_GetNode(r, node_ctx->node_idx);

		Schema *schema = NULL;

		// Set, create label.
		if(node_blueprint->label == NULL) {
			labelID = GRAPH_NO_LABEL;
		} else {
			schema = GraphContext_GetSchema(op->gc, node_blueprint->label, SCHEMA_NODE);
			/* This is the first time we've encountered this label; create its schema */
			if(schema == NULL) {
				schema = GraphContext_AddSchema(op->gc, node_blueprint->label, SCHEMA_NODE);
				op->stats->labels_added++;
			}
			labelID = schema->id;
		}

		Graph_CreateNode(g, labelID, created_node);

		// Convert properties and add to newly-created node.
		PropertyMap *map = node_ctx->properties;
		if(map) _AddProperties(op, r, (GraphEntity *)created_node, map);

		if(schema) Schema_AddNodeToIndices(schema, created_node, false);
	}

	op->stats->nodes_created += node_count;
}

static void _CommitEdges(OpMerge *op, Record r) {
	// Create edges.

	uint edge_count = array_len(op->edges_to_merge);
	// TODO allocate? nodes get allocated here
	for(uint i = 0; i < edge_count; i ++) {
		EdgeCreateCtx *edge_ctx = &op->edges_to_merge[i];
		// Get blueprint of edge to create
		QGEdge *edge_blueprint = edge_ctx->edge;

		// Newly created edge will be placed within given record.
		Edge *created_edge = Record_GetEdge(r, edge_ctx->edge_idx);

		// An edge must have exactly 1 relationship type.
		Schema *schema = GraphContext_GetSchema(op->gc, edge_blueprint->reltypes[0], SCHEMA_EDGE);
		if(!schema) schema = GraphContext_AddSchema(op->gc, edge_blueprint->reltypes[0], SCHEMA_EDGE);

		// Node are already created, get them from record.
		int src_idx = Record_GetEntryIdx(r, edge_ctx->edge->src->alias); // TODO tmp
		int dest_idx = Record_GetEntryIdx(r, edge_ctx->edge->dest->alias); // TODO tmp
		EntityID srcId = ENTITY_GET_ID(Record_GetNode(r, src_idx));
		EntityID destId = ENTITY_GET_ID(Record_GetNode(r, dest_idx));

		assert(Graph_ConnectNodes(op->gc->g, srcId, destId, schema->id, created_edge));

		// Convert properties and add to newly-created node.
		PropertyMap *map = edge_ctx->properties;
		if(map) _AddProperties(op, r, (GraphEntity *)created_edge, map);
	}

	if(op->stats) op->stats->relationships_created += edge_count;
}

static void _CreateEntities(OpMerge *op, Record r) {
	// Track the inherited Record and the newly-allocated Record so that they may be freed if execution fails.
	OpBase_AddVolatileRecord((OpBase *)op, r);

	// Lock everything.
	Graph_AcquireWriteLock(op->gc->g);

	// Commit query graph and set resultset statistics.
	_CommitNodes(op, r);
	_CommitEdges(op, r);

	// Release lock.
	Graph_ReleaseLock(op->gc->g);

	OpBase_RemoveVolatileRecords((OpBase *)op); // No exceptions encountered, Records are not dangling.

}

OpBase *NewMergeOp(const ExecutionPlan *plan, ResultSetStatistics *stats,
				   NodeCreateCtx *nodes_to_merge, EdgeCreateCtx *edges_to_merge) {
	OpMerge *op = malloc(sizeof(OpMerge));
	op->stats = stats;
	op->gc = QueryCtx_GetGraphCtx();
	op->matched = false;
	op->created = false;
	op->nodes_to_merge = nodes_to_merge;
	op->edges_to_merge = edges_to_merge;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_MERGE, "Merge", Init, Consume, Reset, NULL, Free, plan);

	int node_count = array_len(op->nodes_to_merge);
	for(int i = 0; i < node_count; i++) {
		// TODO unsafe assumption
		assert(OpBase_Aware((OpBase *)op, op->nodes_to_merge[i].node->alias,
							&op->nodes_to_merge[i].node_idx));
	}

	int edge_count = array_len(op->edges_to_merge);
	for(int i = 0; i < edge_count; i++) {
		// TODO unsafe assumption
		assert(OpBase_Aware((OpBase *)op, op->edges_to_merge[i].edge->alias,
							&op->edges_to_merge[i].edge_idx));
	}

	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	return OP_OK;
}

static Record Consume(OpBase *opBase) {
	OpMerge *op = (OpMerge *)opBase;

	/* Pattern was created in the previous call
	 * Execution plan is already depleted. */
	if(op->created) return NULL;

	OpBase *child = op->op.children[0];
	Record r = OpBase_Consume(child);
	if(r) {
		/* If we're here that means pattern was matched!
		* in that case there's no need to create any graph entity,
		* we can simply return. */
		op->matched = true;
	} else {
		/* In case there a previous match, execution plan
		 * is simply depleted, no need to create the pattern. */
		if(op->matched) return r;

		// No previous match, create MERGE pattern.

		/* TODO: once MATCH and MERGE will be mixed
		 * we'll need to apply a similar strategy applied by op_create. */

		/* Done reading, we're not going to call consume any longer
		 * there might be operations e.g. index scan that need to free
		 * index R/W lock, as such free all execution plan operation up the chain. */
		OpBase_PropagateFree(child);

		r = OpBase_CreateRecord((OpBase *)op);
		_CreateEntities(op, r);
		op->created = true;
	}

	return r;
}

static OpResult Reset(OpBase *ctx) {
	// Merge doesn't modify anything.
	return OP_OK;
}

static void Free(OpBase *ctx) {
	OpMerge *op = (OpMerge *)ctx;

	if(op->nodes_to_merge) {
		uint node_count = array_len(op->nodes_to_merge);
		for(uint i = 0; i < node_count; i ++) {
			PropertyMap_Free(op->nodes_to_merge[i].properties);
		}
		op->nodes_to_merge = NULL;
	}

	if(op->edges_to_merge) {
		uint edge_count = array_len(op->edges_to_merge);
		for(uint i = 0; i < edge_count; i ++) {
			PropertyMap_Free(op->edges_to_merge[i].properties);
		}
		op->edges_to_merge = NULL;
	}
}

