/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Formatter for compact (client-parsed) replies
void ResultSet_EmitNOPHeader
(
	RedisModuleCtx *ctx,
	const char **columns,
	uint *col_rec_map,
	void *pdata
);

void ResultSet_EmitNOPRow
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	SIValue **row,
	uint numcols,
	void *pdata
);

void *ResultSet_CreateNOPPData(void);

void ResultSet_FreeNOPPData
(
	void *pdata
);

