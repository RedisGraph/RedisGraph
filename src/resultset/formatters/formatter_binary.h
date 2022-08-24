/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

void *Formatter_Binary_CreatePData(void);

void Formatter_Binary_ProcessRow
(
	Record r,
	uint *columns_record_map,  //
	uint numcols,              // length of row
	void *pdata                // formatter's private data
);

void Formatter_Binary_EmitHeader
(
	RedisModuleCtx *ctx,
	const char **columns
);

void Formatter_Binary_EmitData
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	uint ncols,
	void *pdata
);

void Formatter_Binary_FreePData
(
	void *pdata
);

