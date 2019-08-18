/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "../../redismodule.h"
#include "../../execution_plan/record.h"
#include "../../graph/graphcontext.h"
#include "../../graph/query_graph.h"

typedef enum {
	COLUMN_UNKNOWN = 0,
	COLUMN_SCALAR = 1,
	COLUMN_NODE = 2,
	COLUMN_RELATION = 3,
} ColumnTypeUser;

typedef enum {
	PROPERTY_UNKNOWN = 0,
	PROPERTY_NULL = 1,
	PROPERTY_STRING = 2,
	PROPERTY_INTEGER = 3,
	PROPERTY_BOOLEAN = 4,
	PROPERTY_DOUBLE = 5,
    PROPERTY_ERROR = 6,
} PropertyTypeUser;

// Typedef for header formatters.
typedef void (*EmitHeaderFunc)(RedisModuleCtx *ctx, const char **columns, const Record r);

// Typedef for record formatters.
typedef void (*EmitRecordFunc)(RedisModuleCtx *ctx, GraphContext *gc, const Record r);

typedef struct {
	EmitRecordFunc EmitRecord;
	EmitHeaderFunc EmitHeader;
} ResultSetFormatter;

/* Redis prints doubles with up to 17 digits of precision, which captures
 * the inaccuracy of many floating-point numbers (such as 0.1).
 * By using the %g format and a precision of 15 significant digits, we avoid many
 * awkward representations like RETURN 0.1 emitting "0.10000000000000001",
 * though we're still subject to many of the typical issues with floating-point error. */
static inline void _ResultSet_ReplyWithRoundedDouble(RedisModuleCtx *ctx, double d) {
	// Get length required to print number
	int len = snprintf(NULL, 0, "%.15g", d);
	char str[len + 1]; // TODO a reusable buffer would be far preferable
	sprintf(str, "%.15g", d);
	// Output string-formatted number
	RedisModule_ReplyWithStringBuffer(ctx, str, len);
}
