/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_context.h"
#include "assert.h"
#include "../util/rmalloc.h"
#include "../util/rax_extensions.h"

inline GraphDecodeContext *GraphDecodeContext_New() {
	GraphDecodeContext *ctx = rm_malloc(sizeof(GraphDecodeContext));
	ctx->keys_processed = 0;
	ctx->graph_keys_count = 1;
	ctx->meta_keys = raxNew();
	return ctx;
}

inline void GraphDecodeContext_Reset(GraphDecodeContext *ctx) {
	assert(ctx);
	ctx->keys_processed = 0;
}

inline void GraphDecodeContext_SetKeyCount(GraphDecodeContext *ctx, uint64_t key_count) {
	assert(ctx);
	ctx->graph_keys_count = key_count;
}

inline uint64_t GraphDecodeContext_GetKeyCount(const GraphDecodeContext *ctx) {
	assert(ctx);
	return ctx->graph_keys_count;
}

inline void GraphDecodeContext_AddMetaKey(GraphDecodeContext *ctx, const char *key) {
	assert(ctx);
	raxInsert(ctx->meta_keys, (unsigned char *)key, strlen(key), NULL, NULL);
}

inline unsigned char **GraphDecodeContext_GetMetaKeys(const GraphDecodeContext *ctx) {
	assert(ctx);
	return raxKeys(ctx->meta_keys);
}

inline void GraphDecodeContext_ClearMetaKeys(GraphDecodeContext *ctx) {
	assert(ctx);
	raxClear(ctx->meta_keys);
}

// Returns if the the number of processed keys is equal to the total number of graph keys.
inline bool GraphDecodeContext_Finished(const GraphDecodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed == ctx->graph_keys_count;
}

inline void GraphDecodeContext_IncreaseProcessedKeyCount(GraphDecodeContext *ctx) {
	assert(ctx);
	ctx->keys_processed++;
}

inline bool GraphDecodeContext_GetProcessedKeyCount(const GraphDecodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed;
}

inline void GraphDecodeContext_Free(GraphDecodeContext *ctx) {
	if(ctx) {
		raxFree(ctx->meta_keys);
		rm_free(ctx);
	}
}
