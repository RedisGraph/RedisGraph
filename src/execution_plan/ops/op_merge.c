/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge.h"

#include "../../schema/schema.h"
#include "../../arithmetic/arithmetic_expression.h"
#include <assert.h>

/* Forward declarations */
static void Free(OpBase *ctx);
static OpResult Reset(OpBase *ctx);
static OpResult Init(OpBase *opBase);
static Record Consume(OpBase *opBase);

// TODO These two functions are duplicates of op_create functions
// TODO don't need two functions
static void _AddNodeProperties(OpMerge *op, Schema *schema, Node *n, PropertyMap *props) {
	if(props == NULL) return;

	for(int i = 0; i < props->property_count; i++) {
		Attribute_ID prop_id = GraphContext_FindOrAddAttribute(op->gc, props->keys[i]);
		GraphEntity_AddProperty((GraphEntity *)n, prop_id, props->values[i]);
	}

	if(op->stats) op->stats->properties_set += props->property_count;
}

static void _AddEdgeProperties(OpMerge *op, Schema *schema, Edge *e, PropertyMap *props) {
	if(props == NULL) return;

	for(int i = 0; i < props->property_count; i++) {
		Attribute_ID prop_id = GraphContext_FindOrAddAttribute(op->gc, props->keys[i]);
		GraphEntity_AddProperty((GraphEntity *)e, prop_id, props->values[i]);
	}

	if(op->stats) op->stats->properties_set += props->property_count;
}

/* Saves every entity within the query graph into the actual graph.
 * update statistics regarding the number of entities create and properties set. */
static void _CommitNodes(OpMerge *op, Record r) {
	// int labelID;
	// Graph *g = op->gc->g;

	// uint node_count = array_len(op->nodes_to_merge);

	// // Start by creating nodes.
	// Graph_AllocateNodes(g, node_count);

	// for(uint i = 0; i < node_count; i ++) {
	// 	NodeCreateCtx *node_ctx = &op->nodes_to_merge[i];
	// 	// Get blueprint of node to create
	// 	QGNode *node_blueprint = node_ctx->node;

	// 	// Newly created node will be placed within given record.
	// 	Node *created_node = Record_GetNode(r, node_ctx->node_idx);

	// 	Schema *schema = NULL;

	// 	// Set, create label.
	// 	if(node_blueprint->label == NULL) {
	// 		labelID = GRAPH_NO_LABEL;
	// 	} else {
	// 		schema = GraphContext_GetSchema(op->gc, node_blueprint->label, SCHEMA_NODE);
	// 		/* This is the first time we've encountered this label; create its schema */
	// 		if(schema == NULL) {
	// 			schema = GraphContext_AddSchema(op->gc, node_blueprint->label, SCHEMA_NODE);
	// 			op->stats->labels_added++;
	// 		}
	// 		labelID = schema->id;
	// 	}

	// 	Graph_CreateNode(g, labelID, created_node);

	// 	_AddNodeProperties(op, schema, created_node, node_ctx->properties);

	// 	if(schema) Schema_AddNodeToIndices(schema, created_node, false);
	// }

	// op->stats->nodes_created += node_count;
}

static void _CommitEdges(OpMerge *op, Record r) {
	// Create edges.

	// uint edge_count = array_len(op->edges_to_merge);
	// // TODO allocate? nodes get allocated here
	// for(uint i = 0; i < edge_count; i ++) {
	// 	EdgeCreateCtx *edge_ctx = &op->edges_to_merge[i];
	// 	// Get blueprint of edge to create
	// 	QGEdge *edge_blueprint = edge_ctx->edge;

	// 	// Newly created edge will be placed within given record.
	// 	Edge *created_edge = Record_GetEdge(r, edge_ctx->edge_idx);

	// 	// An edge must have exactly 1 relationship type.
	// 	Schema *schema = GraphContext_GetSchema(op->gc, edge_blueprint->reltypes[0], SCHEMA_EDGE);
	// 	if(!schema) schema = GraphContext_AddSchema(op->gc, edge_blueprint->reltypes[0], SCHEMA_EDGE);

	// 	// Node are already created, get them from record.
	// 	EntityID srcId = ENTITY_GET_ID(Record_GetNode(r, edge_ctx->src_idx));
	// 	EntityID destId = ENTITY_GET_ID(Record_GetNode(r, edge_ctx->dest_idx));

	// 	assert(Graph_ConnectNodes(op->gc->g, srcId, destId, schema->id, created_edge));

	// 	_AddEdgeProperties(op, schema, created_edge, edge_ctx->properties);
	// }

	// if(op->stats) op->stats->relationships_created += edge_count;
}

static void _CreateEntities(OpMerge *op, Record r) {
	// Lock everything.
	// Graph_AcquireWriteLock(op->gc->g);

	// // Commit query graph and set resultset statistics.
	// _CommitNodes(op, r);
	// _CommitEdges(op, r);

	// // Release lock.
	// Graph_ReleaseLock(op->gc->g);
}

static void _PrepareMergeContext(OpMerge *op, const AST *ast, QueryGraph *qg) {
	// const cypher_astnode_t *path = cypher_ast_merge_get_pattern_path(merge_clause);

	// uint entity_count = cypher_ast_pattern_path_nelements(path);

	// NodeCreateCtx *nodes_to_merge = array_new(NodeCreateCtx, (entity_count / 2) + 1);
	// EdgeCreateCtx *edges_to_merge = array_new(EdgeCreateCtx, entity_count / 2);

	// for(uint i = 0; i < entity_count; i ++) {
	// 	const cypher_astnode_t *elem = cypher_ast_pattern_path_get_element(path, i);
	// 	if(i % 2) {  // Entity is a relationship
	// 		EdgeCreateCtx new_edge = _NewEdgeCreateCtx(qg, path, i);
	// 		edges_to_merge = array_append(edges_to_merge, new_edge);
	// 	} else {
	// 		// Entity is a node
	// 		NodeCreateCtx new_node = _NewNodeCreateCtx(ast, qg, elem);
	// 		nodes_to_merge = array_append(nodes_to_merge, new_node);
	// 	}
	// }

	// AST_MergeContext ctx = { .nodes_to_merge = nodes_to_merge, .edges_to_merge = edges_to_merge };
	// return ctx;
}

OpBase *NewMergeOp(const ExecutionPlan *plan, ResultSetStatistics *stats, const AST *ast) {
	OpMerge *op = malloc(sizeof(OpMerge));
	op->stats = stats;
	op->gc = GraphContext_GetFromTLS();
	op->matched = false;
	op->created = false;

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_MERGE, "Merge", Init, Consume, Reset, NULL, Free, plan);
	return (OpBase *)op;
}

static OpResult Init(OpBase *opBase) {
	return OP_OK;
}

static Record Consume(OpBase *opBase) {
	// OpMerge *op = (OpMerge *)opBase;

	// /* Pattern was created in the previous call
	//  * Execution plan is already depleted. */
	// if(op->created) return NULL;

	// OpBase *child = op->op.children[0];
	// Record r = OpBase_Consume(child);
	// if(r) {
	// 	/* If we're here that means pattern was matched!
	// 	* in that case there's no need to create any graph entity,
	// 	* we can simply return. */
	// 	op->matched = true;
	// } else {
	// 	/* In case there a previous match, execution plan
	// 	 * is simply depleted, no need to create the pattern. */
	// 	if(op->matched) return r;

	// 	// No previous match, create MERGE pattern.

	// 	/* TODO: once MATCH and MERGE will be mixed
	// 	 * we'll need to apply a similar strategy applied by op_create. */

	// 	/* Done reading, we're not going to call consume any longer
	// 	 * there might be operations e.g. index scan that need to free
	// 	 * index R/W lock, as such free all execution plan operation up the chain. */
	// 	OpBase_PropagateFree(child);

	// 	r = Record_New(opBase->record_map->record_len);
	// 	_CreateEntities(op, r);
	// 	op->created = true;
	// }

	// return r;
	return NULL;
}

static OpResult Reset(OpBase *ctx) {
	// Merge doesn't modify anything.
	return OP_OK;
}

static void Free(OpBase *ctx) {
}
