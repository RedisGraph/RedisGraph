/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "../../util/arr.h"
#include "../../datatypes/array.h"
#include "../../datatypes/sipath.h"

// Forward declarations.
static void _ResultSet_CompactReplyWithNode(RedisModuleCtx *ctx, GraphContext *gc, Node *n);
static void _ResultSet_CompactReplyWithEdge(RedisModuleCtx *ctx, GraphContext *gc, Edge *e);
static void _ResultSet_CompactReplyWithSIArray(RedisModuleCtx *ctx, GraphContext *gc,
											   SIValue array);
static void _ResultSet_CompactReplyWithPath(RedisModuleCtx *ctx, GraphContext *gc, SIValue path);

static inline ValueType _mapValueType(const SIValue v) {
	switch(SI_TYPE(v)) {
	case T_NULL:
		return VALUE_NULL;
	case T_STRING:
		return VALUE_STRING;
	case T_INT64:
		return VALUE_INTEGER;
	case T_BOOL:
		return VALUE_BOOLEAN;
	case T_DOUBLE:
		return VALUE_DOUBLE;
	case T_ARRAY:
		return VALUE_ARRAY;
	case T_NODE:
		return VALUE_NODE;
	case T_EDGE:
		return VALUE_EDGE;
	default:
		return VALUE_UNKNOWN;
	}
}

static inline void _ResultSet_ReplyWithValueType(RedisModuleCtx *ctx, const SIValue v) {
	RedisModule_ReplyWithLongLong(ctx, _mapValueType(v));
}

static void _ResultSet_CompactReplyWithSIValue(RedisModuleCtx *ctx, GraphContext *gc,
											   const SIValue v) {
	// Emit the actual value, then the value type (to facilitate client-side parsing)
	switch(SI_TYPE(v)) {
	case T_STRING:
		_ResultSet_ReplyWithValueType(ctx, v);
		RedisModule_ReplyWithStringBuffer(ctx, v.stringval, strlen(v.stringval));
		return;
	case T_INT64:
		_ResultSet_ReplyWithValueType(ctx, v);
		RedisModule_ReplyWithLongLong(ctx, v.longval);
		return;
	case T_DOUBLE:
		_ResultSet_ReplyWithValueType(ctx, v);
		_ResultSet_ReplyWithRoundedDouble(ctx, v.doubleval);
		return;
	case T_BOOL:
		_ResultSet_ReplyWithValueType(ctx, v);
		if(v.longval != 0) RedisModule_ReplyWithStringBuffer(ctx, "true", 4);
		else RedisModule_ReplyWithStringBuffer(ctx, "false", 5);
		return;
	case T_ARRAY:
		_ResultSet_ReplyWithValueType(ctx, v);
		_ResultSet_CompactReplyWithSIArray(ctx, gc, v);
		break;
	case T_NULL:
		_ResultSet_ReplyWithValueType(ctx, v);
		RedisModule_ReplyWithNull(ctx);
		return;
	case T_NODE:
		_ResultSet_ReplyWithValueType(ctx, v);
		_ResultSet_CompactReplyWithNode(ctx, gc, v.ptrval);
		return;
	case T_EDGE:
		_ResultSet_ReplyWithValueType(ctx, v);
		_ResultSet_CompactReplyWithEdge(ctx, gc, v.ptrval);
		return;
	case T_PATH:
		_ResultSet_CompactReplyWithPath(ctx, gc, v);
		return;
	default:
		assert("Unhandled value type" && false);
	}
}

static void _ResultSet_CompactReplyWithProperties(RedisModuleCtx *ctx, GraphContext *gc,
												  const GraphEntity *e) {
	int prop_count = ENTITY_PROP_COUNT(e);
	RedisModule_ReplyWithArray(ctx, prop_count);
	// Iterate over all properties stored on entity
	for(int i = 0; i < prop_count; i ++) {
		// Compact replies include the value's type; verbose replies do not
		RedisModule_ReplyWithArray(ctx, 3);
		EntityProperty prop = ENTITY_PROPS(e)[i];
		// Emit the string index
		RedisModule_ReplyWithLongLong(ctx, prop.id);
		// Emit the value
		_ResultSet_CompactReplyWithSIValue(ctx, gc, prop.value);
	}
}

static void _ResultSet_CompactReplyWithNode(RedisModuleCtx *ctx, GraphContext *gc, Node *n) {
	/*  Compact node reply format:
	 *  [
	 *      Node ID (integer),
	        [label string index (integer)],
	 *      [[name, value, value type] X N]
	 *  ]
	 */
	// 3 top-level entities in node reply
	RedisModule_ReplyWithArray(ctx, 3);

	// id (integer)
	EntityID id = ENTITY_GET_ID(n);
	RedisModule_ReplyWithLongLong(ctx, id);

	// [label string index]
	// Print label in nested array for multi-label support
	// Retrieve label
	int label_id = Graph_GetNodeLabel(gc->g, id);
	if(label_id == GRAPH_NO_LABEL) {
		// Emit an empty array for unlabeled nodes
		RedisModule_ReplyWithArray(ctx, 0);
	} else {
		RedisModule_ReplyWithArray(ctx, 1);
		RedisModule_ReplyWithLongLong(ctx, label_id);
	}

	// [properties]
	_ResultSet_CompactReplyWithProperties(ctx, gc, (GraphEntity *)n);
}

static void _ResultSet_CompactReplyWithEdge(RedisModuleCtx *ctx, GraphContext *gc, Edge *e) {
	/*  Compact edge reply format:
	 *  [
	 *      Edge ID (integer),
	        reltype string index (integer),
	        src node ID (integer),
	        dest node ID (integer),
	 *      [[name, value, value type] X N]
	 *  ]
	 */
	// 5 top-level entities in edge reply
	RedisModule_ReplyWithArray(ctx, 5);

	// id (integer)
	EntityID id = ENTITY_GET_ID(e);
	RedisModule_ReplyWithLongLong(ctx, id);

	// reltype string index, retrieve reltype.
	int reltype_id = Graph_GetEdgeRelation(gc->g, e);
	assert(reltype_id != GRAPH_NO_RELATION);
	RedisModule_ReplyWithLongLong(ctx, reltype_id);

	// src node ID
	RedisModule_ReplyWithLongLong(ctx, Edge_GetSrcNodeID(e));

	// dest node ID
	RedisModule_ReplyWithLongLong(ctx, Edge_GetDestNodeID(e));

	// [properties]
	_ResultSet_CompactReplyWithProperties(ctx, gc, (GraphEntity *)e);
}

static void _ResultSet_CompactReplyWithSIArray(RedisModuleCtx *ctx, GraphContext *gc,
											   SIValue array) {

	/*  Compact array reply format:
	 *  [
	 *      [type, value] // every member is returned at its compact representation
	 *      [type, value]
	 *      .
	 *      .
	 *      .
	 *      [type, value]
	 *  ]
	 */
	uint arrayLen = SIArray_Length(array);
	RedisModule_ReplyWithArray(ctx, arrayLen);
	for(uint i = 0; i < arrayLen; i++) {
		RedisModule_ReplyWithArray(ctx, 2); // Reply with array with space for type and value
		_ResultSet_CompactReplyWithSIValue(ctx, gc, SIArray_Get(array, i));
	}
}

static void _ResultSet_CompactReplyWithPath(RedisModuleCtx *ctx, GraphContext *gc, SIValue path) {
	/* If path is intermidate it will return as an SIArray of edges, see array compact format.
	 * Compact path reply:
	 * [
	 *      [Node compact reply format]
	 *      [Edge compact reply format]
	 *      .
	 *      .
	 *      .
	 *      [Node compact reply format]
	 *
	 * ]
	*/
	SIPath *sipathPtr = (SIPath *)path.ptrval;
	if(sipathPtr->intermidate) {
		SIValue relationships = SIPath_Relationships(path);
		_ResultSet_CompactReplyWithSIValue(ctx, gc, relationships);
		SIValue_Free(&relationships);
	}
}

void ResultSet_EmitCompactRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r,
								 uint numcols) {
	// Prepare return array sized to the number of RETURN entities
	RedisModule_ReplyWithArray(ctx, numcols);

	for(uint i = 0; i < numcols; i++) {
		switch(Record_GetType(r, i)) {
		case REC_TYPE_NODE:
			_ResultSet_CompactReplyWithNode(ctx, gc, Record_GetNode(r, i));
			break;
		case REC_TYPE_EDGE:
			_ResultSet_CompactReplyWithEdge(ctx, gc, Record_GetEdge(r, i));
			break;
		default:
			RedisModule_ReplyWithArray(ctx, 2); // Reply with array with space for type and value
			_ResultSet_CompactReplyWithSIValue(ctx, gc, Record_GetScalar(r, i));
		}
	}
}

// For every column in the header, emit a 2-array that specifies
// the column alias followed by an enum denoting what type
// (scalar, node, or relation) it holds.
void ResultSet_ReplyWithCompactHeader(RedisModuleCtx *ctx, const char **columns,
									  const Record r) {
	uint columns_len = array_len(columns);
	RedisModule_ReplyWithArray(ctx, columns_len);
	for(uint i = 0; i < columns_len; i++) {
		RedisModule_ReplyWithArray(ctx, 2);
		ColumnType t;
		// First, emit the column type enum
		if(r) {
			RecordEntryType entry_type = Record_GetType(r, i);
			switch(entry_type) {
			case REC_TYPE_NODE:
				t = COLUMN_NODE;
				break;
			case REC_TYPE_EDGE:
				t = COLUMN_RELATION;
				break;
			case REC_TYPE_SCALAR:
			case REC_TYPE_UNKNOWN:  // Treat unknown as scalar.
				t = COLUMN_SCALAR;
				break;
			default:
				assert(false);
			}
		} else {
			// If we didn't receive a record, no results will be returned,
			// and the column types don't matter.
			t = COLUMN_SCALAR;
		}

		RedisModule_ReplyWithLongLong(ctx, t);

		// Second, emit the identifier string associated with the column
		RedisModule_ReplyWithStringBuffer(ctx, columns[i], strlen(columns[i]));
	}
}

