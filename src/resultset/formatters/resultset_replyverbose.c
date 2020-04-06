/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "../../util/arr.h"
#include "../../datatypes/path/sipath.h"

// Forward declarations.
static void _ResultSet_VerboseReplyWithNode(RedisModuleCtx *ctx, GraphContext *gc, Node *n);
static void _ResultSet_VerboseReplyWithEdge(RedisModuleCtx *ctx, GraphContext *gc, Edge *e);
static void _ResultSet_VerboseReplyWithArray(RedisModuleCtx *ctx, SIValue array);
static void _ResultSet_VerboseReplyWithPath(RedisModuleCtx *ctx, SIValue path);

/* This function has handling for all SIValue scalar types.
 * The current RESP protocol only has unique support for strings, 8-byte integers,
 * and NULL values. */
static void _ResultSet_VerboseReplyWithSIValue(RedisModuleCtx *ctx, GraphContext *gc,
											   const SIValue v) {
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
	case T_NULL:
		RedisModule_ReplyWithNull(ctx);
		return;
	case T_NODE:
		_ResultSet_VerboseReplyWithNode(ctx, gc, v.ptrval);
		return;
	case T_EDGE:
		_ResultSet_VerboseReplyWithEdge(ctx, gc, v.ptrval);
		return;
	case T_ARRAY:
		_ResultSet_VerboseReplyWithArray(ctx, v);
		return;
	case T_PATH:
		_ResultSet_VerboseReplyWithPath(ctx, v);
		return;
	default:
		assert("Unhandled value type" && false);
	}
}

static void _ResultSet_VerboseReplyWithProperties(RedisModuleCtx *ctx, GraphContext *gc,
												  const GraphEntity *e) {
	int prop_count = ENTITY_PROP_COUNT(e);
	RedisModule_ReplyWithArray(ctx, prop_count);
	// Iterate over all properties stored on entity
	for(int i = 0; i < prop_count; i ++) {
		RedisModule_ReplyWithArray(ctx, 2);
		EntityProperty prop = ENTITY_PROPS(e)[i];
		// Emit the actual string
		const char *prop_str = GraphContext_GetAttributeString(gc, prop.id);
		RedisModule_ReplyWithStringBuffer(ctx, prop_str, strlen(prop_str));
		// Emit the value
		_ResultSet_VerboseReplyWithSIValue(ctx, gc, prop.value);
	}
}

static void _ResultSet_VerboseReplyWithNode(RedisModuleCtx *ctx, GraphContext *gc, Node *n) {
	/*  Verbose node reply format:
	 *  [
	 *      ["id", Node ID (integer)]
	 *      ["label", [label (string or NULL)]]
	 *      ["properties", [[name, value, value type] X N]
	 *  ]
	 */
	// 3 top-level entities in node reply
	RedisModule_ReplyWithArray(ctx, 3);

	// ["id", id (integer)]
	EntityID id = ENTITY_GET_ID(n);
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithStringBuffer(ctx, "id", 2);
	RedisModule_ReplyWithLongLong(ctx, id);

	// ["labels", [label (string)]]
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithStringBuffer(ctx, "labels", 6);
	// Print label in nested array for multi-label support
	// Retrieve label
	// TODO Make a more efficient lookup for this string
	const char *label = GraphContext_GetNodeLabel(gc, n);
	if(label == NULL) {
		// Emit an empty array for unlabeled nodes
		RedisModule_ReplyWithArray(ctx, 0);
	} else {
		RedisModule_ReplyWithArray(ctx, 1);
		RedisModule_ReplyWithStringBuffer(ctx, label, strlen(label));
	}

	// [properties, [properties]]
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithStringBuffer(ctx, "properties", 10);
	_ResultSet_VerboseReplyWithProperties(ctx, gc, (GraphEntity *)n);
}

static void _ResultSet_VerboseReplyWithEdge(RedisModuleCtx *ctx, GraphContext *gc, Edge *e) {
	/*  Edge reply format:
	 *  [
	 *      ["id", Edge ID (integer)]
	 *      ["type", relation type (string)]
	 *      ["src_node", source node ID (integer)]
	 *      ["dest_node", destination node ID (integer)]
	 *      ["properties", [[name, value, value type] X N]
	 *  ]
	 */
	// 5 top-level entities in edge reply
	RedisModule_ReplyWithArray(ctx, 5);

	// ["id", id (integer)]
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithStringBuffer(ctx, "id", 2);
	RedisModule_ReplyWithLongLong(ctx, ENTITY_GET_ID(e));

	// ["type", type (string)]
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithStringBuffer(ctx, "type", 4);
	// Retrieve relation type
	Schema *s = GraphContext_GetSchemaByID(gc, Edge_GetRelationID(e), SCHEMA_EDGE);
	const char *reltype = Schema_GetName(s);
	RedisModule_ReplyWithStringBuffer(ctx, reltype, strlen(reltype));

	// ["src_node", srcNodeID (integer)]
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithStringBuffer(ctx, "src_node", 8);
	RedisModule_ReplyWithLongLong(ctx, Edge_GetSrcNodeID(e));

	// ["dest_node", destNodeID (integer)]
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithStringBuffer(ctx, "dest_node", 9);
	RedisModule_ReplyWithLongLong(ctx, Edge_GetDestNodeID(e));

	// [properties, [properties]]
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithStringBuffer(ctx, "properties", 10);
	_ResultSet_VerboseReplyWithProperties(ctx, gc, (GraphEntity *)e);
}

static void _ResultSet_VerboseReplyWithArray(RedisModuleCtx *ctx, SIValue array) {
	size_t bufferLen = 512;
	char *str = rm_calloc(bufferLen, sizeof(char));
	size_t bytesWrriten = 0;
	SIValue_ToString(array, &str, &bufferLen, &bytesWrriten);
	RedisModule_ReplyWithStringBuffer(ctx, str, bytesWrriten);
	rm_free(str);
}

static void _ResultSet_VerboseReplyWithPath(RedisModuleCtx *ctx, SIValue path) {
	SIValue path_array = SIPath_ToList(path);
	_ResultSet_VerboseReplyWithArray(ctx, path_array);
	SIValue_Free(path_array);
}

void ResultSet_EmitVerboseRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r,
								 uint numcols, uint *col_rec_map) {
	// Prepare return array sized to the number of RETURN entities
	RedisModule_ReplyWithArray(ctx, numcols);

	for(int i = 0; i < numcols; i++) {
		uint idx = col_rec_map[i];
		switch(Record_GetType(r, idx)) {
		case REC_TYPE_NODE:
			_ResultSet_VerboseReplyWithNode(ctx, gc, Record_GetNode(r, idx));
			break;
		case REC_TYPE_EDGE:
			_ResultSet_VerboseReplyWithEdge(ctx, gc, Record_GetEdge(r, idx));
			break;
		default:
			_ResultSet_VerboseReplyWithSIValue(ctx, gc, Record_Get(r, idx));
		}
	}
}

// Emit the alias or descriptor for each column in the header.
void ResultSet_ReplyWithVerboseHeader(RedisModuleCtx *ctx, const char **columns,
									  const Record unused, uint *col_rec_map) {
	uint columns_len = array_len(columns);
	RedisModule_ReplyWithArray(ctx, columns_len);
	for(uint i = 0; i < columns_len; i++) {
		// Emit the identifier string associated with the column
		RedisModule_ReplyWithStringBuffer(ctx, columns[i], strlen(columns[i]));
	}
}

