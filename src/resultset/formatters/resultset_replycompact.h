/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Formatter for compact (client-parsed) replies
void ResultSet_EmitCompactRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols);
void ResultSet_ReplyWithCompactHeader(RedisModuleCtx *ctx, const ResultSetHeader *header, void *data);
