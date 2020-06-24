/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge_create.h"
#include "../../util/arr.h"
#include "../../query_ctx.h"
#include "../../schema/schema.h"
#include <assert.h>

/* Forward declarations. */
static Record MergeCreateConsume(OpBase *opBase);
static OpBase *MergeCreateClone(const ExecutionPlan *plan, const OpBase *opBase);
static void MergeCreateFree(OpBase *opBase);

// Convert a graph entity's components into an identifying hash code.
static void _IncrementalHashEntity(XXH64_state_t *state, const char *label,
								   PendingProperties *props) {
	// Update hash with label if one is provided.
	if(label) assert(XXH64_update(state, label, strlen(label)) != XXH_ERROR);

	if(props) {
		// Update hash with attribute count.
		assert(XXH64_update(state, &props->property_count, sizeof(props->property_count)) != XXH_ERROR);
		for(int i = 0; i < props->property_count; i++) {
			// Update hash with attribute ID.
			assert(XXH64_update(state, &props->attr_keys[i], sizeof(props->attr_keys[i])) != XXH_ERROR);

			// Update hash with the hashval of the associated SIValue.
			XXH64_hash_t value_hash = SIValue_HashCode(props->values[i]);
			assert(XXH64_update(state, &value_hash, sizeof(value_hash)) != XXH_ERROR);
		}
	}
}

// Revert the most recent set of buffered creations and free any allocations.
static void _RollbackPendingCreations(OpMergeCreate *op) {
	uint nodes_to_create_count = array_len(op->pending.nodes_to_create);
	for(uint i = 0; i < nodes_to_create_count; i++) {
		array_pop(op->pending.created_nodes);
		PendingProperties *props = array_pop(op->pending.node_properties);
		PendingPropertiesFree(props);
	}

	uint edges_to_create_count = array_len(op->pending.edges_to_create);
	for(uint i = 0; i < edges_to_create_count; i++) {
		array_pop(op->pending.created_edges);
		PendingProperties *props = array_pop(op->pending.edge_properties);
		PendingPropertiesFree(props);
	}
}

OpBase *NewMergeCreateOp(const ExecutionPlan *plan, NodeCreateCtx *nodes, EdgeCreateCtx *edges) {
	OpMergeCreate *op = rm_calloc(1, sizeof(OpMergeCreate));
	op->unique_entities = raxNew();       // Create a map to unique pending creations.
	op->hash_state = XXH64_createState(); // Create a hash state.

	op->pending = NewPendingCreationsContainer(nodes, edges); // Prepare all creation variables.
	op->handoff_mode = false;
	op->records = array_new(Record, 32);

	// Set our Op operations
	OpBase_Init((OpBase *)op, OPType_MERGE_CREATE, "MergeCreate", NULL, MergeCreateConsume,
				NULL, NULL, MergeCreateClone, MergeCreateFree, true, plan);

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

/* Prepare all creations associated with the current Record.
 * Returns false and does not buffer data if every entity to create for this Record
 * has been created in a previous call. */
static bool _CreateEntities(OpMergeCreate *op, Record r) {
	assert(XXH64_reset(op->hash_state, 0) != XXH_ERROR); // Reset hash state

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

		/* Update the hash code with this entity. */
		_IncrementalHashEntity(op->hash_state, n->label, converted_properties);

		/* Save node for later insertion. */
		op->pending.created_nodes = array_append(op->pending.created_nodes, node_ref);

		/* Save properties to insert with node. */
		op->pending.node_properties = array_append(op->pending.node_properties, converted_properties);
	}

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

		/* Update the hash code with this entity, an edge is represented by its
		 * relation, properties and nodes.
		 * Note that unbounded nodes were already presented to the hash.
		 * Incase node has its internal entity set, this means the node has been retrieved from the graph
		 * i.e. bounded node. */
		_IncrementalHashEntity(op->hash_state, e->reltypes[0], converted_properties);
		if(src_node->entity != NULL) {
			EntityID id = ENTITY_GET_ID(src_node);
			void *data = &id;
			size_t len = sizeof(id);
			assert(XXH64_update(op->hash_state, data, len) != XXH_ERROR);
		}
		if(dest_node->entity != NULL) {
			EntityID id = ENTITY_GET_ID(dest_node);
			void *data = &id;
			size_t len = sizeof(id);
			assert(XXH64_update(op->hash_state, data, len) != XXH_ERROR);
		}

		/* Save edge for later insertion. */
		op->pending.created_edges = array_append(op->pending.created_edges, edge_ref);

		/* Save properties to insert with node. */
		op->pending.edge_properties = array_append(op->pending.edge_properties, converted_properties);
	}

	// Finalize the hash value for all processed creations.
	XXH64_hash_t const hash = XXH64_digest(op->hash_state);
	// Check if any creations are unique.
	bool should_create_entities = raxTryInsert(op->unique_entities, (unsigned char *)&hash,
											   sizeof(hash), NULL, NULL);
	// If no entity to be created is unique, roll back all the creations that have just been prepared.
	if(!should_create_entities) _RollbackPendingCreations(op);

	return should_create_entities;
}

// Return mode, emit a populated Record.
static Record _handoff(OpMergeCreate *op) {
	Record r = NULL;
	if(array_len(op->records)) r = array_pop(op->records);
	return r;
}

static Record MergeCreateConsume(OpBase *opBase) {
	OpMergeCreate *op = (OpMergeCreate *)opBase;
	Record r;

	// Return mode, all data was consumed.
	if(op->handoff_mode) return _handoff(op);

	// Consume mode.
	if(!opBase->childCount) {
		// No child operation to call.
		r = OpBase_CreateRecord(opBase);

		/* Buffer all entity creations.
		 * If this operation has no children, it should always have unique creations. */
		assert(_CreateEntities(op, r));

		// Save record for later use.
		op->records = array_append(op->records, r);
	} else {
		// Pull record from child.
		r = OpBase_Consume(opBase->children[0]);
		if(r) {
			/* Create entities. */
			if(_CreateEntities(op, r)) {
				// Save record for later use.
				op->records = array_append(op->records, r);
			} else {
				OpBase_DeleteRecord(r);
			}
		}
	}

	// MergeCreate returns no data while in creation mode.
	return NULL;
}

void MergeCreate_Commit(OpBase *opBase) {
	OpMergeCreate *op = (OpMergeCreate *)opBase;
	op->handoff_mode = true;
	/* Done reading, we're not going to call consume any longer
	 * there might be operations e.g. index scan that need to free
	 * index R/W lock, as such free all execution plan operation up the chain. */
	if(opBase->childCount > 0) OpBase_PropagateFree(opBase->children[0]);
	// Create entities.
	CommitNewEntities(opBase, &op->pending);
}

static OpBase *MergeCreateClone(const ExecutionPlan *plan, const OpBase *opBase) {
	assert(opBase->type == OPType_MERGE_CREATE);
	OpMergeCreate *op = (OpMergeCreate *)opBase;
	NodeCreateCtx *nodes;
	EdgeCreateCtx *edges;
	array_clone_with_cb(nodes, op->pending.nodes_to_create, NodeCreateCtx_Clone);
	array_clone_with_cb(edges, op->pending.edges_to_create, EdgeCreateCtx_Clone);
	return NewMergeCreateOp(plan, nodes, edges);
}

static void MergeCreateFree(OpBase *ctx) {
	OpMergeCreate *op = (OpMergeCreate *)ctx;

	if(op->records) {
		uint rec_count = array_len(op->records);
		for(uint i = 0; i < rec_count; i++) OpBase_DeleteRecord(op->records[i]);
		array_free(op->records);
		op->records = NULL;
	}

	if(op->unique_entities) {
		raxFree(op->unique_entities);
		op->unique_entities = NULL;
	}

	if(op->hash_state) {
		XXH64_freeState(op->hash_state);
		op->hash_state = NULL;
	}

	PendingCreationsFree(&op->pending);
}

