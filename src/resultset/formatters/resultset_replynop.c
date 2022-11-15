/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "resultset_formatters.h"
#include "../../util/arr.h"

void ResultSet_EmitNOPHeader(RedisModuleCtx *ctx, const char **columns,
		uint *col_rec_map) {
}

void ResultSet_EmitNOPRow(RedisModuleCtx *ctx, GraphContext *gc, SIValue **row,
		uint numcols) {
}

