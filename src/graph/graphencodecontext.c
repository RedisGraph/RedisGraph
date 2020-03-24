/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graphencodecontext.h"
#include "assert.h"
#include "../util/rmalloc.h"

inline GraphEncodeContext *GraphEncodeContext_New(unsigned long key_count) {
	GraphEncodeContext *ctx = rm_malloc(sizeof(GraphEncodeContext));
	ctx->keys_count = key_count;
	ctx->keys_processed = 0;
	ctx->phase = RESET;
}

inline void GraphEncodeContext_Reset(GraphEncodeContext *ctx) {
	assert(ctx);
	ctx->phase = RESET;
	ctx->keys_processed = 0;
}

inline EncodePhase GraphEncodeContext_GetEncodePhase(GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->phase;
}

inline void GraphEncodeContext_SetEncodePhase(GraphEncodeContext *ctx, EncodePhase phase) {
	assert(ctx);
	ctx->phase = phase;
}

inline unsigned long GraphEncodeContext_GetKeyCount(GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->keys_count;
}

inline unsigned long GraphEncodeContext_GetProccessedKeyCount(GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed;
}

inline bool GraphEncodeContext_Finished(GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed == ctx->keys_count;
}

inline void GraphEncodeContext_IncreaseKeyCount(GraphEncodeContext *ctx, unsigned long delta) {
	assert(ctx);
	ctx->keys_count += delta;
}

inline void GraphEncodeContext_DecreaseKeyCount(GraphEncodeContext *ctx, unsigned long delta) {
	assert(ctx);
	ctx->keys_count -= delta;
}

inline void GraphEncodeContext_IncreaseProcessedCount(GraphEncodeContext *ctx) {
	assert(ctx);
	assert(ctx->keys_processed < ctx->keys_count);
}


inline void GraphEncodeContext_Free(GraphEncodeContext *ctx) {
	if(ctx) rm_free(ctx);
}