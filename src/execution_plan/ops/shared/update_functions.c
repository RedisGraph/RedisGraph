/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "update_functions.h"
#include "../../../undo_log/undo_log.h"
#include "../../../errors.h"
#include "../../../query_ctx.h"

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

// commits delayed updates
void CommitUpdates(GraphContext *gc, ResultSetStatistics *stats,
		PendingUpdateCtx *updates, bool is_rollback, struct UndoLog *undo_log) {
	ASSERT(gc != NULL);
	ASSERT(stats != NULL);
	ASSERT(updates != NULL);

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

		if(likely(!is_rollback)) {
			// Inc the update counter in the undo_log
			//UndoLog_UpdateCommitted(undo_log, 1);
		}

		// following updates apply to a new graph entity
		// index previous entity if we're required to
		if(ge != updates[i].ge) {
			if(reindex) {
				s = GraphContext_GetSchemaByID(gc, updates[i-1].label_id,
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
		s = GraphContext_GetSchemaByID(gc, updates[i-1].label_id, SCHEMA_NODE);
		ASSERT(s != NULL);
		// introduce updated entity to index
		Schema_AddNodeToIndices(s, (Node *)ge);
	}

	if(stats) {
		if(unlikely(is_rollback)) { // On rollback need to rollback the stats changes also
			stats->properties_set -= properties_set;
		} else {
			stats->properties_set += properties_set;
		}
	}
}

void EvalEntityUpdates(GraphContext *gc, PendingUpdateCtx **updates, const Record r,
					   const EntityUpdateEvalCtx *ctx, bool allow_null, UndoLog *undo_log) {
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
			.ge            =  entity,
			.label_id      =  label_id,
			.update_index  =  (label_id != GRAPH_NO_LABEL),
			.attr_id       =  ATTRIBUTE_ALL,
		};
		*updates = array_append(*updates, update);
	}

	// if we're converting a SET clause, NULL is acceptable
	// as it indicates a deletion
	SIType accepted_properties = SI_VALID_PROPERTY_VALUE;
	if(allow_null) accepted_properties |= T_NULL;

	uint exp_count = array_len(ctx->properties);
	for(uint i = 0; i < exp_count; i++) {
		bool            update_index         = false;
		PropertySetCtx  property             = ctx->properties[i];
		Attribute_ID    attr_id              = property.id;
		SIValue         new_value            = AR_EXP_Evaluate(property.exp, r);

		//----------------------------------------------------------------------
		// validate value type
		//----------------------------------------------------------------------

		// emit an error and exit if we're trying to add an invalid type
		if(!(SI_TYPE(new_value) & accepted_properties)) {
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
			break;
		}

		// determine whether we must update the index for this update
		if(node_update && label_id != GRAPH_NO_LABEL) {
			// if the (label:attribute) combination has an index, take note
			update_index = GraphContext_GetIndexByID(gc, label_id, &attr_id,
					IDX_ANY) != NULL;
		}

		PendingUpdateCtx update = {
			.ge            =  entity,
			.attr_id       =  attr_id,
			.label_id      =  label_id,
			.new_value     =  new_value,
			.update_index  =  update_index,
		};

		// enqueue the current update
		*updates = array_append(*updates, update);

		// enqueue the current update's undo
		SIValue *orig_val = GraphEntity_GetProperty(entity, attr_id);
		//UndoLog_AddUpdate(undo_log, &update, orig_val);
	}
}

