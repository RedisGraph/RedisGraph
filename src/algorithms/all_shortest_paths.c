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
int AllShortestPaths_FindMinimumLength(
	AllPathsCtx *ctx,   // context of the all shortest path
	Node *src,          // the source node
	Node *dest          // the destination node
) {
	ASSERT(ctx != NULL);
	ASSERT(src != NULL);
	ASSERT(dest != NULL);
	ASSERT(ENTITY_GET_ID(ctx->levels[0][0].node) != ENTITY_GET_ID(src));


	int    depth  = 0;
	NodeID destID = ENTITY_GET_ID(dest);

	GrB_Vector visited;
	// benchmark GrB_Vector vs rax vs RoaringBitmap
	GrB_Vector_new(&visited, GrB_BOOL, Graph_UncompactedNodeCount(ctx->g));
	// TODO check format of the vector

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
		// add the rest of the nodes in this level to the visited nodes
		// this is needed for AllShortestPaths_NextPath
		GrB_Vector_setElement_BOOL(visited, true, destID);
		uint count = array_len(ctx->levels[depth]);
		Node node;
		NodeID id;
		for(uint i = 0; i < count; i++) {
			node = ctx->levels[depth][i].node;
			id = ENTITY_GET_ID(&node);
			GrB_Vector_setElement_BOOL(visited, true, id);
		}

		// clean data
		array_clear(ctx->levels[depth]);
		depth++; // switch from edge count to node count
		if(depth < array_len(ctx->levels)) array_clear(ctx->levels[depth]);
	}

	ctx->visited = visited;

	return depth;
}

Path *AllShortestPaths_NextPath(AllPathsCtx *ctx) {
	ASSERT(ctx != NULL);
	// find pathes from src to dest by starting traverse from dest node to
	// one level at time only in nodes that was visited in the AllShortestPaths_FindMinimumLength

	uint32_t depth = Path_NodeCount(ctx->path);
	if(depth > 0) {
		// backtrack to move to next path.
		Path_Reverse(ctx->path);
		Path_PopNode(ctx->path);
		if(Path_EdgeCount(ctx->path)) Path_PopEdge(ctx->path);
		depth--;
	}
	// as long as we didn't found a full path from src to dest
	while (depth < ctx->maxLen) {
		if (array_len(ctx->levels[depth]) > 0) {
			// Get a new node from the frontier.
			LevelConnection frontierConnection = array_pop(ctx->levels[depth]);
			Node frontierNode = frontierConnection.node;
			NodeID frontierID = ENTITY_GET_ID(&frontierNode);
			bool is_visited;
			GrB_Info info = GrB_Vector_extractElement_BOOL(&is_visited, ctx->visited, frontierID);
			// continue only on visited nodes
			if(info == GrB_NO_VALUE) continue;
			// if we reached to the end of the path and this node is not the dst node continue
			if(depth == ctx->maxLen - 1 && frontierID != ENTITY_GET_ID(ctx->dst)) continue;

			Path_AppendNode(ctx->path, frontierNode);
			if(depth > 0) Path_AppendEdge(ctx->path, frontierConnection.edge);

			depth++;
			GRAPH_EDGE_DIR dir = ctx->dir;
			if(dir == GRAPH_EDGE_DIR_BOTH) {
				/* If we're performing a bidirectional traversal, first add all incoming
				* edges, then switch to outgoing edges for the default call. */
				addNeighbors(ctx, &frontierConnection, depth, GRAPH_EDGE_DIR_INCOMING);
				dir = GRAPH_EDGE_DIR_OUTGOING;
			} else if(dir == GRAPH_EDGE_DIR_INCOMING) {
				dir = GRAPH_EDGE_DIR_OUTGOING;
			} else {
				dir = GRAPH_EDGE_DIR_INCOMING;
			}
			addNeighbors(ctx, &frontierConnection, depth, dir);
		} else if(depth == 0) {
			return NULL;
		} else {
			// No way to advance, backtrack.
			Path_PopNode(ctx->path);
			if(Path_EdgeCount(ctx->path)) Path_PopEdge(ctx->path);
			depth--;
		}
	}

	Path_Reverse(ctx->path);
	
	return ctx->path;
}