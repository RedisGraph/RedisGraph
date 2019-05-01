/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./bfs.h"
#include "../util/arr.h"
#include "../../deps/rax/rax.h"
#include "../graph/entities/edge.h"

bool _DFS(Node *n, int level, int current_level, rax *visited, Edge ***path) {
    // As long as we've yet to reach required level and there are nodes to process.
    if(current_level >= level) return true;

    // Mark n as visited, return if node already marked.
    if(!raxInsert(visited, (unsigned char*)n->alias, strlen(n->alias), NULL, NULL)) {
        // We've already processed n.
        return false;
    }

    // Expand node N by visiting all of its neighbors
    void *seen;
    for(int i = 0; i < array_len(n->outgoing_edges); i++) {
        Edge *e = n->outgoing_edges[i];
        char *dest = e->dest->alias;
        seen = raxFind(visited, (unsigned char*)dest, strlen(dest));
        if(seen == raxNotFound) {
            *path = array_append(*path, e);
            if(_DFS(e->dest, level, ++current_level, visited, path)) return true;
            array_pop(*path);
        }
    }

    for(int i = 0; i < array_len(n->incoming_edges); i++) {
        Edge *e = n->incoming_edges[i];
        char *src = e->src->alias;
        seen = raxFind(visited, (unsigned char*)src, strlen(src));
        if(seen == raxNotFound) {
            *path = array_append(*path, e);
            if(_DFS(e->src, level, ++current_level, visited, path)) return true;
            array_pop(*path);
        }
    }

    raxRemove(visited, (unsigned char*)n->alias, strlen(n->alias), NULL);
    return false;
}

// Returns a single path from S to a reachable node at distance level.
Edge** DFS(Node *s, int level) {    
    int current_level = 0;                  // Tracks BFS level.
    rax *visited = raxNew();                // Dictionary of visited nodes.
    Edge **path = array_new(Edge*, 0);      // Path found.
    
    _DFS(s, level, current_level, visited, &path);
    raxFree(visited);
    return path;
}
