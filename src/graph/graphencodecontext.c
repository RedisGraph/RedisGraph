/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "graphencodecontext.h"
#include "assert.h"
#include "../util/rmalloc.h"

inline GraphEncodeContext *GraphEncodeContext_New(uint64_t key_count) {
	GraphEncodeContext *ctx = rm_malloc(sizeof(GraphEncodeContext));
	ctx->keys_count = key_count;
	GraphEncodeContext_Reset(ctx);
	return ctx;
}

inline void GraphEncodeContext_Reset(GraphEncodeContext *ctx) {
	assert(ctx);
	ctx->phase = RESET;
	ctx->keys_processed = 0;
	ctx->processed_nodes = 0;
	ctx->processed_edges = 0;
	ctx->processed_deleted_nodes = 0;
	ctx->processed_deleted_edges = 0;
	ctx->datablock_iterator = NULL;
}

inline EncodePhase GraphEncodeContext_GetEncodePhase(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->phase;
}

inline void GraphEncodeContext_SetEncodePhase(GraphEncodeContext *ctx, EncodePhase phase) {
	assert(ctx);
	ctx->phase = phase;
}

inline uint64_t GraphEncodeContext_GetKeyCount(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->keys_count;
}

inline uint64_t GraphEncodeContext_GetProccessedKeyCount(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed;
}

inline uint64_t GraphEncodeContext_GetProccessedNodes(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->processed_nodes;
}

inline void GraphEncodeContext_SetProcessedNodes(GraphEncodeContext *ctx, uint64_t nodes) {
	assert(ctx);
	ctx->processed_nodes = nodes;
}

inline uint64_t GraphEncodeContext_GetProccessedDeletedNodes(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->processed_deleted_nodes;
}

inline void GraphEncodeContext_SetProcessedDeletedNodes(GraphEncodeContext *ctx,
														uint64_t deleted_nodes) {
	assert(ctx);
	ctx->processed_deleted_nodes = deleted_nodes;
}

inline uint64_t GraphEncodeContext_GetProccessedEdges(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->processed_edges;
}

inline void GraphEncodeContext_SetProcessedEdges(GraphEncodeContext *ctx, uint64_t edges) {
	assert(ctx);
	ctx->processed_edges = edges;
}

inline uint64_t GraphEncodeContext_GetProccessedDeletedEdges(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->processed_deleted_edges;
}

inline void GraphEncodeContext_SetProcessedDeletedEdges(GraphEncodeContext *ctx,
														uint64_t deleted_edges) {
	assert(ctx);
	ctx->processed_deleted_edges = deleted_edges;
}

inline DataBlockIterator *GraphEncodeContext_GetDatablockIterator(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->datablock_iterator;
}

inline void GraphEncodeContext_SetDatablockIterator(GraphEncodeContext *ctx,
													DataBlockIterator *iter) {
	assert(ctx);
	ctx->datablock_iterator = iter;
}

inline bool GraphEncodeContext_Finished(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed == ctx->keys_count;
}

inline void GraphEncodeContext_IncreaseKeyCount(GraphEncodeContext *ctx, uint64_t delta) {
	assert(ctx);
	ctx->keys_count += delta;
}

inline void GraphEncodeContext_DecreaseKeyCount(GraphEncodeContext *ctx, uint64_t delta) {
	assert(ctx);
	ctx->keys_count -= delta;
}

inline void GraphEncodeContext_IncreaseProcessedCount(GraphEncodeContext *ctx) {
	assert(ctx);
	assert(ctx->keys_processed < ctx->keys_count);
	ctx->keys_processed++;
}

inline void GraphEncodeContext_Free(GraphEncodeContext *ctx) {
	if(ctx) rm_free(ctx);
}
