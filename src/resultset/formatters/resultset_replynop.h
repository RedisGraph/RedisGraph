/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once

// Formatter for compact (client-parsed) replies
void ResultSet_EmitNOPHeader(RedisModuleCtx *ctx, const char **columns, const Record r, uint *col_rec_map);
void ResultSet_EmitNOPRecord(RedisModuleCtx *ctx, GraphContext *gc, const Record r, uint numcols, uint *col_rec_map);
