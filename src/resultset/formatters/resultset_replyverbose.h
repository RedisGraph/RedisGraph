/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Formatter for verbose (human-readable) replies
void ResultSet_EmitVerboseRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r,
								 uint numcols);
void ResultSet_ReplyWithVerboseHeader(RedisModuleCtx *ctx, const char **columns, const Record r);

