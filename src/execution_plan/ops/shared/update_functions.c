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
#include "../../../graph/graph_hub.h"

static void _PreparePendingUpdate
(
	AttributeSet *props,
	SIType accepted_properties,
	Attribute_ID attr_id,
	SIValue new_value
) {
	//--------------------------------------------------------------------------
	// validate value type
	//--------------------------------------------------------------------------

	// emit an error and exit if we're trying to add an invalid type
	if(!(SI_TYPE(new_value) & accepted_properties)) {
		AttributeSet_Free(props);
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
			AttributeSet_Free(props);
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
		}
	}

	AttributeSet_Set_Allow_Null(props, attr_id, new_value);
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

	uint update_count       = array_len(updates);
	uint labels_added       = 0;
	uint labels_removed     = 0;
	uint properties_set     = 0;
	uint properties_removed = 0;

	// return early if no updates are enqueued
	if(update_count == 0) return;

	for(uint i = 0; i < update_count; i++) {
		PendingUpdateCtx *update = updates + i;

		// if entity has been deleted, perform no updates
		if(GraphEntity_IsDeleted(update->ge)) continue;
		uint _labels_added   = 0;
		uint _labels_removed = 0;
		uint _props_set      = 0;
		uint _props_removed  = 0;

		// update the attributes on the graph entity
		UpdateEntityProperties(gc, update->ge, update->attributes,
				type == ENTITY_NODE ? GETYPE_NODE : GETYPE_EDGE, &_props_set,
				&_props_removed);

		if(type == ENTITY_NODE) {
			UpdateNodeLabels(gc, (Node*)update->ge, update->add_labels,
				update->remove_labels, &_labels_added, &_labels_removed);
		}

		labels_added       += _labels_added;
		labels_removed     += _labels_removed;
		properties_set     += _props_set;
		properties_removed += _props_removed;
	}

	if(stats) {
		stats->labels_added       += labels_added;
		stats->labels_removed     += labels_removed;
		stats->properties_set     += properties_set;
		stats->properties_removed += properties_removed;
	}
}

void EvalEntityUpdates
(
	GraphContext *gc,
	PendingUpdateCtx **node_updates,
	PendingUpdateCtx **edge_updates,
	const Record r,
	const EntityUpdateEvalCtx *ctx,
	bool allow_null
) {
	Schema *s = NULL;

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

	// label(s) update can only be performed on nodes
	if ((ctx->add_labels != NULL || ctx->remove_labels != NULL) && t != REC_TYPE_NODE) {
		ErrorCtx_RaiseRuntimeException(
				"Type mismatch: expected Node but was Relationship");
	}

	PendingUpdateCtx **updates = t == REC_TYPE_NODE
		? node_updates
		: edge_updates;

	GraphEntity *entity = Record_GetGraphEntity(r, ctx->record_idx);

	PendingUpdateCtx update = {0};
	update.ge            = entity;
	update.attributes    = AttributeSet_New();
	update.add_labels    = ctx->add_labels;
	update.remove_labels = ctx->remove_labels;

	// if we're converting a SET clause, NULL is acceptable
	// as it indicates a deletion
	SIType accepted_properties = SI_VALID_PROPERTY_VALUE;
	if(allow_null) accepted_properties |= T_NULL;

	uint exp_count = array_len(ctx->properties);

	//--------------------------------------------------------------------------
	// enqueue update
	//--------------------------------------------------------------------------
	
	for(uint i = 0; i < exp_count; i++) {
		PropertySetCtx *property = ctx->properties + i;
		SIValue        new_value = AR_EXP_Evaluate(property->exp, r);
		UPDATE_MODE mode = property->mode;
		const char *attribute = property->attribute;
		if(attribute == NULL && (SI_TYPE(new_value) & SI_VALID_PROPERTY_VALUE )){
			AttributeSet_Free(&update.attributes);
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
		}

		if(attribute != NULL && !((SI_TYPE(new_value) & (SI_VALID_PROPERTY_VALUE | T_NULL) ))){
			AttributeSet_Free(&update.attributes);
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
		}

		// if this update replaces all existing properties
		// enqueue a clear update to do so
		if(mode == UPDATE_REPLACE) {
			AttributeSet_Set_Allow_Null(&update.attributes, ATTRIBUTE_ID_ALL,
					SI_NullVal());
		}

		if(mode == UPDATE_REPLACE &&
		   !(SI_TYPE(new_value) & (T_NODE | T_EDGE | T_MAP))) {
			// left-hand side is alias reference but right-hand side is a
			// scalar, emit an error
			AttributeSet_Free(&update.attributes);
			Error_InvalidPropertyValue();
			ErrorCtx_RaiseRuntimeException(NULL);
		} else if(SI_TYPE(new_value) == T_MAP) {
			// Map value can only be assigned as the entity property map or added to it
			// MATCH n SET n.v = {k:v} is not allowed.
			ASSERT(property->attribute == NULL);
			// value is of type map e.g. n.v = {a:1, b:2}
			SIValue m = new_value;
			// iterate over all map elements to build updates
			uint map_size = Map_KeyCount(m);
			for(uint j = 0; j < map_size; j ++) {
				SIValue key;
				SIValue value;
				Map_GetIdx(m, j, &key, &value);
				Attribute_ID attr_id = FindOrAddAttribute(gc, key.stringval);

				_PreparePendingUpdate(&update.attributes, accepted_properties,
					attr_id, value);
			}
			continue;
		} else if(SI_TYPE(new_value) & (T_NODE | T_EDGE)) {
			// Node or edge property maps can only be assigned as the entity property map or added to it
			// MATCH n, M SET n.v = m is not allowed.
			ASSERT(property->attribute == NULL);
			// value is a node or edge; perform attribute set reassignment
			GraphEntity *ge = new_value.ptrval;
			// // iterate over all entity properties to build updates
			const AttributeSet set = GraphEntity_GetAttributes(ge);
			for(uint j = 0; j < ATTRIBUTE_SET_COUNT(set); j ++) {
				Attribute_ID attr_id;
				SIValue value = AttributeSet_GetIdx(set, j, &attr_id);

				_PreparePendingUpdate(&update.attributes, accepted_properties,
					attr_id, SI_CloneValue(value));
			}
			continue;
		}
		Attribute_ID attr_id = FindOrAddAttribute(gc, attribute);
		_PreparePendingUpdate(&update.attributes, accepted_properties,
				attr_id, new_value);
	}

	// enqueue the current update
	array_append(*updates, update);
}

