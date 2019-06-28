/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Formatter for verbose (human-readable) replies
void ResultSet_EmitVerboseRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols);
void ResultSet_ReplyWithVerboseHeader(RedisModuleCtx *ctx, const QueryGraph* qg, AR_ExpNode **exps);
