/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "update_functions.h"
#include "../../../errors.h"
#include "../../../query_ctx.h"
#include "../../../datatypes/map.h"
#include "../../../datatypes/array.h"
#include "../../../graph/graph_hub.h"

static bool _ValidateAttrType
(
	SIType accepted_properties,
	SIValue v
) {
	//--------------------------------------------------------------------------
	// validate value type
	//--------------------------------------------------------------------------

	SIType t = SI_TYPE(v);

	// make sure value is of an acceptable type
	if(!(t & accepted_properties)) {
		return false;
	}

	// in case of an array, make sure each element is of an
	// acceptable type
	if(t == T_ARRAY) {
		SIType invalid_properties = ~SI_VALID_PROPERTY_VALUE;
		return !SIArray_ContainsType(v, invalid_properties);
	}

	return true;
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
	if(t == REC_TYPE_UNKNOWN) {
		return;
	}

	// make sure we're updating either a node or an edge
	if(t != REC_TYPE_NODE && t != REC_TYPE_EDGE) {
		ErrorCtx_RaiseRuntimeException(
			"Update error: alias '%s' did not resolve to a graph entity",
			ctx->alias);
	}

	// label(s) update can only be performed on nodes
	if((ctx->add_labels != NULL || ctx->remove_labels != NULL) &&
			t != REC_TYPE_NODE) {
		ErrorCtx_RaiseRuntimeException(
				"Type mismatch: expected Node but was Relationship");
	}

	PendingUpdateCtx **updates = (t == REC_TYPE_NODE)
		? node_updates
		: edge_updates;

	GraphEntity *entity = Record_GetGraphEntity(r, ctx->record_idx);

	PendingUpdateCtx update = {0};

	update.ge            = entity;
	update.attributes    = NULL;
	update.add_labels    = ctx->add_labels;
	update.remove_labels = ctx->remove_labels;

	// if we're converting a SET clause, NULL is acceptable
	// as it indicates a deletion
	SIType accepted_properties = SI_VALID_PROPERTY_VALUE;
	if(allow_null) {
		accepted_properties |= T_NULL;
	}

	bool error = false;
	uint exp_count = array_len(ctx->properties);

	// evaluate each assigned expression
	// e.g. n.v = n.a + 2
	//
	// validate each new value type
	// e.g. invalid n.v = [1, {}]
	//
	// collect all updates into a single attribute-set
	//

	for(uint i = 0; i < exp_count && !error; i++) {
		PropertySetCtx *property = ctx->properties + i;

		SIValue     v         = AR_EXP_Evaluate(property->exp, r);
		SIType      t         = SI_TYPE(v);
		UPDATE_MODE mode      = property->mode;
		const char* attribute = property->attribute;

		//----------------------------------------------------------------------
		// n.v = 2
		//----------------------------------------------------------------------

		if(attribute != NULL) {
			// a specific attribute is set, validate the value type
			if(!_ValidateAttrType(accepted_properties, v)) {
				error = true;
				SIValue_Free(v);
				Error_InvalidPropertyValue();
				break;
			}

			Attribute_ID attr_id = FindOrAddAttribute(gc, attribute);
			AttributeSet_Set_Allow_Null(&update.attributes, attr_id, v);
			SIValue_Free(v);
			continue;
		}

		//----------------------------------------------------------------------
		// n = {v:2}, n = m
		//----------------------------------------------------------------------

		if(!(t & (T_NODE | T_EDGE | T_MAP))) {
			error = true;
			SIValue_Free(v);
			Error_InvalidPropertyValue();
			break;
		}

		if(mode == UPDATE_REPLACE) {
			// if this update replaces all existing properties
			// enqueue a 'clear' update to do so
			AttributeSet_Set_Allow_Null(&update.attributes, ATTRIBUTE_ID_ALL,
					SI_NullVal());
		}

		//----------------------------------------------------------------------
		// n = {v:2}
		//----------------------------------------------------------------------

		if(t == T_MAP) {
			// value is of type map e.g. n.v = {a:1, b:2}
			// iterate over all map elements to build updates
			uint map_size = Map_KeyCount(v);
			for(uint j = 0; j < map_size; j ++) {
				SIValue key;
				SIValue value;

				Map_GetIdx(v, j, &key, &value);

				if(!_ValidateAttrType(accepted_properties, value)) {
					error = true;
					Error_InvalidPropertyValue();
					break;
				}

				Attribute_ID attr_id = FindOrAddAttribute(gc, key.stringval);
				AttributeSet_Set_Allow_Null(&update.attributes, attr_id, value);
			}

			// free map
			SIValue_Free(v);
			continue;
		}

		//----------------------------------------------------------------------
		// n = m
		//----------------------------------------------------------------------

		// value is a node or edge; perform attribute set reassignment
		ASSERT((t & (T_NODE | T_EDGE)));

		GraphEntity *ge = v.ptrval;

		// iterate over all entity properties to build updates
		const AttributeSet set = GraphEntity_GetAttributes(ge);

		for(uint j = 0; j < ATTRIBUTE_SET_COUNT(set); j++) {
			Attribute_ID attr_id;
			SIValue v = AttributeSet_GetIdx(set, j, &attr_id);

			// simple assignment, no need to value validation
			AttributeSet_Set_Allow_Null(&update.attributes, attr_id, v);
		}
	} // for loop end

	// enqueue the current update
	array_append(*updates, update);
}
