/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_create.h"
#include "RG.h"
#include "../../../../errors.h"
#include "../../../../util/arr.h"
#include "../../../../query_ctx.h"

/* Forward declarations. */
static Record CreateConsume(RT_OpBase *opBase);
static RT_OpBase *CreateClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void CreateFree(RT_OpBase *opBase);

RT_OpBase *RT_NewCreateOp(const RT_ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges) {
	RT_OpCreate *op = rm_calloc(1, sizeof(RT_OpCreate));
	op->records = NULL;
	op->pending = NewPendingCreationsContainer(nodes, edges); // Prepare all creation variables.
	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_CREATE, NULL, CreateConsume,
				NULL, CreateClone, CreateFree, true, plan);

	return (RT_OpBase *)op;
}

// Prepare to create all nodes for the current Record.
static void _CreateNodes(RT_OpCreate *op, Record r) {
	uint nodes_to_create_count = array_len(op->pending.nodes_to_create);
	for(uint i = 0; i < nodes_to_create_count; i++) {
		// get specified node to create
		NodeCreateCtx *n = op->pending.nodes_to_create + i;

		// create a new node
		Node newNode = GE_NEW_LABELED_NODE(n->label, n->labelId);

		// add new node to Record and save a reference to it
		Node *node_ref = Record_AddNode(r, op->pending.nodes_to_create[i].node_idx, newNode);

		// convert query-level properties
		PropertyMap *map = op->pending.nodes_to_create[i].properties;
		PendingProperties *converted_properties = NULL;
		if(map) converted_properties = ConvertPropertyMap(r, map, false);

		// save node for later insertion
		array_append(op->pending.created_nodes, node_ref);

		// save properties to insert with node
		array_append(op->pending.node_properties, converted_properties);
	}
}

// Prepare to create all edges for the current Record.
static void _CreateEdges(RT_OpCreate *op, Record r) {
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
		PendingProperties *converted_properties = NULL;
		if(map) converted_properties = ConvertPropertyMap(r, map, false);

		// save edge for later insertion
		array_append(op->pending.created_edges, edge_ref);

		// save properties to insert with node
		array_append(op->pending.edge_properties, converted_properties);
	}
}

// Return mode, emit a populated Record.
static Record _handoff(RT_OpCreate *op) {
	Record r = NULL;
	if(array_len(op->records)) r = array_pop(op->records);
	return r;
}

static Record CreateConsume(RT_OpBase *opBase) {
	RT_OpCreate *op = (RT_OpCreate *)opBase;
	Record r;

	// Return mode, all data was consumed.
	if(op->records) return _handoff(op);

	// Consume mode.
	op->records = array_new(Record, 32);

	RT_OpBase *child = NULL;
	if(!op->op.childCount) {
		// No child operation to call.
		r = RT_OpBase_CreateRecord(opBase);
		/* Create entities. */
		_CreateNodes(op, r);
		_CreateEdges(op, r);

		// Save record for later use.
		array_append(op->records, r);
	} else {
		// Pull data until child is depleted.
		child = op->op.children[0];
		while((r = RT_OpBase_Consume(child))) {
			/* Persist scalars from previous ops before storing the record,
			 * as those ops will be freed before the records are handed off. */
			Record_PersistScalars(r);

			// create entities
			_CreateNodes(op, r);
			_CreateEdges(op, r);

			// save record for later use
			array_append(op->records, r);
		}
	}

	/* Done reading, we're not going to call consume any longer
	 * there might be operations e.g. index scan that need to free
	 * index R/W lock, as such free all execution plan operation up the chain. */
	if(child) RT_OpBase_PropagateFree(child);

	// Create entities.
	CommitNewEntities(opBase, &op->pending);

	// Return record.
	return _handoff(op);
}

static RT_OpBase *CreateClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_CREATE);
	RT_OpCreate *op = (RT_OpCreate *)opBase;
	NodeCreateCtx *nodes;
	EdgeCreateCtx *edges;
	array_clone_with_cb(nodes, op->pending.nodes_to_create, NodeCreateCtx_Clone);
	array_clone_with_cb(edges, op->pending.edges_to_create, EdgeCreateCtx_Clone);
	return RT_NewCreateOp(plan, nodes, edges);
}

static void CreateFree(RT_OpBase *ctx) {
	RT_OpCreate *op = (RT_OpCreate *)ctx;

	if(op->records) {
		uint rec_count = array_len(op->records);
		for(uint i = 0; i < rec_count; i++) RT_OpBase_DeleteRecord(op->records[i]);
		array_free(op->records);
		op->records = NULL;
	}

	PendingCreationsFree(&op->pending);
}
