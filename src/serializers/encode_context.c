/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "encode_context.h"
#include "assert.h"
#include "../util/rmalloc.h"
#include "../util/rax_extensions.h"

inline GraphEncodeContext *GraphEncodeContext_New() {
	GraphEncodeContext *ctx = rm_malloc(sizeof(GraphEncodeContext));
	ctx->meta_keys_count = 0;
	GraphEncodeContext_Reset(ctx);
	return ctx;
}

inline void GraphEncodeContext_Reset(GraphEncodeContext *ctx) {
	assert(ctx);
	ctx->state = INIT;
	ctx->keys_processed = 0;
	ctx->processed_entities = 0;
	ctx->datablock_iterator = NULL;
	ctx->current_relation_matrix_id = 0;
	ctx->matrix_tuple_iterator = NULL;
	ctx->multiple_edges_array = NULL;
	ctx->multiple_edges_current_index = 0;
	ctx->multiple_edges_src_id = 0;
	ctx->multiple_edges_dest_id = 0;
}

inline EncodeState GraphEncodeContext_GetEncodeState(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->state;
}

inline void GraphEncodeContext_SetEncodeState(GraphEncodeContext *ctx, EncodeState state) {
	assert(ctx);
	ctx->state = state;
}

inline uint64_t GraphEncodeContext_GetKeyCount(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->meta_keys_count + 1;
}

void GraphEncodeContext_SetMetaKeysCount(GraphEncodeContext *ctx, uint64_t meta_keys_count) {
	assert(ctx);
	ctx->meta_keys_count = meta_keys_count;
}

inline uint64_t GraphEncodeContext_GetProcessedKeyCount(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed;
}

inline uint64_t GraphEncodeContext_GetProcessedEntitiesCount(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->processed_entities;
}

inline void GraphEncodeContext_SetProcessedEntitiesCount(GraphEncodeContext *ctx,
														 uint64_t processed_entities) {
	assert(ctx);
	ctx->processed_entities = processed_entities;
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

inline uint GraphEncodeContext_GetCurrentRelationID(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->current_relation_matrix_id;
}

inline void GraphEncodeContext_SetCurrentRelationID(GraphEncodeContext *ctx,
													uint current_relation_matrix_id) {
	assert(ctx);
	ctx->current_relation_matrix_id = current_relation_matrix_id;
}

inline GxB_MatrixTupleIter *GraphEncodeContext_GetMatrixTupleIterator(
	const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->matrix_tuple_iterator;
}

inline void GraphEncodeContext_SetMatrixTupleIterator(GraphEncodeContext *ctx,
													  GxB_MatrixTupleIter *iter) {
	assert(ctx);
	ctx->matrix_tuple_iterator = iter;
}

inline void GraphEncodeContext_SetMutipleEdgesArray(GraphEncodeContext *ctx, EdgeID *edges,
													uint current_index, NodeID src, NodeID dest) {
	assert(ctx);
	ctx->multiple_edges_array = edges;
	ctx->multiple_edges_current_index = current_index;
	ctx->multiple_edges_src_id = src;
	ctx->multiple_edges_dest_id = dest;
}

inline EdgeID *GraphEncodeContext_GetMultipleEdgesArray(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->multiple_edges_array;
}

inline uint GraphEncodeContext_GetMultipleEdgesCurrentIndex(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->multiple_edges_current_index;
}


inline NodeID GraphEncodeContext_GetMultipleEdgesSourceNode(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->multiple_edges_src_id;
}

inline NodeID GraphEncodeContext_GetMultipleEdgesDestinationNode(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->multiple_edges_dest_id;
}

inline bool GraphEncodeContext_Finished(const GraphEncodeContext *ctx) {
	assert(ctx);
	return ctx->keys_processed == GraphEncodeContext_GetKeyCount(ctx);
}

inline void GraphEncodeContext_IncreaseProcessedCount(GraphEncodeContext *ctx) {
	assert(ctx);
	assert(ctx->keys_processed < GraphEncodeContext_GetKeyCount(ctx));
	ctx->keys_processed++;
}

inline void GraphEncodeContext_Free(GraphEncodeContext *ctx) {
	if(ctx) rm_free(ctx);
}
