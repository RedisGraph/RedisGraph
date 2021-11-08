/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "decode_context.h"
#include "../RG.h"
#include "../util/rmalloc.h"
#include "../util/rax_extensions.h"

GraphDecodeContext *GraphDecodeContext_New() {
	GraphDecodeContext *ctx = rm_malloc(sizeof(GraphDecodeContext));
	ctx->keys_processed = 0;
	ctx->graph_keys_count = 1;
	ctx->meta_keys = raxNew();
	return ctx;
}

void GraphDecodeContext_Reset(GraphDecodeContext *ctx) {
	ASSERT(ctx);
	ctx->keys_processed = 0;
	ctx->graph_keys_count = 1;
}

void GraphDecodeContext_SetKeyCount(GraphDecodeContext *ctx, uint64_t key_count) {
	ASSERT(ctx);
	ctx->graph_keys_count = key_count;
}

uint64_t GraphDecodeContext_GetKeyCount(const GraphDecodeContext *ctx) {
	ASSERT(ctx);
	return ctx->graph_keys_count;
}

void GraphDecodeContext_AddMetaKey(GraphDecodeContext *ctx, const char *key) {
	ASSERT(ctx);
	raxInsert(ctx->meta_keys, (unsigned char *)key, strlen(key), NULL, NULL);
}

unsigned char **GraphDecodeContext_GetMetaKeys(const GraphDecodeContext *ctx) {
	ASSERT(ctx);
	return raxKeys(ctx->meta_keys);
}

void GraphDecodeContext_ClearMetaKeys(GraphDecodeContext *ctx) {
	ASSERT(ctx);
	raxFree(ctx->meta_keys);
	ctx->meta_keys = raxNew();
}

// Returns if the the number of processed keys is equal to the total number of graph keys.
bool GraphDecodeContext_Finished(const GraphDecodeContext *ctx) {
	ASSERT(ctx);
	return ctx->keys_processed == ctx->graph_keys_count;
}

bool GraphDecodeContext_Started(const GraphDecodeContext *ctx) {
	ASSERT(ctx);
	return ctx->keys_processed > 0;
}

void GraphDecodeContext_IncreaseProcessedKeyCount(GraphDecodeContext *ctx) {
	ASSERT(ctx);
	ctx->keys_processed++;
}

bool GraphDecodeContext_GetProcessedKeyCount(const GraphDecodeContext *ctx) {
	ASSERT(ctx);
	return ctx->keys_processed;
}

void GraphDecodeContext_Free(GraphDecodeContext *ctx) {
	if(ctx) {
		raxFree(ctx->meta_keys);
		rm_free(ctx);
	}
}

