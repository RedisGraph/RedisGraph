/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// formatter for verbose (human-readable) replies
void ResultSet_ReplyWithVerboseHeader
(
	RedisModuleCtx *ctx,
	const char **columns,
	uint *col_rec_map,
	void *pdata
);

void ResultSet_EmitVerboseRow
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue **row,
	uint numcols,
	void *pdata
);

void *ResultSet_CreateVerbosePData(void);

void ResultSet_FreeVerbosePData
(
	void *pdata
);

