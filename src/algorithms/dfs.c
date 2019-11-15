/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./bfs.h"
#include "../util/arr.h"
#include "rax.h"
#include "../graph/entities/qg_edge.h"

bool _DFS(QGNode *n, int level, bool close_cycle, int current_level, rax *visited, QGEdge ***path) {
	// As long as we've yet to reach required level and there are nodes to process.
	if(current_level >= level) return true;

	// Mark n as visited, return if node already marked.
	if(!raxInsert(visited, (unsigned char *)n->alias, strlen(n->alias), NULL, NULL)) {
		// We've already processed n.
		return false;
	}

	// Expand node N by visiting all of its neighbors
	bool not_seen;
	for(uint i = 0; i < array_len(n->outgoing_edges); i++) {
		QGEdge *e = n->outgoing_edges[i];
		not_seen = raxFind(visited, (unsigned char *)e->dest->alias, strlen(e->dest->alias)) == raxNotFound;
		if(not_seen || close_cycle) {
			*path = array_append(*path, e);
			if(_DFS(e->dest, level, close_cycle, current_level + 1, visited, path)) return true;
			array_pop(*path);
		}
	}

	for(uint i = 0; i < array_len(n->incoming_edges); i++) {
		QGEdge *e = n->incoming_edges[i];
		not_seen = raxFind(visited, (unsigned char *)e->src->alias, strlen(e->src->alias)) == raxNotFound;
		if(not_seen || close_cycle) {
			*path = array_append(*path, e);
			if(_DFS(e->src, level, close_cycle, current_level + 1, visited, path)) return true;
			array_pop(*path);
		}
	}

	raxRemove(visited, (unsigned char *)n->alias, strlen(n->alias), NULL);
	return false;
}

// Returns a single path from S to a reachable node at distance level.
QGEdge **DFS(QGNode *s, int level, bool close_cycle) {
	int current_level = 0;                  // Tracks BFS level.
	rax *visited = raxNew();                // Dictionary of visited nodes.
	QGEdge **path = array_new(QGEdge *, 0); // Path found.

	_DFS(s, level, close_cycle, current_level, visited, &path);
	raxFree(visited);
	return path;
}
