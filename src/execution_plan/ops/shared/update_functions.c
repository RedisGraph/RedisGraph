/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "update_functions.h"
#include "../../../errors.h"
#include "../../../query_ctx.h"
#include "../../../datatypes/map.h"
#include "../../../datatypes/array.h"

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

static PendingUpdateCtx _PreparePendingUpdate
(
	GraphContext *gc,
	SIType accepted_properties,
	GraphEntity *entity,
	Attribute_ID attr_id,
	SIValue new_value,
	SchemaType t
) {
	//--------------------------------------------------------------------------
	// validate value type
	//--------------------------------------------------------------------------

	// emit an error and exit if we're trying to add an invalid type
	if(!(SI_TYPE(new_value) & accepted_properties)) {
		Error_InvalidPropertyValue();
		ErrorCtx_RaiseRuntimeException(NULL);
	}

	// emit an error and exit if we're trying to add
	// an array containing an invalid type
	if(SI_TYPE(new_value) == T_ARRAY) {
		SIType invalid_properties = ~SI_VALID_PROPERTY_VALUE;
		bool res = SIArray_ContainsType(new_value, invalid_properties);
		if(res) {
			// validation failed
			SIValue_Free(new_value);
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
		}
	}


	bool update_index = false;
	// determine whether we must update the index for this update
	if(t == SCHEMA_NODE) {
		uint label_count;
		NODE_GET_LABELS(gc->g, (Node *)entity, label_count);
		for(uint i = 0; i < label_count; i ++) {
			update_index = GraphContext_GetIndexByID(gc, labels[i], &attr_id,
													 IDX_ANY, SCHEMA_NODE) != NULL;
			if(update_index) break;
		}
	} else {
		Edge *e = (Edge *)entity;
		update_index = GraphContext_GetIndexByID(gc, EDGE_GET_RELATION_ID(e, gc->g),
												 &attr_id, IDX_ANY, SCHEMA_EDGE) != NULL;
	}

	return (PendingUpdateCtx) {
		.ge            =  entity,
		.attr_id       =  attr_id,
		.new_value     =  new_value,
		.update_index  =  update_index,
	};
}

// commits delayed updates
void CommitUpdates
(
	GraphContext *gc,
	ResultSetStatistics *stats,
	PendingUpdateCtx *updates,
	EntityType type
) {
	ASSERT(gc      != NULL);
	ASSERT(stats   != NULL);
	ASSERT(updates != NULL);
	ASSERT(type    != ENTITY_UNKNOWN);

	uint       properties_set = 0;
	Schema     *s             = NULL;
	bool       reindex        = false;
	uint       update_count   = array_len(updates);
	SchemaType t              = type == ENTITY_NODE ? SCHEMA_NODE : SCHEMA_EDGE;

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
				if(t == SCHEMA_NODE) {
					Node *n = (Node *)ge;
					// retrieve node labels
					uint label_count;
					NODE_GET_LABELS(gc->g, n, label_count);
					for(uint i = 0; i < label_count; i ++) {
						Schema *s = GraphContext_GetSchemaByID(gc, labels[i],
															SCHEMA_NODE);
						ASSERT(s != NULL);
						// introduce updated entity to index
						Schema_AddNodeToIndices(s, n);
					}
				} else {
					Edge *e =(Edge *)ge;
					s = GraphContext_GetSchemaByID(gc, EDGE_GET_RELATION_ID(e, gc->g), SCHEMA_EDGE);
					Schema_AddEdgeToIndices(s, e);
				}
			}

			// update state
			reindex  =  false;
			ge       =  updates[i].ge;
		}

		// if entity has been deleted, perform no updates
		if(GraphEntity_IsDeleted(ge)) continue;

		// update the property on the graph entity
		int updated = _UpdateEntity(update);
		properties_set += updated;
		// reindex only if update performed
		reindex |= update->update_index & (bool)updated;
	}

	// handle last updated entity
	if(reindex) {
		if(t == SCHEMA_NODE) {
			Node *n = (Node *)ge;
			// Retrieve node labels
			uint label_count;
			NODE_GET_LABELS(gc->g, n, label_count);
			for(uint i = 0; i < label_count; i ++) {
				Schema *s = GraphContext_GetSchemaByID(gc, labels[i],
					SCHEMA_NODE);
				ASSERT(s != NULL);
				// introduce updated entity to index
				Schema_AddNodeToIndices(s, n);
			}
		} else {
			Edge *e = (Edge *)ge;
			Schema *s = GraphContext_GetSchemaByID(gc, EDGE_GET_RELATION_ID(e, gc->g),
				SCHEMA_EDGE);
			Schema_AddEdgeToIndices(s, e);
		}
	}

	if(stats) stats->properties_set += properties_set;
}

void EvalEntityUpdates
(
	GraphContext *gc,
	PendingUpdateCtx **node_updates,
	PendingUpdateCtx **edge_updates,
	const Record r,
	const EntityUpdateEvalCtx *ctx,
	bool allow_null,
	UndoLog *undo_log
) {
	Schema *s         = NULL;
	bool update_index     = false;

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

	SchemaType st = t == REC_TYPE_NODE ? SCHEMA_NODE : SCHEMA_EDGE;

	PendingUpdateCtx **updates = t == REC_TYPE_NODE
		? node_updates
		: edge_updates;

	GraphEntity *entity = Record_GetGraphEntity(r, ctx->record_idx);

	// if the entity is a node
	if(t == REC_TYPE_NODE) {
		Node *n = (Node *)entity;
		uint label_count;
		NODE_GET_LABELS(gc->g, n, label_count);
		update_index = (label_count > 0);
	} else {
		update_index = true;
	}

	// if this update replaces all existing properties
	// enqueue a clear update to do so
	if(ctx->mode == UPDATE_REPLACE) {
		PendingUpdateCtx update = {
			.ge            =  entity,
			.update_index  =  update_index,
			.attr_id       =  ATTRIBUTE_ALL,
		};
		array_append(*updates, update);
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

		if(attr_id == ATTRIBUTE_ALL && !(SI_TYPE(new_value) & (T_NODE | T_EDGE | T_MAP))) {
			// left-hand side is alias reference but right-hand side is a
			// scalar, emit an error
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
		} else if(SI_TYPE(new_value) == T_MAP) {
			// value is of type map e.g. n.v = {a:1, b:2}
			SIValue m = new_value;
			if(attr_id != ATTRIBUTE_ALL) {
				Error_InvalidPropertyValue();
				ErrorCtx_RaiseRuntimeException(NULL);
			}
			// iterate over all map elements to build updates
			uint map_size = Map_KeyCount(m);
			for(uint j = 0; j < map_size; j ++) {
				SIValue key;
				SIValue value;
				Map_GetIdx(m, j, &key, &value);
				Attribute_ID attr_id = GraphContext_FindOrAddAttribute(gc,
																	   key.stringval);

				update = _PreparePendingUpdate(gc, accepted_properties, entity,
											   attr_id, value, st);
				// enqueue the current update
				array_append(*updates, update);
			}
			continue;
		} else if(SI_TYPE(new_value) & (T_NODE | T_EDGE)) {
			// value is a node or edge; perform attribute set reassignment
			GraphEntity *ge = new_value.ptrval;
			if(attr_id != ATTRIBUTE_ALL) {
				Error_InvalidPropertyValue();
				ErrorCtx_RaiseRuntimeException(NULL);
			}
			// iterate over all entity properties to build updates
			uint property_count = ENTITY_PROP_COUNT(ge);
			for(uint j = 0; j < property_count; j ++) {
				Attribute_ID attr_id = ENTITY_PROPS(ge)[j].id;
				SIValue value = ENTITY_PROPS(ge)[j].value;

				update = _PreparePendingUpdate(gc, accepted_properties, entity,
											   attr_id, value, st);
				// enqueue the current update
				array_append(*updates, update);
			}
			continue;
		}

		update = _PreparePendingUpdate(gc, accepted_properties, entity,
				attr_id, new_value, st);
		// enqueue the current update
		array_append(*updates, update);

		SIValue *orig_val = GraphEntity_GetProperty(entity, attr_id);
		UndoLog_AddUpdate(undo_log, &update, orig_val);
	}
}

