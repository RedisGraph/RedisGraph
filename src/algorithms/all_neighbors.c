/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include "RG.h"
#include "all_neighbors.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

static void _AllNeighborsCtx_CollectNeighbors
(
	AllNeighborsCtx *ctx,
	EntityID id
) {
	ctx->current_level++;
	if(ctx->current_level == array_len(ctx->levels)) {
		RG_MatrixTupleIter iter = {0};
		RG_MatrixTupleIter_AttachRange(&iter, ctx->M, id, id);
		array_append(ctx->levels, iter);
	} else {
		RG_MatrixTupleIter_iterate_row(&ctx->levels[ctx->current_level], id);
	}
}

static bool _AllNeighborsCtx_Visited
(
	AllNeighborsCtx *ctx,
	EntityID id
) {
	uint count = ctx->current_level;

	for(uint i = 0; i < count; i++) {
		if(ctx->visited[i] == id) return true;
	}

	return false;
}

void AllNeighborsCtx_Reset
(
	AllNeighborsCtx *ctx,  // all neighbors context to reset
	EntityID src,          // source node from which to traverse
	RG_Matrix M,           // matrix describing connections
	uint minLen,           // minimum traversal depth
	uint maxLen            // maximum traversal depth
) {
	ASSERT(M             != NULL);
	ASSERT(src           != INVALID_ENTITY_ID);
	ASSERT(ctx           != NULL);
	ASSERT(ctx->levels   != NULL);
	ASSERT(ctx->visited  != NULL);

	ctx->M              =  M;
	ctx->src            =  src;
	ctx->minLen         =  minLen;
	ctx->maxLen         =  maxLen;
	ctx->first_pull     =  true;
	ctx->current_level  =  0;

	uint count = array_len(ctx->levels);
	for (uint i = 0; i < count; i++) {
		RG_MatrixTupleIter_detach(ctx->levels + i);
	}
	array_clear(ctx->levels);
	array_clear(ctx->visited);

	// Dummy iterator at level 0
	array_append(ctx->levels, (RG_MatrixTupleIter) {0});
}

AllNeighborsCtx *AllNeighborsCtx_New
(
	EntityID src,  // source node from which to traverse
	RG_Matrix M,   // matrix describing connections
	uint minLen,   // minimum traversal depth
	uint maxLen    // maximum traversal depth
) {
	ASSERT(M   != NULL);
	ASSERT(src != INVALID_ENTITY_ID);

	AllNeighborsCtx *ctx = rm_calloc(1, sizeof(AllNeighborsCtx));

	ctx->M              =  M;
	ctx->src            =  src;
	ctx->minLen         =  minLen;
	ctx->maxLen         =  maxLen;
	ctx->levels         =  array_new(RG_MatrixTupleIter, 1);
	ctx->visited        =  array_new(EntityID, 1);
	ctx->first_pull     =  true;
	ctx->current_level  =  0;

	// Dummy iterator at level 0
	array_append(ctx->levels, (RG_MatrixTupleIter) {0});

	return ctx;
}

EntityID AllNeighborsCtx_NextNeighbor
(
	AllNeighborsCtx *ctx
) {
	if(!ctx) return INVALID_ENTITY_ID;

	if(ctx->first_pull) {
		ASSERT(ctx->current_level == 0);
		ctx->first_pull = false;

		// update visited path, replace frontier with current node
		array_append(ctx->visited, ctx->src);

		// current_level >= ctx->minLen
		// see if we should expand further?
		if(ctx->current_level < ctx->maxLen) {
			// we can expand further
			_AllNeighborsCtx_CollectNeighbors(ctx, ctx->src);
		}

		if(ctx->minLen == 0) {
			return ctx->src;
		}
	}

	while(ctx->current_level > 0) {
		ASSERT(ctx->current_level < array_len(ctx->levels));
		RG_MatrixTupleIter *it = &ctx->levels[ctx->current_level];

		GrB_Index dest_id;
		GrB_Info info = RG_MatrixTupleIter_next_UINT64(it, NULL, &dest_id, NULL);

		if(info == GxB_EXHAUSTED) {
			// backtrack
			ctx->current_level--;
			array_pop(ctx->visited);
			continue;
		}

		// update visited path, replace frontier with current node
		array_append(ctx->visited, dest_id);

		if(ctx->current_level < ctx->minLen) {
			// continue traversing
			_AllNeighborsCtx_CollectNeighbors(ctx, dest_id);
			continue;
		}

		// current_level >= ctx->minLen
		// see if we should expand further?
		if(ctx->current_level < ctx->maxLen &&
		   !_AllNeighborsCtx_Visited(ctx, dest_id)) {
			// we can expand further
			_AllNeighborsCtx_CollectNeighbors(ctx, dest_id);
		}

		return dest_id;
	}

	// couldn't find a neighbor
	return INVALID_ENTITY_ID;
}

void AllNeighborsCtx_Free
(
	AllNeighborsCtx *ctx
) {
	if(!ctx) return;

	uint count = array_len(ctx->levels);
	for (uint i = 0; i < count; i++) {
		RG_MatrixTupleIter_detach(ctx->levels + i);
	}
	array_free(ctx->levels);
	array_free(ctx->visited);

	rm_free(ctx);
}

