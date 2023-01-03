/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "op_create.h"
#include "RG.h"
#include "../../errors.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"

/* Forward declarations. */
static Record CreateConsume(OpBase *opBase);
static OpBase *CreateClone(const ExecutionPlan *plan, const OpBase *opBase);
static void CreateFree(OpBase *opBase);

OpBase *NewCreateOp(const ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges) {
	OpCreate *op = rm_calloc(1, sizeof(OpCreate));
	op->records = NULL;
	NewPendingCreationsContainer(&op->pending, nodes, edges); // Prepare all creation variables.
	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_CREATE, "Create", NULL, CreateConsume,
				NULL, NULL, CreateClone, CreateFree, true, plan);

	uint node_blueprint_count = array_len(nodes);
	uint edge_blueprint_count = array_len(edges);

	// Construct the array of IDs this operation modifies
	for(uint i = 0; i < node_blueprint_count; i ++) {
		NodeCreateCtx *n = nodes + i;
		n->node_idx = OpBase_Modifies((OpBase *)op, n->alias);
	}
	for(uint i = 0; i < edge_blueprint_count; i ++) {
		EdgeCreateCtx *e = edges + i;
		e->edge_idx = OpBase_Modifies((OpBase *)op, e->alias);
		bool aware;
		UNUSED(aware);
		aware = OpBase_Aware((OpBase *)op, e->src, &e->src_idx);
		ASSERT(aware == true);
		aware = OpBase_Aware((OpBase *)op, e->dest, &e->dest_idx);
		ASSERT(aware == true);
	}

	return (OpBase *)op;
}

// Prepare to create all nodes for the current Record.
static void _CreateNodes(OpCreate *op, Record r, GraphContext *gc) {
	uint nodes_to_create_count = array_len(op->pending.nodes_to_create);
	for(uint i = 0; i < nodes_to_create_count; i++) {
		// get specified node to create
		NodeCreateCtx *n = op->pending.nodes_to_create + i;

		// create a new node
		Node newNode = GE_NEW_NODE();

		// add new node to Record and save a reference to it
		Node *node_ref = Record_AddNode(r, op->pending.nodes_to_create[i].node_idx, newNode);

		// convert query-level properties
		AttributeSet converted_attr = NULL;
		PropertyMap *map = op->pending.nodes_to_create[i].properties;
		if(map != NULL) {
			ConvertPropertyMap(gc, &converted_attr, r, map, false);
		}

		// save node for later insertion
		array_append(op->pending.created_nodes, node_ref);

		// save attributes to insert with node
		array_append(op->pending.node_attributes, converted_attr);

		// save labels to assigned to node
		array_append(op->pending.node_labels, n->labelsId);
	}
}

// Prepare to create all edges for the current Record.
static void _CreateEdges(OpCreate *op, Record r, GraphContext *gc) {
	uint edges_to_create_count = array_len(op->pending.edges_to_create);
	for(uint i = 0; i < edges_to_create_count; i++) {
		// get specified edge to create
		EdgeCreateCtx *e = op->pending.edges_to_create + i;

		// retrieve source and dest nodes
		Node *src_node = Record_GetNode(r, e->src_idx);
		Node *dest_node = Record_GetNode(r, e->dest_idx);
		// verify that the endpoints of the new edge resolved properly; fail otherwise
		if(!src_node || !dest_node) {
			ErrorCtx_RaiseRuntimeException("Failed to create relationship; endpoint was not found.");
		}

		// create the actual edge
		Edge newEdge = {0};
		newEdge.relationship = e->relation;
		Edge_SetSrcNode(&newEdge, src_node);
		Edge_SetDestNode(&newEdge, dest_node);
		Edge *edge_ref = Record_AddEdge(r, e->edge_idx, newEdge);

		// convert query-level properties
		PropertyMap *map = op->pending.edges_to_create[i].properties;
		AttributeSet converted_attr = NULL;
		if(map != NULL) {
			ConvertPropertyMap(gc, &converted_attr, r, map, false);
		}

		// save edge for later insertion
		array_append(op->pending.created_edges, edge_ref);

		// save attributes to insert with node
		array_append(op->pending.edge_attributes, converted_attr);
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

	GraphContext *gc = QueryCtx_GetGraphCtx();
	OpBase *child = NULL;
	if(!op->op.childCount) {
		// No child operation to call.
		r = OpBase_CreateRecord(opBase);
		/* Create entities. */
		_CreateNodes(op, r, gc);
		_CreateEdges(op, r, gc);

		// Save record for later use.
		array_append(op->records, r);
	} else {
		// Pull data until child is depleted.
		child = op->op.children[0];
		while((r = OpBase_Consume(child))) {
			/* Persist scalars from previous ops before storing the record,
			 * as those ops will be freed before the records are handed off. */
			Record_PersistScalars(r);

			// create entities
			_CreateNodes(op, r, gc);
			_CreateEdges(op, r, gc);

			// save record for later use
			array_append(op->records, r);
		}
	}

	/* Done reading, we're not going to call consume any longer
	 * there might be operations e.g. index scan that need to free
	 * index R/W lock, as such free all execution plan operation up the chain. */
	if(child) OpBase_PropagateReset(child);

	// Create entities.
	CommitNewEntities(opBase, &op->pending);

	// Return record.
	return _handoff(op);
}

static OpBase *CreateClone(const ExecutionPlan *plan, const OpBase *opBase) {
	ASSERT(opBase->type == OPType_CREATE);
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

