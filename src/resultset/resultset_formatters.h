/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

#include "resultset_header.h"
#include "../redismodule.h"
#include "../execution_plan/record.h"
#include "../graph/graphcontext.h"
#include "../graph/query_graph.h"

typedef enum {
    COLUMN_UNKNOWN,
    COLUMN_SCALAR,
    COLUMN_NODE,
    COLUMN_RELATION,
} ColumnTypeUser;

typedef enum {
    PROPERTY_UNKNOWN,
    PROPERTY_NULL,
    PROPERTY_STRING,
    PROPERTY_INTEGER,
    PROPERTY_BOOLEAN,
    PROPERTY_DOUBLE,
} PropertyTypeUser;

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

// Typedef for record formatters
typedef void (*EmitRecordFunc)(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols);

// Formatter for verbose (human-readable) replies
void ResultSet_EmitVerboseRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols);

// Formatter for compact (client-parsed) replies
void ResultSet_EmitCompactRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols);

// Formatter for verbose header reply
void ResultSet_ReplyWithVerboseHeader(RedisModuleCtx *ctx, const ResultSetHeader *header);

// Formatter for compact header reply
void ResultSet_ReplyWithCompactHeader(RedisModuleCtx *ctx, const ResultSetHeader *header, TrieMap *entities);
