/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

void *Formatter_Verbose_CreatePData(void);

void Formatter_Verbose_ProcessRow
(
	Record r,                  //
	uint *columns_record_map,  //
	uint ncols,                // length of row
	void *pdata                // formatter's private data
);

void Formatter_Verbose_EmitHeader
(
	RedisModuleCtx *ctx,
	const char **columns
);

void Formatter_Verbose_EmitData
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	uint ncols,
	void *pdata
);

void Formatter_Verbose_FreePData
(
	void *pdata
);

