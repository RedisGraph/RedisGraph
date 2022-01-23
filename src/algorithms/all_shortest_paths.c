/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "all_shortest_paths.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

// run BFS from src untill dest is visited
// then add the rest of the nodes in the current depth to the visited nodes
// so it can be used in AllShortestPaths_NextPath finding all pathes
// and return the current depth
int AllShortestPaths_FindMinimumLength
(
	AllPathsCtx *ctx,   // context of the all shortest path
	Node *src,          // the source node
	Node *dest          // the destination node
) {
	ASSERT(ctx  != NULL);
	ASSERT(src  != NULL);
	ASSERT(dest != NULL);
	ASSERT(ENTITY_GET_ID(&ctx->levels[0][0].node) != ENTITY_GET_ID(src));

	int    depth  = 0;
	NodeID destID = ENTITY_GET_ID(dest);

	GrB_Vector visited;
	// TODO: benchmark GrB_Vector vs rax vs RoaringBitmap
	GrB_Vector_new(&visited, GrB_BOOL, Graph_UncompactedNodeCount(ctx->g));

	while (true) {
		// see if we have nodes in the current level
		uint neighborCount = array_len(ctx->levels[depth]);
		if(neighborCount == 0) {
			// move to next level
			depth++;
			if(depth > ctx->maxLen || depth >= array_len(ctx->levels)) {
				// we reached to the max level and didn't found the dest node
				depth = 0;
				break;
			}
		}

		// get the node from the frontier
		LevelConnection frontierConnection = array_pop(ctx->levels[depth]);
		Node frontierNode = frontierConnection.node;
		NodeID frontierID = ENTITY_GET_ID(&frontierNode);
		// check if we reached the destination
		if(destID == frontierID && depth > 0) {
			break;
		}

		// the node has already been visited if it is already in the vector
		bool is_visited;
		GrB_Info info = GrB_Vector_extractElement_BOOL(&is_visited, visited, frontierID);

		// introduce neighbors only if path depth < maximum path length
		// and frontier wasn't already expanded
		if(info == GrB_NO_VALUE && depth < ctx->maxLen) {
			GrB_Vector_setElement_BOOL(visited, true, frontierID);
			// add all neighbors of the current node to the next depth level
			GRAPH_EDGE_DIR dir = ctx->dir;
			if(dir == GRAPH_EDGE_DIR_BOTH) {
				// if we're performing a bidirectional traversal, first add all incoming
				// edges, then switch to outgoing edges for the default call
				addNeighbors(ctx, &frontierConnection, depth + 1, GRAPH_EDGE_DIR_INCOMING);
				dir = GRAPH_EDGE_DIR_OUTGOING;
			}
			addNeighbors(ctx, &frontierConnection, depth + 1, dir);
		}
	}

	if(depth > 0) {
		// clean data
		array_clear(ctx->levels[depth]);
		depth++; // switch from edge count to node count
		if(depth < array_len(ctx->levels)) array_clear(ctx->levels[depth]);
	}

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

	// TODO: add an explanation on how paths are discovered

	uint32_t depth = Path_NodeCount(ctx->path);
	if(depth > 0) {
		// TODO: further explain what is happening here
		// backtrack to move to next path
		Path_Reverse(ctx->path);
		Path_PopNode(ctx->path);
		if(Path_EdgeCount(ctx->path)) Path_PopEdge(ctx->path);
		depth--;
	} else {
		Path_AppendNode(ctx->path, array_pop(ctx->levels[0]));
		depth++;
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

			// consider only on visited nodes
			if(info == GrB_NO_VALUE) continue;

			// if we reached to the end of the path and this node is not the
			// dst node continue
			if(depth == ctx->maxLen - 1 && frontierID != ENTITY_GET_ID(ctx->dst)) continue;

			Path_AppendNode(ctx->path, frontierNode);
			Path_AppendEdge(ctx->path, frontierConnection.edge);

			depth++;
			GRAPH_EDGE_DIR dir = ctx->dir;
			if(dir == GRAPH_EDGE_DIR_BOTH) {
				// if we're performing a bidirectional traversal
				// first add all incoming edges
				// then switch to outgoing edges for the default call
				addNeighbors(ctx, &frontierConnection, depth, GRAPH_EDGE_DIR_INCOMING);
				dir = GRAPH_EDGE_DIR_OUTGOING;
			} else if(dir == GRAPH_EDGE_DIR_INCOMING) {
				// TODO: perform this update only once
				dir = GRAPH_EDGE_DIR_OUTGOING;
			} else {
				dir = GRAPH_EDGE_DIR_INCOMING;
			}
			addNeighbors(ctx, &frontierConnection, depth, dir);
		} else if(depth == 0) {
			// first level fully consumed
			// there are no more paths leading from src to dest, we're done
			return NULL;
		} else {
			// no way to advance, backtrack
			Path_PopNode(ctx->path);
			if(Path_EdgeCount(ctx->path)) Path_PopEdge(ctx->path);
			depth--;
		}
	}

	// we've traversed going from dest to src, caller expects path going from
	// src to dest, reverse path
	Path_Reverse(ctx->path);
	
	return ctx->path;
}

