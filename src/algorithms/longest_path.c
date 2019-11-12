/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./longest_path.h"
#include "./bfs.h"
#include "./dfs.h"
#include "../util/arr.h"
#include"./detect_cycle.h"

// Scans the graph in a DFS fashion, keeps track after the longest path length.
static void __DFSMaxDepth(QGNode *n, int level, int *max_depth, rax *visited) {
	if(level > *max_depth) *max_depth = level;

	// Mark n as visited, return if node already marked.
	if(!raxInsert(visited, (unsigned char *)n->alias, strlen(n->alias), NULL, NULL)) {
		// We've already processed n.
		return;
	}

	// Expand node N by visiting all of its neighbors
	for(uint i = 0; i < array_len(n->outgoing_edges); i++) {
		QGEdge *e = n->outgoing_edges[i];
		__DFSMaxDepth(e->dest, level + 1, max_depth, visited);
	}

	for(uint i = 0; i < array_len(n->incoming_edges); i++) {
		QGEdge *e = n->incoming_edges[i];
		__DFSMaxDepth(e->src, level + 1, max_depth, visited);
	}

	raxRemove(visited, (unsigned char *)n->alias, strlen(n->alias), NULL);
}

// Finds out the longest path distance from given node.
static int _DFSMaxDepth(QGNode *n) {
	int level = 0;              // Starting at level 0.
	int max_depth = 0;          // Longest path length.
	rax *visited = raxNew();    // Dictionary of visited nodes.

	__DFSMaxDepth(n, level, &max_depth, visited);

	raxFree(visited);
	return max_depth;
}

// Finds the longest path in an cyclic graph.
QGNode *LongestPathGraph(const QueryGraph *g, int *level) {
	QGNode *n = NULL;  // Node from which the longest path expand.
	int max_path_len = 0;
	uint node_count = QueryGraph_NodeCount(g);

	// Run DFS from each node.
	for(uint i = 0; i < node_count; i++) {
		QGNode *node = g->nodes[i];
		int longest_path_len = _DFSMaxDepth(node);

		// Found a longer path, update.
		if(max_path_len < longest_path_len) {
			max_path_len = longest_path_len;
			n = node;
		}
	}
	assert(n);

	*level = max_path_len;
	return n;
}

// Finds the longest path in an acyclic graph.
QGNode *LongestPathTree(const QueryGraph *g, int *level) {
	int l = BFS_LOWEST_LEVEL;
	QGNode **leafs = BFS(g->nodes[0], &l);
	QGNode *leaf = leafs[0];
	array_free(leafs);

	l = BFS_LOWEST_LEVEL;
	leafs = BFS(leaf, &l);

	assert(array_len(leafs) > 0 && l >= 0);
	QGNode *n = leafs[0];
	array_free(leafs);

	*level = l;
	return n;
}
