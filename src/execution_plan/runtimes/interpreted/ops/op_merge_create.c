/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "op_merge_create.h"
#include "../../../../errors.h"
#include "../../../../util/arr.h"
#include "../../../../query_ctx.h"

/* Forward declarations. */
static Record MergeCreateConsume(RT_OpBase *opBase);
static RT_OpBase *MergeCreateClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase);
static void MergeCreateFree(RT_OpBase *opBase);

// Convert a graph entity's components into an identifying hash code.
static void _IncrementalHashEntity(XXH64_state_t *state, const char *label,
								   PendingProperties *props) {
	// Update hash with label if one is provided.
	XXH_errorcode res;
	UNUSED(res);
	if(label) {
		res = XXH64_update(state, label, strlen(label));
		ASSERT(res != XXH_ERROR);
	}

	if(props) {
		// Update hash with attribute count.
		res = XXH64_update(state, &props->property_count, sizeof(props->property_count));
		ASSERT(res != XXH_ERROR);
		for(int i = 0; i < props->property_count; i++) {
			// Update hash with attribute ID.
			res = XXH64_update(state, &props->attr_keys[i], sizeof(props->attr_keys[i]));
			ASSERT(res != XXH_ERROR);

			// Update hash with the hashval of the associated SIValue.
			XXH64_hash_t value_hash = SIValue_HashCode(props->values[i]);
			res = XXH64_update(state, &value_hash, sizeof(value_hash));
			ASSERT(res != XXH_ERROR);
		}
	}
}

// Revert the most recent set of buffered creations and free any allocations.
static void _RollbackPendingCreations(RT_OpMergeCreate *op) {
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

RT_OpBase *RT_NewMergeCreateOp(const RT_ExecutionPlan *plan, const OpMergeCreate *op_desc) {
	RT_OpMergeCreate *op = rm_calloc(1, sizeof(RT_OpMergeCreate));
	op->op_desc = op_desc;
	op->unique_entities = raxNew();       // Create a map to unique pending creations.
	op->hash_state = XXH64_createState(); // Create a hash state.
	NodeCreateCtx *nodes;
	EdgeCreateCtx *edges;
	array_clone_with_cb(nodes, op_desc->nodes, NodeCreateCtx_Clone);
	array_clone_with_cb(edges, op_desc->edges, EdgeCreateCtx_Clone);
	op->pending = NewPendingCreationsContainer(nodes, edges); // Prepare all creation variables.
	op->handoff_mode = false;
	op->records = array_new(Record, 32);

	// Set our Op operations
	RT_OpBase_Init((RT_OpBase *)op, OPType_MERGE_CREATE, NULL, MergeCreateConsume,
				NULL, MergeCreateClone, MergeCreateFree, true, plan);

	return (RT_OpBase *)op;
}

/* Prepare all creations associated with the current Record.
 * Returns false and does not buffer data if every entity to create for this Record
 * has been created in a previous call. */
static bool _CreateEntities(RT_OpMergeCreate *op, Record r) {
	XXH_errorcode res = XXH64_reset(op->hash_state, 0); // Reset hash state
	UNUSED(res);
	ASSERT(res != XXH_ERROR);

	uint nodes_to_create_count = array_len(op->pending.nodes_to_create);
	for(uint i = 0; i < nodes_to_create_count; i++) {
		/* Get specified node to create. */
		NodeCreateCtx *n = op->pending.nodes_to_create + i;

		/* Create a new node. */
		Node newNode = GE_NEW_LABELED_NODE(n->label, n->labelId);

		/* Add new node to Record and save a reference to it. */
		Node *node_ref = Record_AddNode(r, n->node_idx, newNode);

		/* Convert query-level properties. */
		PropertyMap *map = n->properties;
		PendingProperties *converted_properties = NULL;
		if(map) converted_properties = ConvertPropertyMap(r, map, true);

		/* Update the hash code with this entity. */
		_IncrementalHashEntity(op->hash_state, n->label, converted_properties);

		/* Save node for later insertion. */
		array_append(op->pending.created_nodes, node_ref);

		/* Save properties to insert with node. */
		array_append(op->pending.node_properties, converted_properties);
	}

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
		PropertyMap *map = e->properties;
		PendingProperties *converted_properties = NULL;
		if(map) converted_properties = ConvertPropertyMap(r, map, true);

		/* Update the hash code with this entity, an edge is represented by its
		 * relation, properties and nodes.
		 * Note that unbounded nodes were already presented to the hash.
		 * Incase node has its internal entity set, this means the node has been retrieved from the graph
		 * i.e. bounded node. */
		_IncrementalHashEntity(op->hash_state, e->relation, converted_properties);
		if(src_node->entity != NULL) {
			EntityID id = ENTITY_GET_ID(src_node);
			void *data = &id;
			size_t len = sizeof(id);
			res = XXH64_update(op->hash_state, data, len);
			ASSERT(res != XXH_ERROR);
		}
		if(dest_node->entity != NULL) {
			EntityID id = ENTITY_GET_ID(dest_node);
			void *data = &id;
			size_t len = sizeof(id);
			res = XXH64_update(op->hash_state, data, len);
			ASSERT(res != XXH_ERROR);
		}

		/* Save edge for later insertion. */
		array_append(op->pending.created_edges, edge_ref);

		/* Save properties to insert with node. */
		array_append(op->pending.edge_properties, converted_properties);
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
static Record _handoff(RT_OpMergeCreate *op) {
	Record r = NULL;
	if(array_len(op->records)) r = array_pop(op->records);
	return r;
}

static Record MergeCreateConsume(RT_OpBase *opBase) {
	RT_OpMergeCreate *op = (RT_OpMergeCreate *)opBase;
	Record r;

	// Return mode, all data was consumed.
	if(op->handoff_mode) return _handoff(op);

	// Consume mode.
	if(!opBase->childCount) {
		// No child operation to call.
		r = RT_OpBase_CreateRecord(opBase);

		/* Buffer all entity creations.
		 * If this operation has no children, it should always have unique creations. */
		bool entities_created = _CreateEntities(op, r);
		ASSERT(entities_created == true);

		// Save record for later use.
		array_append(op->records, r);
	} else {
		// Pull record from child.
		r = RT_OpBase_Consume(opBase->children[0]);
		if(r) {
			/* Create entities. */
			if(_CreateEntities(op, r)) {
				// Save record for later use.
				array_append(op->records, r);
			} else {
				RT_OpBase_DeleteRecord(r);
			}
		}
	}

	// MergeCreate returns no data while in creation mode.
	return NULL;
}

void MergeCreate_Commit(RT_OpBase *opBase) {
	RT_OpMergeCreate *op = (RT_OpMergeCreate *)opBase;
	op->handoff_mode = true;
	/* Done reading, we're not going to call consume any longer
	 * there might be operations e.g. index scan that need to free
	 * index R/W lock, as such free all execution plan operation up the chain. */
	if(opBase->childCount > 0) RT_OpBase_PropagateFree(opBase->children[0]);
	// Create entities.
	CommitNewEntities(opBase, &op->pending);
}

static RT_OpBase *MergeCreateClone(const RT_ExecutionPlan *plan, const RT_OpBase *opBase) {
	ASSERT(opBase->type == OPType_MERGE_CREATE);
	RT_OpMergeCreate *op = (RT_OpMergeCreate *)opBase;
	return RT_NewMergeCreateOp(plan, op->op_desc);
}

static void MergeCreateFree(RT_OpBase *ctx) {
	RT_OpMergeCreate *op = (RT_OpMergeCreate *)ctx;

	if(op->records) {
		uint rec_count = array_len(op->records);
		for(uint i = 0; i < rec_count; i++) RT_OpBase_DeleteRecord(op->records[i]);
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
