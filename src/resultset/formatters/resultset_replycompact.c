/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "../../util/arr.h"
#include "../../datatypes/array.h"
#include "../../datatypes/path/sipath.h"

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
	case T_PATH:
		return VALUE_PATH;
	default:
		return VALUE_UNKNOWN;
	}
}

static inline void _ResultSet_ReplyWithValueType(RedisModuleCtx *ctx, const SIValue v) {
	RedisModule_ReplyWithLongLong(ctx, _mapValueType(v));
}

static void _ResultSet_CompactReplyWithSIValue(RedisModuleCtx *ctx, GraphContext *gc,
											   const SIValue v) {
	// Emit the value type, then the actual value (to facilitate client-side parsing)
	_ResultSet_ReplyWithValueType(ctx, v);

	switch(SI_TYPE(v)) {
	case T_STRING:
		RedisModule_ReplyWithStringBuffer(ctx, v.stringval, strlen(v.stringval));
		return;
	case T_INT64:
		RedisModule_ReplyWithLongLong(ctx, v.longval);
		return;
	case T_DOUBLE:
		_ResultSet_ReplyWithRoundedDouble(ctx, v.doubleval);
		return;
	case T_BOOL:
		if(v.longval != 0) RedisModule_ReplyWithStringBuffer(ctx, "true", 4);
		else RedisModule_ReplyWithStringBuffer(ctx, "false", 5);
		return;
	case T_ARRAY:
		_ResultSet_CompactReplyWithSIArray(ctx, gc, v);
		break;
	case T_NULL:
		RedisModule_ReplyWithNull(ctx);
		return;
	case T_NODE:
		_ResultSet_CompactReplyWithNode(ctx, gc, v.ptrval);
		return;
	case T_EDGE:
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
	/* Path will return as an array of two SIArrays, the first is path nodes and the second is edges,
	* see array compact format.
	* Compact path reply:
	* [
	*      type : array,
	*      [
	*          [Node compact reply format],
	*          .
	*          .
	*          .
	*          [Node compact reply format]
	*      ],
	*      type: array,
	*      [
	*          [Edge compact reply format],
	*          .
	*          .
	*          .
	*          [Edge compact reply format]
	*      ]
	* ]
	*/

	// Response consists of two arrays.
	RedisModule_ReplyWithArray(ctx, 2);
	// First array type and value.
	RedisModule_ReplyWithArray(ctx, 2);
	SIValue nodes = SIPath_Nodes(path);
	_ResultSet_CompactReplyWithSIValue(ctx, gc, nodes);
	SIValue_Free(nodes);
	// Second array type and value.
	RedisModule_ReplyWithArray(ctx, 2);
	SIValue relationships = SIPath_Relationships(path);
	_ResultSet_CompactReplyWithSIValue(ctx, gc, relationships);
	SIValue_Free(relationships);
}

void ResultSet_EmitCompactRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r,
								 uint numcols, uint *col_rec_map) {
	// Prepare return array sized to the number of RETURN entities
	RedisModule_ReplyWithArray(ctx, numcols);

	for(uint i = 0; i < numcols; i++) {
		uint idx = col_rec_map[i];
		RedisModule_ReplyWithArray(ctx, 2); // Reply with array with space for type and value
		_ResultSet_CompactReplyWithSIValue(ctx, gc, Record_Get(r, idx));
	}
}

// For every column in the header, emit a 2-array containing the ColumnType enum
// followed by the column alias.
void ResultSet_ReplyWithCompactHeader(RedisModuleCtx *ctx, const char **columns, const Record r,
									  uint *col_rec_map) {
	uint columns_len = array_len(columns);
	RedisModule_ReplyWithArray(ctx, columns_len);
	for(uint i = 0; i < columns_len; i++) {
		RedisModule_ReplyWithArray(ctx, 2);
		/* Because the types found in the first Record do not necessarily inform the types
		 * in subsequent records, we will always set the column type as scalar. */
		ColumnType t = COLUMN_SCALAR;
		RedisModule_ReplyWithLongLong(ctx, t);

		// Second, emit the identifier string associated with the column
		RedisModule_ReplyWithStringBuffer(ctx, columns[i], strlen(columns[i]));
	}
}

