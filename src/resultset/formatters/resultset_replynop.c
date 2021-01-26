/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "../../util/arr.h"

void ResultSet_EmitNOPHeader(RedisModuleCtx *ctx, const char **columns,
							 const Record r, uint *col_rec_map) {
}

void ResultSet_EmitNOPRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, uint numcols,
							 uint *col_rec_map) {
}

void ResultSet_EmitNOPRow(RedisModuleCtx *ctx, GraphContext *gc, SIValue **row,
		uint numcols) {
}

