/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "update_functions.h"
#include "../../../errors.h"
#include "../../../query_ctx.h"
#include "../../../datatypes/map.h"

/* set a property on a graph entity
 * for non-NULL values, the property will be added or updated
 * if it is already present
 * for NULL values, the property will be deleted if present
 * and nothing will be done otherwise
 * returns 1 if a property was set or deleted */
static int _UpdateEntity(PendingUpdateCtx *update) {
	int           res        =  0;
	GraphEntity   *ge        =  update->ge;
	Attribute_ID  attr_id    =  update->attr_id;
	SIValue       new_value  =  update->new_value;

	// handle the case in which we are deleting all properties
	if(attr_id == ATTRIBUTE_ALL) return GraphEntity_ClearProperties(ge);

	// try to get current property value
	SIValue *old_value = GraphEntity_GetProperty(ge, attr_id);

	if(old_value == PROPERTY_NOTFOUND) {
		// adding a new property; do nothing if its value is NULL
		if(SI_TYPE(new_value) != T_NULL) {
			res = GraphEntity_AddProperty(ge, attr_id, new_value);
		}
	} else {
		// update property
		res = GraphEntity_SetProperty(ge, attr_id, new_value);
	}

	SIValue_Free(new_value);
	return res;
}

static PendingUpdateCtx _PreparePendingUpdate(GraphContext *gc, SIType accepted_properties,
											  int label_id, GraphEntity *entity,
											  Attribute_ID attr_id, SIValue new_value) {
	//--------------------------------------------------------------------------
	// validate value type
	//--------------------------------------------------------------------------

	// emit an error and exit if we're trying to add an invalid type
	if(!(SI_TYPE(new_value) & accepted_properties)) {
		Error_InvalidPropertyValue();
		ErrorCtx_RaiseRuntimeException(NULL);
		return (PendingUpdateCtx) {};
	}

	bool update_index = false;
	// determine whether we must update the index for this update
	if(label_id != GRAPH_NO_LABEL) {
		// if the (label:attribute) combination has an index, take note
		update_index = GraphContext_GetIndexByID(gc, label_id, &attr_id,
												 IDX_ANY) != NULL;
	}

	return (PendingUpdateCtx) {
		.ge            =  entity,
		.attr_id       =  attr_id,
		.label_id      =  label_id,
		.new_value     =  new_value,
		.update_index  =  update_index,
	};
}

// commits delayed updates
void CommitUpdates(GraphContext *gc, ResultSetStatistics *stats,
				   PendingUpdateCtx *updates) {
	ASSERT(gc != NULL);
	ASSERT(stats != NULL);

	uint    properties_set  =  0;
	Schema  *s              =  NULL;
	bool    reindex         =  false;
	uint    update_count    =  array_len(updates);

	// return early if no updates are enqueued
	if(update_count == 0) return;

	uint i = 0;
	GraphEntity *ge = updates[0].ge;

	for(; i < update_count; i++) {
		PendingUpdateCtx *update = updates + i;

		// following updates apply to a new graph entity
		// index previous entity if we're required to
		if(ge != updates[i].ge) {
			if(reindex) {
				s = GraphContext_GetSchemaByID(gc, updates[i - 1].label_id,
											   SCHEMA_NODE);
				ASSERT(s != NULL);
				// introduce updated entity to index
				Schema_AddNodeToIndices(s, (Node *)ge);
			}

			// update state
			reindex  =  false;
			ge       =  updates[i].ge;
		}

		// if entity has been deleted, perform no updates
		if(GraphEntity_IsDeleted(updates[i].ge)) continue;

		// update the property on the graph entity
		int updated = _UpdateEntity(update);
		properties_set += updated;
		// reindex only if update performed
		reindex |= update->update_index & (bool)updated;
	}

	// handle last updated entity
	if(reindex) {
		s = GraphContext_GetSchemaByID(gc, updates[i - 1].label_id, SCHEMA_NODE);
		ASSERT(s != NULL);
		// introduce updated entity to index
		Schema_AddNodeToIndices(s, (Node *)ge);
	}

	if(stats) stats->properties_set += properties_set;
}

void EvalEntityUpdates(GraphContext *gc, PendingUpdateCtx **updates,
					   const Record r, const EntityUpdateEvalCtx *ctx, bool allow_null) {
	Schema *s         = NULL;
	int label_id      = GRAPH_NO_LABEL;
	bool node_update  = false;

	//--------------------------------------------------------------------------
	// validate entity type
	//--------------------------------------------------------------------------

	// get the type of the entity to update
	// if the expected entity was not found, make no updates but do not error
	RecordEntryType t = Record_GetType(r, ctx->record_idx);
	if(t == REC_TYPE_UNKNOWN) return;

	// make sure we're updating either a node or an edge
	if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) {
		ErrorCtx_RaiseRuntimeException(
			"Update error: alias '%s' did not resolve to a graph entity",
			ctx->alias);
	}

	GraphEntity *entity = Record_GetGraphEntity(r, ctx->record_idx);
	node_update = (t == REC_TYPE_NODE);

	// if the entity is a node
	if(node_update) {
		Node *n = (Node *)entity;
		// retrieve the node's Label ID from a local member or the graph
		label_id = NODE_GET_LABEL_ID(n, gc->g);
	}

	// if this update replaces all existing properties
	// enqueue a clear update to do so
	if(ctx->mode == UPDATE_REPLACE) {
		PendingUpdateCtx update = {
			.ge            = entity,
			.label_id      = label_id,
			.update_index  = (label_id != GRAPH_NO_LABEL),
			.attr_id       = ATTRIBUTE_ALL,
		};
		*updates = array_append(*updates, update);
	}

	// if we're converting a SET clause, NULL is acceptable
	// as it indicates a deletion
	SIType accepted_properties = SI_VALID_PROPERTY_VALUE;
	if(allow_null) accepted_properties |= T_NULL;

	uint exp_count = array_len(ctx->properties);

	//--------------------------------------------------------------------------
	// enqueue update
	//--------------------------------------------------------------------------

	for(uint i = 0; i < exp_count; i++) {
		PendingUpdateCtx  update;
		PropertySetCtx    property   =  ctx->properties[i];
		Attribute_ID      attr_id    =  property.id;
		SIValue           new_value  =  AR_EXP_Evaluate(property.exp,  r);

		// value is of type map e.g. n.v = {a:1, b:2}
		if(SI_TYPE(new_value) == T_MAP) {
			SIValue m = new_value;
			ASSERT(attr_id == ATTRIBUTE_ALL);
			// iterate over all map elements to build updates
			uint map_size = Map_KeyCount(m);
			for(uint j = 0; j < map_size; j ++) {
				SIValue key;
				SIValue value;
				Map_GetIdx(m, j, &key, &value);
				Attribute_ID attr_id = GraphContext_FindOrAddAttribute(gc,
																	   key.stringval);

				update = _PreparePendingUpdate(gc, accepted_properties,
											   label_id, entity, attr_id, value);
				// enqueue the current update
				*updates = array_append(*updates, update);
			}
			continue;
		}

		update = _PreparePendingUpdate(gc, accepted_properties, label_id,
									   entity, attr_id, new_value);
		// enqueue the current update
		*updates = array_append(*updates, update);
	}
}

