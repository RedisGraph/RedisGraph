/*
 * Copyright 2018-2019 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Apache License, Version 2.0,
 * modified with the Commons Clause restriction.
 */

#ifndef __RESULTSET_RECORDS_H__
#define __RESULTSET_RECORDS_H__

#include "../redismodule.h"
#include "../execution_plan/record.h"
#include "../graph/graphcontext.h"

typedef enum {
    PROPERTY_UNKNOWN,
    PROPERTY_NULL,
    // PROPERTY_STRING,
    PROPERTY_STRING_OFFSET,
    PROPERTY_INTEGER,
    PROPERTY_BOOLEAN,
    PROPERTY_DOUBLE,
} PropertyTypeUser;

typedef void (*EmitRecordFunc)(RedisModuleCtx *ctx, GraphContext *gc, const Record r, unsigned int numcols);

// Must be explicitly declared
void ResultSet_ReplyWithSIValue(RedisModuleCtx *ctx, GraphContext *gc, const SIValue v, bool print_type);

EmitRecordFunc ResultSet_SetReplyFormatter(bool compact);

#endif
