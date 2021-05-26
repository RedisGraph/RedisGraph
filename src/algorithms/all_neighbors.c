/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "all_neighbors.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

static void _AllNeighbotsCtx_CollectNeighbors
(
	AllNeighborsCtx *ctx,
	EntityID id
) {
	GxB_MatrixTupleIter *iter;
	if(ctx->current_level == array_len(ctx->levels)) {
		GxB_MatrixTupleIter_new(&iter, ctx->M);
		ctx->levels = array_append(ctx->levels, iter);
	} else {
		iter = ctx->levels[ctx->current_level];
	}

	GxB_MatrixTupleIter_iterate_row(iter, id);
}

static bool _AllNeighbotsCtx_Visited
(
	AllNeighborsCtx *ctx,
	EntityID id
) {
	uint count = ctx->current_level;

	for(int i = 0; i <= count; i++) {
		if(ctx->visited[i] == id) return true;
	}

	return false;
}

AllNeighborsCtx *AllNeighborsCtx_New
(
	EntityID src,  // source node from which to traverse
	GrB_Matrix M,  // matrix describing connections
	uint minLen,   // minimum traversal depth
	uint maxLen    // maximum traversal depth
) {
	AllNeighborsCtx *ctx = rm_malloc(sizeof(AllNeighborsCtx));

	ctx->M              =  M;
	ctx->src            =  src;
	ctx->minLen         =  minLen;
	ctx->maxLen         =  maxLen;
	ctx->levels         =  array_new(GxB_MatrixTupleIter*, 1);
	ctx->visited        =  array_new(EntityID, 1);
	ctx->current_level  =  0;

	ctx->visited = array_append(ctx->visited, src);
	_AllNeighbotsCtx_CollectNeighbors(ctx, src);

	return ctx;
}

EntityID AllNeighborsCtx_NextNeighbor
(
	AllNeighborsCtx *ctx
) {
	if(!ctx) return INVALID_ENTITY_ID;

	while(ctx->current_level >= 0) {
		GxB_MatrixTupleIter *it = ctx->levels[ctx->current_level];
		
		bool depleted;
		GrB_Index dest_id;
		GxB_MatrixTupleIter_next(it, NULL, &dest_id, &depleted);

		if(depleted) {
			// rollback
			ctx->current_level--;
			array_pop(ctx->visited);
			continue;
		}

		// update visited path, replace frontier with current node
		ctx->visited = array_append(ctx->visited, dest_id);

		// TODO: not sure we need to increase level here
		ctx->current_level++;

		if(ctx->current_level < ctx->minLen) {
			// ctx->current_level++;
			// continue traversing
			_AllNeighbotsCtx_CollectNeighbors(ctx, dest_id);
			continue;
		}

		// current_level >= ctx->minLen
		// see if we should expand further?
		if(ctx->current_level < ctx->maxLen &&
		   !_AllNeighbotsCtx_Visited(ctx, dest_id)) {
			// ctx->current_level++;
			// we can expand further
			_AllNeighbotsCtx_CollectNeighbors(ctx, dest_id);
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

	// free each level
	uint32_t levelsCount = array_len(ctx->levels);
	for(int i = 0; i < levelsCount; i++) {
		GxB_MatrixTupleIter_free(ctx->levels[i]);
	}
	array_free(ctx->levels);
	array_free(ctx->visited);

	rm_free(ctx);
	ctx = NULL;
}

