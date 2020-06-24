/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_create.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../schema/schema.h"
#include <assert.h>
#include "../../query_ctx.h"

/* Forward declarations. */
static Record CreateConsume(OpBase *opBase);
static OpBase *CreateClone(const ExecutionPlan *plan, const OpBase *opBase);
static void CreateFree(OpBase *opBase);

OpBase *NewCreateOp(const ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges) {
	OpCreate *op = rm_calloc(1, sizeof(OpCreate));
	op->records = NULL;
	op->pending = NewPendingCreationsContainer(nodes, edges); // Prepare all creation variables.
	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CREATE, "Create", NULL, CreateConsume,
				NULL, NULL, CreateClone, CreateFree, true, plan);

	uint node_blueprint_count = array_len(nodes);
	uint edge_blueprint_count = array_len(edges);

	// Construct the array of IDs this operation modifies
	for(uint i = 0; i < node_blueprint_count; i ++) {
		nodes[i].node_idx = OpBase_Modifies((OpBase *)op, nodes[i].node->alias);
	}
	for(uint i = 0; i < edge_blueprint_count; i ++) {
		edges[i].edge_idx = OpBase_Modifies((OpBase *)op, edges[i].edge->alias);
		assert(OpBase_Aware((OpBase *)op, edges[i].edge->src->alias, &edges[i].src_idx));
		assert(OpBase_Aware((OpBase *)op, edges[i].edge->dest->alias, &edges[i].dest_idx));
	}

	return (OpBase *)op;
}

// Prepare to create all nodes for the current Record.
static void _CreateNodes(OpCreate *op, Record r) {
	uint nodes_to_create_count = array_len(op->pending.nodes_to_create);
	for(uint i = 0; i < nodes_to_create_count; i++) {
		/* Get specified node to create. */
		QGNode *n = op->pending.nodes_to_create[i].node;

		/* Create a new node. */
		Node newNode = {
			.entity = NULL,
			.label = n->label,
			.labelID = n->labelID
		};

		/* Add new node to Record and save a reference to it. */
		Node *node_ref = Record_AddNode(r, op->pending.nodes_to_create[i].node_idx, newNode);

		/* Convert query-level properties. */
		PropertyMap *map = op->pending.nodes_to_create[i].properties;
		PendingProperties *converted_properties = NULL;
		if(map) converted_properties = ConvertPropertyMap(r, map);

		/* Save node for later insertion. */
		op->pending.created_nodes = array_append(op->pending.created_nodes, node_ref);

		/* Save properties to insert with node. */
		op->pending.node_properties = array_append(op->pending.node_properties, converted_properties);
	}
}

// Prepare to create all edges for the current Record.
static void _CreateEdges(OpCreate *op, Record r) {
	uint edges_to_create_count = array_len(op->pending.edges_to_create);
	for(uint i = 0; i < edges_to_create_count; i++) {
		/* Get specified edge to create. */
		QGEdge *e = op->pending.edges_to_create[i].edge;

		/* Retrieve source and dest nodes. */
		Node *src_node = Record_GetNode(r, op->pending.edges_to_create[i].src_idx);
		Node *dest_node = Record_GetNode(r, op->pending.edges_to_create[i].dest_idx);
		/* Verify that the endpoints of the new edge resolved properly; fail otherwise. */
		if(!src_node || !dest_node) {
			QueryCtx_SetError("Failed to create relationship; endpoint was not found.");
			QueryCtx_RaiseRuntimeException();
		}

		/* Create the actual edge. */
		Edge newEdge = {0};
		if(array_len(e->reltypes) > 0) newEdge.relationship = e->reltypes[0];
		Edge_SetSrcNode(&newEdge, src_node);
		Edge_SetDestNode(&newEdge, dest_node);
		Edge *edge_ref = Record_AddEdge(r, op->pending.edges_to_create[i].edge_idx, newEdge);

		/* Convert query-level properties. */
		PropertyMap *map = op->pending.edges_to_create[i].properties;
		PendingProperties *converted_properties = NULL;
		if(map) converted_properties = ConvertPropertyMap(r, map);

		/* Save edge for later insertion. */
		op->pending.created_edges = array_append(op->pending.created_edges, edge_ref);

		/* Save properties to insert with node. */
		op->pending.edge_properties = array_append(op->pending.edge_properties, converted_properties);
	}
}

// Return mode, emit a populated Record.
static Record _handoff(OpCreate *op) {
	Record r = NULL;
	if(array_len(op->records)) r = array_pop(op->records);
	return r;
}

static Record CreateConsume(OpBase *opBase) {
	OpCreate *op = (OpCreate *)opBase;
	Record r;

	// Return mode, all data was consumed.
	if(op->records) return _handoff(op);

	// Consume mode.
	op->records = array_new(Record, 32);

	OpBase *child = NULL;
	if(!op->op.childCount) {
		// No child operation to call.
		r = OpBase_CreateRecord(opBase);
		/* Create entities. */
		_CreateNodes(op, r);
		_CreateEdges(op, r);

		// Save record for later use.
		op->records = array_append(op->records, r);
	} else {
		// Pull data until child is depleted.
		child = op->op.children[0];
		while((r = OpBase_Consume(child))) {
			/* Create entities. */
			_CreateNodes(op, r);
			_CreateEdges(op, r);

			// Save record for later use.
			op->records = array_append(op->records, r);
		}
	}

	/* Done reading, we're not going to call consume any longer
	 * there might be operations e.g. index scan that need to free
	 * index R/W lock, as such free all execution plan operation up the chain. */
	if(child) OpBase_PropagateFree(child);

	// Create entities.
	CommitNewEntities(opBase, &op->pending);

	// Return record.
	return _handoff(op);
}

static OpBase *CreateClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_CREATE);
	OpCreate *op = (OpCreate *)opBase;
	NodeCreateCtx *nodes;
	EdgeCreateCtx *edges;
	array_clone_with_cb(nodes, op->pending.nodes_to_create, NodeCreateCtx_Clone);
	array_clone_with_cb(edges, op->pending.edges_to_create, EdgeCreateCtx_Clone);
	return NewCreateOp(plan, nodes, edges);
}

static void CreateFree(OpBase *ctx) {
	OpCreate *op = (OpCreate *)ctx;

	if(op->records) {
		uint rec_count = array_len(op->records);
		for(uint i = 0; i < rec_count; i++) OpBase_DeleteRecord(op->records[i]);
		array_free(op->records);
		op->records = NULL;
	}

	PendingCreationsFree(&op->pending);
}

