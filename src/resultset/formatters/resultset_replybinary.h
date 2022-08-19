/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// formatter for binary replies
void ResultSet_ReplyWithBinaryHeader
(
	RedisModuleCtx *ctx,
	const char **columns,
	uint *col_rec_map,
	void *pdata
);

void ResultSet_EmitBinaryRow
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue **row,
	uint numcols,
	void *pdata
);

void *ResultSet_CreateBinaryPData(void);

void ResultSet_FreeBinaryPData
(
	void *pdata
);

