/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "all_paths.h"
#include "RG.h"
#include "../util/arr.h"
#include "../util/rmalloc.h"

// Make sure context levels array have atleast 'level' entries,
// Append given 'node' to given 'level' array.
static void _AllPathsCtx_AddConnectionToLevel(AllPathsCtx *ctx, uint level, Node *node,
											  Edge *edge) {
	while(array_len(ctx->levels) <= level) {
		ctx->levels = array_append(ctx->levels, array_new(LevelConnection, 1));
	}
	LevelConnection connection;
	connection.node = *node;
	if(edge) connection.edge = *edge;
	ctx->levels[level] = array_append(ctx->levels[level], connection);
}

// Check to see if context levels array has entries at position 'level'.
static bool _AllPathsCtx_LevelNotEmpty(const AllPathsCtx *ctx, uint level) {
	return (level < array_len(ctx->levels) && array_len(ctx->levels[level]) > 0);
}

// Traverse from the frontier node in the specified direction and add all encountered nodes and edges.
static void _addNeighbors(AllPathsCtx *ctx, LevelConnection *frontier, uint32_t depth,
						  GRAPH_EDGE_DIR dir) {
	EntityID frontierId = INVALID_ENTITY_ID;
	if(depth > 1) frontierId = ENTITY_GET_ID(&frontier->edge);

	// Get frontier neighbors.
	for(int i = 0; i < ctx->relationCount; i++) {
		Graph_GetNodeEdges(ctx->g, &frontier->node, dir, ctx->relationIDs[i], &ctx->neighbors);
	}

	// Add unvisited neighbors to next level.
	uint32_t neighborsCount = array_len(ctx->neighbors);
	for(uint32_t i = 0; i < neighborsCount; i++) {
		// Don't follow the frontier edge again.
		if(frontierId == ENTITY_GET_ID(ctx->neighbors + i)) continue;
		// Set the neighbor by following the edge in the correct directoin.
		Node neighbor = GE_NEW_NODE();
		switch(dir) {
		case GRAPH_EDGE_DIR_OUTGOING:
			Graph_GetNode(ctx->g, Edge_GetDestNodeID(ctx->neighbors + i), &neighbor);
			break;
		case GRAPH_EDGE_DIR_INCOMING:
			Graph_GetNode(ctx->g, Edge_GetSrcNodeID(ctx->neighbors + i), &neighbor);
			break;
		default:
			ASSERT(false && "encountered unexpected traversal direction in AllPaths");
			break;
		}
		// Add the node and edge to the frontier.
		_AllPathsCtx_AddConnectionToLevel(ctx, depth, &neighbor, (ctx->neighbors + i));
	}
	array_clear(ctx->neighbors);
}

AllPathsCtx *AllPathsCtx_New(Node *src, Node *dst, Graph *g, int *relationIDs, int relationCount,
							 GRAPH_EDGE_DIR dir, unsigned int minLen, unsigned int maxLen) {
	ASSERT(src != NULL);

	AllPathsCtx *ctx = rm_malloc(sizeof(AllPathsCtx));
	ctx->g = g;
	ctx->dir = dir;
	// Cypher variable path "[:*min..max]"" specifies edge count
	// While the path constructed here contains only nodes.
	// As such a path which require min..max edges
	// should contain min+1..max+1 nodes.
	ctx->minLen = minLen + 1;
	ctx->maxLen = maxLen + 1;
	ctx->relationIDs = relationIDs;
	ctx->relationCount = relationCount;
	ctx->levels = array_new(LevelConnection *, 1);
	ctx->path = Path_New(1);
	ctx->neighbors = array_new(Edge, 32);
	_AllPathsCtx_AddConnectionToLevel(ctx, 0, src, NULL);
	ctx->dst = dst;
	return ctx;
}

Path *AllPathsCtx_NextPath(AllPathsCtx *ctx) {
	if(!ctx) return NULL;
	// As long as path is not empty OR there are neighbors to traverse.
	while(Path_NodeCount(ctx->path) || _AllPathsCtx_LevelNotEmpty(ctx, 0)) {
		uint32_t depth = Path_NodeCount(ctx->path);

		// Can we advance?
		if(_AllPathsCtx_LevelNotEmpty(ctx, depth)) {
			// Get a new frontier.
			LevelConnection frontierConnection = array_pop(ctx->levels[depth]);
			Node frontierNode = frontierConnection.node;

			/* See if frontier is already on path,
			 * it is OK for a path to contain an entity twice,
			 * such as in the case of a cycle, but in such case we
			 * won't expand frontier.
			 * i.e. closing a cycle and continuing traversal. */
			bool frontierAlreadyOnPath = Path_ContainsNode(ctx->path, &frontierNode);

			// Add frontier to path.
			Path_AppendNode(ctx->path, frontierNode);

			/* If depth is 0 this is the source node, there is no leading edge to it.
			 * For depth > 0 for each frontier node, there is a leading edge. */
			if(depth > 0) Path_AppendEdge(ctx->path, frontierConnection.edge);

			// Update path depth.
			depth++;

			/* Introduce neighbors only if path depth < maximum path length.
			 * and frontier wasn't already expanded. */
			if(depth < ctx->maxLen && !frontierAlreadyOnPath) {
				GRAPH_EDGE_DIR dir = ctx->dir;
				if(dir == GRAPH_EDGE_DIR_BOTH) {
					/* If we're performing a bidirectional traversal, first add all incoming
					 * edges, then switch to outgoing edges for the default call. */
					_addNeighbors(ctx, &frontierConnection, depth, GRAPH_EDGE_DIR_INCOMING);
					dir = GRAPH_EDGE_DIR_OUTGOING;
				}
				_addNeighbors(ctx, &frontierConnection, depth, dir);
			}

			// See if we can return path.
			/* TODO Note that further calls to this function will continue to operate on
			 * this path, so it is essential that the caller does not modify it (or creates
			 * a copy beforehand). If future features like an algorithm API use this routine,
			 * they should either be responsible for memory safety or a memory-safe boolean/routine
			 * should be offered. */
			if(depth >= ctx->minLen && depth <= ctx->maxLen) {
				if(ctx->dst != NULL) {
					Node dst = Path_Head(ctx->path);
					if(ENTITY_GET_ID(ctx->dst) != ENTITY_GET_ID(&dst)) continue;
				}
				return ctx->path;
			}
		} else {
			// No way to advance, backtrack.
			Path_PopNode(ctx->path);
			if(Path_EdgeCount(ctx->path)) Path_PopEdge(ctx->path);
		}
	}
	// Couldn't find a path.
	return NULL;
}

void AllPathsCtx_Free(AllPathsCtx *ctx) {
	if(!ctx) return;
	uint32_t levelsCount = array_len(ctx->levels);
	for(int i = 0; i < levelsCount; i++) array_free(ctx->levels[i]);
	array_free(ctx->levels);
	Path_Free(ctx->path);
	array_free(ctx->neighbors);
	rm_free(ctx);
	ctx = NULL;
}

