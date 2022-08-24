/*
 * Copyright 2018-2022 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#include "resultset_formatters.h"
#include "../../util/arr.h"

void* Formatter_NOP_CreatePData(void)
{
	return NULL;
}

void Formatter_NOP_ProcessRow
(
	Record r,
	uint *columns_record_map,
	uint ncols,
	void *pdata
) {

}

void Formatter_NOP_EmitHeader
(
	RedisModuleCtx *ctx,
	const char **columns
) {

}

void Formatter_NOP_EmitData
(
	RedisModuleCtx *ctx,
	GraphContext *gc,
	uint ncols,
	void *pdata
) {

}

void Formatter_NOP_FreePData
(
	void *pdata
) {

}

