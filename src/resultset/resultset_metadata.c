/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset.h"
#include "../util/arr.h"
#include "../query_ctx.h"

static inline void _ResultSet_ReplyWithVersion(RedisModuleCtx *ctx, GraphContext *gc) {
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithCString(ctx, "version");
	uint version = GraphContext_GetVersion(gc);
	RedisModule_ReplyWithLongLong(ctx, version);
}

static void _ResultSet_ReplyWithSchemaNames(RedisModuleCtx *ctx, Schema **schemas) {
	uint count = array_len(schemas);
	RedisModule_ReplyWithArray(ctx, count);
	for(uint i = 0; i < count; i++) {
		RedisModule_ReplyWithArray(ctx, 2);
		RedisModule_ReplyWithLongLong(ctx, VALUE_STRING);
		const char *name = schemas[i]->name;
		RedisModule_ReplyWithCString(ctx, name);
	}
}

static inline void _ResultSet_ReplyWithLabels(RedisModuleCtx *ctx, GraphContext *gc) {
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithCString(ctx, "labels");
	_ResultSet_ReplyWithSchemaNames(ctx, gc->node_schemas);
}

static inline void _ResultSet_ReplyWithReltypes(RedisModuleCtx *ctx, GraphContext *gc) {
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithCString(ctx, "relationship types");
	_ResultSet_ReplyWithSchemaNames(ctx, gc->relation_schemas);
}

static inline void _ResultSet_ReplyWithProperties(RedisModuleCtx *ctx, GraphContext *gc) {
	RedisModule_ReplyWithArray(ctx, 2);
	RedisModule_ReplyWithCString(ctx, "property keys");
	uint count = array_len(gc->string_mapping);
	RedisModule_ReplyWithArray(ctx, count);
	for(uint i = 0; i < count; i++) {
		RedisModule_ReplyWithArray(ctx, 2);
		RedisModule_ReplyWithLongLong(ctx, VALUE_STRING);
		RedisModule_ReplyWithCString(ctx, gc->string_mapping[i]);
	}
}

/* Emit metadata of the form:
 * [
 *   ["version", VERSION],
 *   ["labels", [[VALUE_STRING, "label_1"] ... ]],
 *   ["relationship types ", [[VALUE_STRING, "reltype_1"] ... ]],
 *   ["property keys", [[VALUE_STRING, "prop_1"] ... ]]
 * ]
 */
void ResultSet_ReplyWithMetadata(ResultSet *set) {
	GraphContext *gc = QueryCtx_GetGraphCtx();
	// Reply with a 4-array of version, labels, reltypes, and property keys.
	RedisModule_ReplyWithArray(set->ctx, 4);

	_ResultSet_ReplyWithVersion(set->ctx, gc);
	_ResultSet_ReplyWithLabels(set->ctx, gc);
	_ResultSet_ReplyWithReltypes(set->ctx, gc);
	_ResultSet_ReplyWithProperties(set->ctx, gc);
}

