/*
 * Copyright 2018-2021 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Formatter for compact (client-parsed) replies
void ResultSet_ReplyWithCompactHeader(RedisModuleCtx *ctx, const char **columns);

void ResultSet_EmitCompactRow(RedisModuleCtx *ctx, GraphContext *gc,
							  SIValue **row, uint numcols);

