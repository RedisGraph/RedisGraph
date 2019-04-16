/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "all_paths.h"
#include "../util/arr.h"

// Make sure context levels array have atleast 'level' entries,
// Append given 'node' to given 'level' array.
static void _AllPathsCtx_AddNodeToLevel(AllPathsCtx *ctx, uint level, Node *node) {
	while(array_len(ctx->levels) <= level) {
        ctx->levels = array_append(ctx->levels, array_new(Node, 1));
    }
    ctx->levels[level] = array_append(ctx->levels[level], *node);
}

// Check to see if context levels array has entries at position 'level'.
static bool _AllPathsCtx_LevelNotEmpty(const AllPathsCtx *ctx, uint level) {
    return (level < array_len(ctx->levels) && array_len(ctx->levels[level]) > 0);
}

AllPathsCtx* AllPathsCtx_New(Node *src, Graph *g, int *relationIDs, int relationCount, GRAPH_EDGE_DIR dir, unsigned int minLen, unsigned int maxLen) {
    assert(src);
    
    AllPathsCtx *ctx = rm_malloc(sizeof(AllPathsCtx));
    ctx->g = g;
    ctx->dir = dir;
    // Cypher variable path "[:*min..max]"" specifies edge count
    // While the path constructed here contains only nodes.
    // As such a path which require min..max edges
    // should contain min+1..max+1 nodes.
    ctx->minLen = minLen+1;
    ctx->maxLen = maxLen+1;
    ctx->relationIDs = relationIDs;
    ctx->relationCount = relationCount;
    ctx->levels = array_new(Node*, 1);
	ctx->path = array_new(Node, 1);
	_AllPathsCtx_AddNodeToLevel(ctx, 0, src);
	return ctx;
}

Path AllPathsCtx_NextPath(AllPathsCtx *ctx) {
    if(!ctx) return NULL;
    // As long as path is not empty OR there are neighbors to traverse.
	while(!Path_empty(ctx->path) || _AllPathsCtx_LevelNotEmpty(ctx, 0)) {
		uint32_t depth = Path_len(ctx->path);

		// Can we advance?
		if(_AllPathsCtx_LevelNotEmpty(ctx, depth)) {
            // Get a new frontier.
			Node frontier = array_pop(ctx->levels[depth]);

            // Add frontier to path.
            ctx->path = Path_append(ctx->path, frontier);

            // Update path depth.
            depth++;

            // Introduce neighbors only if path depth < maximum path length.
            if(depth < ctx->maxLen) {
                // Get frontier neighbors.
                Edge *neighbors = array_new(Edge, 32);
                for(int i = 0; i < ctx->relationCount; i++) {
                    Graph_GetNodeEdges(ctx->g, &frontier, ctx->dir, ctx->relationIDs[i], &neighbors);
                }

                // Add unvisited neighbors to next level.
                uint32_t neighborsCount = array_len(neighbors);
                for(uint32_t i = 0; i < neighborsCount; i++) {
                    // TODO might need to check direction!
                    Node neighbor;
                    if(ctx->dir == GRAPH_EDGE_DIR_OUTGOING) {
                        Graph_GetNode(ctx->g, Edge_GetDestNodeID(neighbors+i), &neighbor);
                    } else {
                        Graph_GetNode(ctx->g, Edge_GetSrcNodeID(neighbors+i), &neighbor);
                    }

                    if(!Path_containsNode(ctx->path, &neighbor))
                        _AllPathsCtx_AddNodeToLevel(ctx, depth, &neighbor);
                }
                array_free(neighbors);
            }

            // See if we can return path.
            if(depth >= ctx->minLen && depth <= ctx->maxLen) return Path_clone(ctx->path);
		} else {
            // No way to advance, backtrack.
            Path_pop(ctx->path);
        }
	}
    // Couldn't find a path.
	return NULL;
}

void AllPathsCtx_Free(AllPathsCtx *ctx) {
    if(!ctx) return;
    uint32_t levelsCount = array_len(ctx->levels);
    for(int i = 0; i < levelsCount; i++) array_free(ctx->levels[i]);
    Path_free(ctx->path);
    rm_free(ctx);
    ctx = NULL;
}
