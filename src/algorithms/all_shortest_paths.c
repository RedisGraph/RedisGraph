/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "all_shortest_paths.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

// run BFS from `src` untill `dest` is rached
// then add the rest of the nodes in the current depth to `visited`
// so it can be used later on in `AllShortestPaths_NextPath`
int AllShortestPaths_FindMinimumLength
(
	AllPathsCtx *ctx,   // context of the all shortest path
	Node *src,          // source node
	Node *dest          // destination node
) {
	ASSERT(ctx  != NULL);
	ASSERT(src  != NULL);
	ASSERT(dest != NULL);
	ASSERT(ENTITY_GET_ID(&ctx->levels[0]->node) == ENTITY_GET_ID(src));

	int    depth  = 0;
	NodeID destID = ENTITY_GET_ID(dest);

	GrB_Vector visited;
	GrB_Vector current_visited;

	GrB_Vector_new(&visited, GrB_BOOL, Graph_UncompactedNodeCount(ctx->g));
	GxB_set(visited, GxB_SPARSITY_CONTROL, GxB_BITMAP);

	GrB_Vector_new(&current_visited, GrB_BOOL, Graph_UncompactedNodeCount(ctx->g));
	GxB_set(current_visited, GxB_SPARSITY_CONTROL, GxB_BITMAP);

	while (true) {
		// see if we have nodes in the current level
		uint neighborCount = array_len(ctx->levels[depth]);
		if(neighborCount == 0) {
			// move to next level
			depth++;
			if(depth == ctx->maxLen || depth >= array_len(ctx->levels)) {
				// we reached max level and didn't found the dest node
				depth = 0; // indicate `dest` wasn't reached
				break;
			}
			// add the current level visited to the global visited vector
			// as we finished with current level and move to next level
			GrB_Vector_eWiseAdd_BinaryOp(visited, NULL, NULL, GxB_ANY_BOOL, visited, current_visited, NULL);
			GrB_Vector_clear(current_visited);
		}

		// get the node from the frontier
		LevelConnection frontierConnection = array_pop(ctx->levels[depth]);
		Node frontierNode = frontierConnection.node;
		NodeID frontierID = ENTITY_GET_ID(&frontierNode);
		// check if we reached dest
		if(destID == frontierID && depth > 0) {
			break;
		}

		// introduce neighbors only if path depth < maximum path length
		if(depth == ctx->maxLen - 1) continue;

		// the node has already been visited if it is already in the vector
		bool x;
		GrB_Info info = GrB_Vector_extractElement_BOOL(&x, visited, frontierID);
		bool is_visited = (info == GrB_SUCCESS);
		if(is_visited) continue;
		info = GrB_Vector_extractElement_BOOL(&x, current_visited, frontierID);
		is_visited = (info == GrB_SUCCESS);
		if(is_visited) continue;

		GrB_Vector_setElement_BOOL(current_visited, true, frontierID);
		// add all neighbors of the current node to the next level
		addNeighbors(ctx, &frontierConnection, depth + 1, ctx->dir);
	}

	if(depth > 0) {
		// `dest` was reached
		array_clear(ctx->levels[depth]); // clean data
		depth++; // switch from edge count to node count
		if(depth < array_len(ctx->levels)) array_clear(ctx->levels[depth]);
	}

	GrB_free(&current_visited);
	ctx->visited = visited;

	return depth;
}

// find paths from src to dest by traversing from dest to src using DFS
// inspecting nodes which where discovered by
// the previous call to `AllShortestPaths_FindMinimumLength`
Path *AllShortestPaths_NextPath
(
	AllPathsCtx *ctx
) {
	ASSERT(ctx != NULL);

	uint32_t depth = Path_NodeCount(ctx->path);
	if(depth > 0) {
		// a full path already returned
		// advance to next path by backtracking
		depth--;
	} else {
		if(array_len(ctx->levels[0]) == 0) return NULL;

		Path_EnsureLen(ctx->path, ctx->minLen);
		LevelConnection frontierConnection = array_pop(ctx->levels[0]);
		Node frontierNode = frontierConnection.node;
		Path_SetNode(ctx->path, ctx->minLen - depth - 1, frontierNode);
		depth++;
		addNeighbors(ctx, &frontierConnection, depth, ctx->dir);
	}

	// as long as we didn't found a full path from src to dest
	while (depth < ctx->maxLen) {
		if (array_len(ctx->levels[depth]) > 0) {
			// get a new node from the frontier
			bool is_visited;
			LevelConnection frontierConnection = array_pop(ctx->levels[depth]);
			Node frontierNode = frontierConnection.node;
			NodeID frontierID = ENTITY_GET_ID(&frontierNode);
			GrB_Info info = GrB_Vector_extractElement_BOOL(&is_visited,
					ctx->visited, frontierID);

			// consider only previously discovered nodes
			if(info == GrB_NO_VALUE) continue;

			// if we reached to the end of the path and this node is not the
			// dst node continue
			if(depth == ctx->maxLen - 1 &&
			   frontierID != ENTITY_GET_ID(ctx->dst)) {
				continue;
			}

			Path_SetNode(ctx->path, ctx->minLen - depth - 1, frontierNode);
			Path_SetEdge(ctx->path, ctx->minLen - depth - 1, frontierConnection.edge);

			if(depth == ctx->maxLen - 1) break;

			depth++;
			addNeighbors(ctx, &frontierConnection, depth, ctx->dir);
		} else {
			depth--;
			if(depth == 0) {
				Path_Clear(ctx->path);
				// first level fully consumed
				// there are no more paths leading from src to dest, we're done
				return NULL;
			}
		}
	}
	
	return ctx->path;
}

