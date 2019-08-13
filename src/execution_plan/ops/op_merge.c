/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge.h"

#include "../../schema/schema.h"
#include "../../arithmetic/arithmetic_expression.h"
#include <assert.h>

static void _AddProperties(OpMerge *op, Record r, GraphEntity *ge,
						   PropertyMap *props) {
	for(int i = 0; i < props->property_count; i++) {
		Attribute_ID prop_id = GraphContext_FindOrAddAttribute(op->gc, props->keys[i]);
		SIValue val = AR_EXP_Evaluate(props->values[i], r);
		GraphEntity_AddProperty(ge, prop_id, val);
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

		if(schema) GraphContext_AddNodeToIndices(op->gc, schema, created_node);
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
		EntityID srcId = ENTITY_GET_ID(Record_GetNode(r, edge_ctx->src_idx));
		EntityID destId = ENTITY_GET_ID(Record_GetNode(r, edge_ctx->dest_idx));

		assert(Graph_ConnectNodes(op->gc->g, srcId, destId, schema->id, created_edge));

		// Convert properties and add to newly-created node.
		PropertyMap *map = edge_ctx->properties;
		if(map) _AddProperties(op, r, (GraphEntity *)created_edge, map);
	}

	if(op->stats) op->stats->relationships_created += edge_count;
}

static void _CreateEntities(OpMerge *op, Record r) {
	// Lock everything.
	Graph_AcquireWriteLock(op->gc->g);

	// Commit query graph and set resultset statistics.
	_CommitNodes(op, r);
	_CommitEdges(op, r);

	// Release lock.
	Graph_ReleaseLock(op->gc->g);
}

OpBase *NewMergeOp(ResultSetStatistics *stats, NodeCreateCtx *nodes_to_merge,
				   EdgeCreateCtx *edges_to_merge) {
	OpMerge *op_merge = malloc(sizeof(OpMerge));
	op_merge->stats = stats;
	op_merge->gc = GraphContext_GetFromTLS();
	op_merge->matched = false;
	op_merge->created = false;

	op_merge->nodes_to_merge = nodes_to_merge;
	op_merge->edges_to_merge = edges_to_merge;

	// Set our Op operations
	OpBase_Init(&op_merge->op);
	op_merge->op.name = "Merge";
	op_merge->op.type = OPType_MERGE;
	op_merge->op.consume = OpMergeConsume;
	op_merge->op.init = OpMergeInit;
	op_merge->op.reset = OpMergeReset;
	op_merge->op.free = OpMergeFree;

	return (OpBase *)op_merge;
}

OpResult OpMergeInit(OpBase *opBase) {
	return OP_OK;
}

Record OpMergeConsume(OpBase *opBase) {
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
		r = Record_New(opBase->record_map->record_len);
		_CreateEntities(op, r);
		op->created = true;
	}

	return r;
}

OpResult OpMergeReset(OpBase *ctx) {
	// Merge doesn't modify anything.
	return OP_OK;
}

void OpMergeFree(OpBase *ctx) {
}
