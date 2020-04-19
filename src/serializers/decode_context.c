/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_context.h"
#include "assert.h"
#include "../util/rmalloc.h"

inline GraphDecodeContext *GraphDecodeContext_New() {
	GraphDecodeContext *ctx = rm_malloc(sizeof(GraphDecodeContext));
	ctx->keys_processed = 0;
	ctx->graph_decoding = false;
	ctx->graph_keys_count = 1;
	return ctx;
}

inline void GraphDecodeContext_Reset(GraphDecodeContext *ctx) {
	assert(ctx);
	ctx->keys_processed = 0;
	ctx->graph_decoding = false;
}

inline void GraphDecodeContext_SetKeyCount(GraphDecodeContext *ctx, uint64_t key_count) {
	assert(ctx);
	ctx->graph_keys_count = key_count;
}

inline uint64_t GraphDecodeContext_GetKeyCount(const GraphDecodeContext *ctx) {
	assert(ctx);
	return ctx->graph_keys_count;
}

inline void GraphDecodeContext_SetGraphInDecode(GraphDecodeContext *ctx) {
	assert(ctx);
	ctx->graph_decoding = true;
}

inline bool GraphDecodeContext_IsGraphInDecode(GraphDecodeContext *ctx) {
	assert(ctx);
	return ctx->graph_decoding;
}

// Returns if the the number of processed keys is equal to the total number of graph keys.
inline bool GraphDecodeContext_Finished(const GraphDecodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed == ctx->graph_keys_count;
}

inline void GraphDecodeContext_IncreaseProcessedCount(GraphDecodeContext *ctx) {
	assert(ctx);
	ctx->keys_processed++;
}

inline void GraphDecodeContext_Free(GraphDecodeContext *ctx) {
	assert(ctx);
	rm_free(ctx);
}
