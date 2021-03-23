/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "update_functions.h"
#include "../../../errors.h"

/* Set a property on a graph entity. For non-NULL values, the property
 * will be added or updated if it is already present.
 * For NULL values, the property will be deleted if present
 * and nothing will be done otherwise.
 * Returns 1 if a property was set or deleted.  */
static int _UpdateEntity(PendingUpdateCtx *update) {
	int           res        =  0;
	GraphEntity   *ge        =  update->ge;
	Attribute_ID  attr_id    =  update->attr_id;
	SIValue       new_value  =  update->new_value;

	// If this entity has been deleted, perform no updates and return early.
	if(GraphEntity_IsDeleted(ge)) goto cleanup;

	// Handle the case in which we are deleting all properties.
	if(attr_id == ATTRIBUTE_ALL) return GraphEntity_ClearProperties(ge);

	// Try to get current property value.
	SIValue *old_value = GraphEntity_GetProperty(ge, attr_id);

	if(old_value == PROPERTY_NOTFOUND) {
		// Adding a new property; do nothing if its value is NULL.
		if(SI_TYPE(new_value) != T_NULL) {
			res = GraphEntity_AddProperty(ge, attr_id, new_value);
		}
	} else {
		// Update property.
		res = GraphEntity_SetProperty(ge, attr_id, new_value);
	}

cleanup:
	SIValue_Free(new_value);
	return res;
}

// Commits delayed updates.
void CommitUpdates(GraphContext *gc, ResultSetStatistics *stats, PendingUpdateCtx *updates) {
	uint update_count = array_len(updates);
	uint properties_set = 0;
	for(uint i = 0; i < update_count; i++) {
		PendingUpdateCtx *update = updates + i;
		// Update the property on the graph entity.
		properties_set += _UpdateEntity(update);

		// Update index for node entities if indexed fields have been modified.
		if(update->update_index) {
			Schema *s = GraphContext_GetSchemaByID(gc, update->label_id, SCHEMA_NODE);
			// Introduce updated entity to index.
			Schema_AddNodeToIndices(s, (Node *)update->ge);
		}
	}

	if(stats) stats->properties_set += properties_set;
}

void EvalEntityUpdates(GraphContext *gc, PendingUpdateCtx **updates, Record r, char *alias,
					   EntityUpdateEvalCtx *ctx, bool allow_null) {
	Schema *s         = NULL;
	int label_id      = GRAPH_NO_LABEL;
	const char *label = NULL;
	bool node_update  = false;

	// Get the type of the entity to update. If the expected entity was not
	// found, make no updates but do not error.
	RecordEntryType t = Record_GetType(r, ctx->record_idx);
	if(t == REC_TYPE_UNKNOWN) return;

	// Make sure we're updating either a node or an edge.
	if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) {
		ErrorCtx_RaiseRuntimeException("Update error: alias '%s' did not resolve to a graph entity",
									   alias);
	}

	GraphEntity *entity = Record_GetGraphEntity(r, ctx->record_idx);

	if(t == REC_TYPE_NODE) node_update = true;

	// If the entity is a node, set its label if possible.
	if(node_update) {
		Node *n = (Node *)entity;
		// Retrieve the node's local label if present.
		label = NODE_GET_LABEL(n);
		// Retrieve the node's Label ID from a local member or the graph.
		label_id = NODE_GET_LABEL_ID(n, gc->g);
		if((label == NULL) && (label_id != GRAPH_NO_LABEL)) {
			// Label ID has been found but its name is unknown, retrieve its name and update the node.
			s = GraphContext_GetSchemaByID(gc, label_id, SCHEMA_NODE);
			label = Schema_GetName(s);
			n->label = label;
		}
	}

	// If this update replaces all existing properties, enqueue an update to do so.
	if(ctx->mode == UPDATE_REPLACE) {
		PendingUpdateCtx update = {
			.ge            =  entity,
			.label_id      =  label_id,
			.update_index  = (label_id != GRAPH_NO_LABEL),
			.attr_id       =  ATTRIBUTE_ALL,
		};
		*updates = array_append(*updates, update);
	}

	uint exp_count = array_len(ctx->properties);
	for(uint i = 0; i < exp_count; i++) {
		bool update_index = false;
		SIValue new_value = AR_EXP_Evaluate(ctx->properties[i].value, r);

		// Emit an error and exit if we're trying to add an invalid type.
		// If we're converting a SET clause, NULL is acceptable as it indicates a deletion.
		if((allow_null && !(SI_TYPE(new_value) & (SI_VALID_PROPERTY_VALUE | T_NULL))) ||
		   (!allow_null && !(SI_TYPE(new_value) & SI_VALID_PROPERTY_VALUE))) {
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
			break;
		}

		Attribute_ID attr_id = ctx->properties[i].id;
		// Determine whether we must update the index for this update.
		if(node_update && label_id != GRAPH_NO_LABEL) {
			// If the (label:attribute) combination has an index, take note.
			update_index = GraphContext_GetIndex(gc, label, &attr_id, IDX_ANY) != NULL;
		}

		PendingUpdateCtx update = {
			.ge            =  entity,
			.label_id      =  label_id,
			.new_value     =  new_value,
			.update_index  =  update_index,
			.attr_id       =  attr_id,
		};

		// Enqueue the current update.
		*updates = array_append(*updates, update);
	}
}

