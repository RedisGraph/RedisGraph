/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graphdecodecontext.h"
#include "assert.h"
#include "../util/rmalloc.h"

inline GraphDecodeContext *GraphDecodeContext_New() {
	GraphDecodeContext *ctx = rm_malloc(sizeof(GraphDecodeContext));
	ctx->keys_processed = 0;
	return ctx;
}

inline void GraphDecodeContext_Reset(GraphDecodeContext *ctx) {
	assert(ctx);
	ctx->keys_processed = 0;
}

inline uint64_t GraphDecodeContext_GetProccessedKeyCount(const GraphDecodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed;
}

inline void GraphDecodeContext_IncreaseProcessedCount(GraphDecodeContext *ctx) {
	assert(ctx);
	ctx->keys_processed++;
}

inline void GraphDecodeContext_Free(GraphDecodeContext *ctx) {
	assert(ctx);
	rm_free(ctx);
}