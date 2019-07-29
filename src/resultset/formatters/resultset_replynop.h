/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Formatter for compact (client-parsed) replies
void ResultSet_EmitNOPHeader(RedisModuleCtx *ctx, const QueryGraph *qg, AR_ExpNode **exps);
void ResultSet_EmitNOPRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r,
							 unsigned int numcols);
