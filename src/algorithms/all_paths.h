/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

/*
 * Finds all paths starting at given source node
 * We're computing one path at a time, this is done
 * to take advantage of scenarios where a query specifies LIMIT.
 * To implement this kind of iterative path fiding using DFS
 * we're keeping track after:
 * 1. the last path computed, which we'll try to expand
 * 2. neighboring nodes discovered, each placed within a "level"
 * array containing all nodes discovered at a specific level.
 * */

#ifndef _ALL_PATHS_H_
#define _ALL_PATHS_H_

#include "../datatypes/path/path.h"
#include "../graph/graph.h"
#include "../graph/entities/node.h"

typedef struct {
	Node node;
	Edge edge;
} LevelConnection;

typedef struct {
	LevelConnection **levels;   // Nodes reached at depth i, and edges leading to them.
	Path *path;                 // Current path.
	Graph *g;                   // Graph to traverse.
	Edge *neighbors;            // Reusable buffer of edges along the current path.
	int *relationIDs;           // edge type(s) to traverse.
	int relationCount;          // length of relationIDs.
	GRAPH_EDGE_DIR dir;         // traverse direction.
	unsigned int minLen;        // Path minimum length.
	unsigned int maxLen;        // Path max length.
	Node *dst;                  // Destination node, defaults to NULL in case of general all paths execution.
} AllPathsCtx;

// Create a new All paths context object.
AllPathsCtx *AllPathsCtx_New(
	Node *src,           // Source node to traverse.
	Node *dst,           // Destination node of the paths
	Graph *g,            // Graph to traverse.
	int *relationIDs,    // Edge type(s) on which we'll traverse.
	int relationCount,   // Length of relationIDs.
	GRAPH_EDGE_DIR dir,  // Traversal direction.
	unsigned int minLen, // Path length must contain be at least minLen + 1 nodes.
	unsigned int maxLen  // Path length must not exceed maxLen + 1 nodes.
);

// Tries to produce a new path from given context
// If no additional path can be computed return NULL.
Path *AllPathsCtx_NextPath(AllPathsCtx *ctx);

// Free context object.
void AllPathsCtx_Free(AllPathsCtx *ctx);

#endif
