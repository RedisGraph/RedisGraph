/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "RG.h"
#include "../../util/arr.h"
#include "../../datatypes/datatypes.h"

// Forward declarations.
static void _ResultSet_CompactReplyWithNode
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	Node *n
);

static void _ResultSet_CompactReplyWithEdge
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	Edge *e
);

static void _ResultSet_CompactReplyWithSIArray
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *array
);

static void _ResultSet_CompactReplyWithPath
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *path
);

static void _ResultSet_CompactReplyWithMap
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *v
);

static void _ResultSet_CompactReplyWithPoint
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *v
);

static inline ValueType _mapValueType
(
	const SIValue *v
) {
	switch(SI_TYPE(*v)) {
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
	case T_MAP:
		return VALUE_MAP;
	case T_POINT:
		return VALUE_POINT;
	default:
		return VALUE_UNKNOWN;
	}
}

static inline void _ResultSet_ReplyWithValueType
(
	RedisModuleCtx *ctx,
	const SIValue *v
) {
	RedisModule_ReplyWithLongLong(ctx, _mapValueType(v));
}

static void _ResultSet_CompactReplyWithSIValue
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *v
) {
	// Emit the value type, then the actual value (to facilitate client-side parsing)
	_ResultSet_ReplyWithValueType(ctx, v);

	switch(SI_TYPE(*v)) {
	case T_STRING:
		RedisModule_ReplyWithStringBuffer(ctx, v->stringval, strlen(v->stringval));
		return;
	case T_INT64:
		RedisModule_ReplyWithLongLong(ctx, v->longval);
		return;
	case T_DOUBLE:
		_ResultSet_ReplyWithRoundedDouble(ctx, v->doubleval);
		return;
	case T_BOOL:
		if(v->longval != 0) RedisModule_ReplyWithStringBuffer(ctx, "true", 4);
		else RedisModule_ReplyWithStringBuffer(ctx, "false", 5);
		return;
	case T_ARRAY:
		_ResultSet_CompactReplyWithSIArray(ctx, gc, v);
		break;
	case T_NULL:
		RedisModule_ReplyWithNull(ctx);
		return;
	case T_NODE:
		_ResultSet_CompactReplyWithNode(ctx, gc, v->ptrval);
		return;
	case T_EDGE:
		_ResultSet_CompactReplyWithEdge(ctx, gc, v->ptrval);
		return;
	case T_PATH:
		_ResultSet_CompactReplyWithPath(ctx, gc, v);
		return;
	case T_MAP:
		_ResultSet_CompactReplyWithMap(ctx, gc, v);
		return;
	case T_POINT:
		_ResultSet_CompactReplyWithPoint(ctx, gc, v);
		return;
	default:
		RedisModule_Assert("Unhandled value type" && false);
		break;
	}
}

static void _ResultSet_CompactReplyWithProperties
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	const GraphEntity *e
) {
	const AttributeSet set = GraphEntity_GetAttributes(e);
	int prop_count = ATTRIBUTE_SET_COUNT(set);
	RedisModule_ReplyWithArray(ctx, prop_count);
	// Iterate over all properties stored on entity
	for(int i = 0; i < prop_count; i ++) {
		// Compact replies include the value's type; verbose replies do not
		RedisModule_ReplyWithArray(ctx, 3);
		Attribute_ID attr_id;
		SIValue value = AttributeSet_GetIdx(set, i, &attr_id);
		// emit the string index
		RedisModule_ReplyWithLongLong(ctx, attr_id);
		// emit the value
		_ResultSet_CompactReplyWithSIValue(ctx, gc, &value);
	}
}

static void _ResultSet_CompactReplyWithNode
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	Node *n
) {
	/*  Compact node reply format:
	 *  [
	 *      Node ID (integer),
	        [label string index (integer) X N],
	 *      [[name, value, value type] X N]
	 *  ]
	 */
	// 3 top-level entities in node reply
	RedisModule_ReplyWithArray(ctx, 3);

	// id (integer)
	EntityID id = ENTITY_GET_ID(n);
	RedisModule_ReplyWithLongLong(ctx, id);

	// [label string index X N]
	// Retrieve node labels
	uint lbls_count;
	NODE_GET_LABELS(gc->g, n, lbls_count);
	RedisModule_ReplyWithArray(ctx, lbls_count);
	for(int i = 0; i < lbls_count; i++) {
		RedisModule_ReplyWithLongLong(ctx, labels[i]);
	}

	// [properties]
	_ResultSet_CompactReplyWithProperties(ctx, gc, (GraphEntity *)n);
}

static void _ResultSet_CompactReplyWithEdge
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	Edge *e
) {
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
	ASSERT(reltype_id != GRAPH_NO_RELATION);
	RedisModule_ReplyWithLongLong(ctx, reltype_id);

	// src node ID
	RedisModule_ReplyWithLongLong(ctx, Edge_GetSrcNodeID(e));

	// dest node ID
	RedisModule_ReplyWithLongLong(ctx, Edge_GetDestNodeID(e));

	// [properties]
	_ResultSet_CompactReplyWithProperties(ctx, gc, (GraphEntity *)e);
}

static void _ResultSet_CompactReplyWithSIArray
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *array
) {

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
	uint arrayLen = SIArray_Length(*array);
	RedisModule_ReplyWithArray(ctx, arrayLen);
	for(uint i = 0; i < arrayLen; i++) {
		RedisModule_ReplyWithArray(ctx, 2); // Reply with array with space for type and value
		SIValue v = SIArray_Get(*array, i);
		_ResultSet_CompactReplyWithSIValue(ctx, gc, &v);
	}
}

static void _ResultSet_CompactReplyWithPath
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *path
) {
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
	SIValue nodes = SIPath_Nodes(*path);
	_ResultSet_CompactReplyWithSIValue(ctx, gc, &nodes);
	SIValue_Free(nodes);
	// Second array type and value.
	RedisModule_ReplyWithArray(ctx, 2);
	SIValue relationships = SIPath_Relationships(*path);
	_ResultSet_CompactReplyWithSIValue(ctx, gc, &relationships);
	SIValue_Free(relationships);
}

static void _ResultSet_CompactReplyWithMap
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *v
) {
	// map will be returned as an array of key/value pairs
	// consider the map object: {a:1, b:'str', c: {x:1, y:2}}
	//
	// the reply will be structured:
	// [
	//     string(a), int(1),
	//     string(b), string(str),
	//     string(c), [
	//                    string(x), int(1),
	//                    string(y), int(2)
	//                 ]
	// ]

	uint key_count = Map_KeyCount(*v);
	Map m = v->map;

	// response consists of N pairs array:
	// (string, value type, value)
	RedisModule_ReplyWithArray(ctx, key_count * 2);
	for(int i = 0; i < key_count; i++) {
		Pair     p     =  m[i];
		SIValue  val   =  p.val;
		char     *key  =  p.key.stringval;

		// emit key
		RedisModule_ReplyWithCString(ctx, key);

		// emit value
		RedisModule_ReplyWithArray(ctx, 2);
		_ResultSet_CompactReplyWithSIValue(ctx, gc, &val);
	}
}

static void _ResultSet_CompactReplyWithPoint
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue *v
) {
	ASSERT(SI_TYPE(*v) == T_POINT);
	RedisModule_ReplyWithArray(ctx, 2);

	_ResultSet_ReplyWithRoundedDouble(ctx, Point_lat(*v));
	_ResultSet_ReplyWithRoundedDouble(ctx, Point_lon(*v));
}

// For every column in the header, emit a 2-array containing the ColumnType enum
// followed by the column alias.
void ResultSet_ReplyWithCompactHeader
(
	RedisModuleCtx *ctx,
	const char **columns,
	uint *col_rec_map,
	void *pdata
) {
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

typedef struct {
	DataBlock *cells;
	bool cells_allocation;
} FormatterCompactCtx;

void *Formatter_Compact_CreatePData(void) {
	FormatterCompactCtx *ctx = rm_malloc(sizeof(FormatterCompactCtx));
	ctx->cells = DataBlock_New(16384, 1, sizeof(SIValue), NULL);
	ctx->cells_allocation = M_NONE;

	return ctx;
}

void Formatter_Compact_ProcessRow
(
	Record r,
	uint *columns_record_map,  //
	uint ncols,                // length of row
	void *pdata                // formatter's private data
) {
	FormatterCompactCtx *ctx = (FormatterCompactCtx*)pdata;

	DataBlock *cells = ctx->cells;

	// copy projected values from record to resultset
	for(int i = 0; i < ncols; i++) {
		int idx = columns_record_map[i];
		SIValue *cell = DataBlock_AllocateItem(cells, NULL);
		*cell = Record_Get(r, idx);
		ctx->cells_allocation |= SI_ALLOCATION(cell);
	}

	// remove entry from record in a second pass
	// this will ensure duplicated projections are not removed
	// too early, consider: MATCH (a) RETURN max(a.val), max(a.val)
	for(int i = 0; i < ncols; i++) {
		int idx = columns_record_map[i];
		Record_Remove(r, idx);
	}
}
// for every column in the header, emit a 2-array containing the ColumnType enum
// followed by the column alias
void Formatter_Compact_EmitHeader
(
	RedisModuleCtx *ctx,
	const char **columns
) {
	uint columns_len = array_len(columns);
	RedisModule_ReplyWithArray(ctx, columns_len);
	for(uint i = 0; i < columns_len; i++) {
		RedisModule_ReplyWithArray(ctx, 2);
		// because the types found in the first Record
		// do not necessarily inform the types in subsequent records
		// we will always set the column type as scalar
		ColumnType t = COLUMN_SCALAR;
		RedisModule_ReplyWithLongLong(ctx, t);

		// second, emit the identifier string associated with the column
		RedisModule_ReplyWithStringBuffer(ctx, columns[i], strlen(columns[i]));
	}
}

void Formatter_Compact_EmitData
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	uint ncols,
	void *pdata
) {
	DataBlock *cells = ((FormatterCompactCtx*)pdata)->cells;
	uint64_t nrows = DataBlock_ItemCount(cells) / ncols;

	RedisModule_ReplyWithArray(ctx, nrows);

	// for each row
	uint64_t idx = 0;
	for(uint64_t row = 0; row < nrows; row++) {
		// prepare return array sized to the number of RETURN entities
		RedisModule_ReplyWithArray(ctx, ncols);	

		// for each column
		for(uint64_t col = 0; col < ncols; col++) {
			SIValue *v = DataBlock_GetItem(cells, idx);
			// reply with array with space for type and value
			RedisModule_ReplyWithArray(ctx, 2); 
			_ResultSet_CompactReplyWithSIValue(ctx, gc, v);
			idx++;
		}
	}
}

void Formatter_Compact_FreePData
(
	void *pdata
) {
	if(pdata == NULL) {
		return;
	}

	FormatterCompactCtx *ctx = (FormatterCompactCtx*)pdata;

	DataBlock *cells = ctx->cells;
	bool cells_allocation = ctx->cells_allocation;

	if(cells_allocation & M_SELF) {
		uint64_t n = DataBlock_ItemCount(cells);
		for(uint64_t i = 0; i < n; i++) {
			SIValue *v = DataBlock_GetItem(cells, i);
			SIValue_Free(*v);
		}
	}

	DataBlock_Free(cells);
	rm_free(ctx);
}

