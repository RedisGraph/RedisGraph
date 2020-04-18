/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "../graph/graphcontext.h"

typedef struct {
	GraphContext *gc;
	char *meta_key_name;
} GraphMetaContext;

// Create a new graph meta context.
GraphMetaContext *GraphMetaContext_New(GraphContext *gc, const char *meta_key_name);
// Free graph meta context memory.
void GraphMetaContext_Free(GraphMetaContext *ctx);

