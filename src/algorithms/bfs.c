/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./bfs.h"
#include "../util/arr.h"
#include "../../deps/rax/rax.h"
#include "../graph/entities/edge.h"

// Returns a set of nodes reached at given level from S.
Node** BFS(Node *s, int *level) {
    void *seen;                             // Has node been visited?
    int current_level = 0;                  // Tracks BFS level.
    rax *visited = raxNew();                // Dictionary of visited nodes.
    Node **next = array_new(Node*, 0);      // Nodes to explore next.
    Node **current = array_new(Node*, 1);   // Nodes currently explored.
    
    current = array_append(current, s);

    // As long as we've yet to reach required level and there are nodes to process.
    while(current_level < *level && array_len(current)) {
        // As long as there are nodes in the frontier.
        for(int i = 0; i < array_len(current); i++) {
            Node *n = current[i];

            // Mark n as visited.
            if(!raxInsert(visited, (unsigned char*)n->alias, strlen(n->alias), NULL, NULL)) {
                // We've already processed n.
                continue;
            }

            // Expand node N by visiting all of its neighbors
            for(int j = 0; j < array_len(n->outgoing_edges); j++) {
                Edge *e = n->outgoing_edges[j];
                char *dest = e->dest->alias;
                seen = raxFind(visited, (unsigned char*)dest, strlen(dest));
                if(seen == raxNotFound) {
                    next = array_append(next, e->dest);
                }
            }
            for(int j = 0; j < array_len(n->incoming_edges); j++) {
                Edge *e = n->incoming_edges[j];
                char *src = e->src->alias;
                seen = raxFind(visited, (unsigned char*)src, strlen(src));
                if(seen == raxNotFound) {
                    next = array_append(next, e->src);
                }
            }
        }

        /* No way to progress and we're interested in the lowest level leafs
         * do not clear current queue. */
        if(array_len(next) == 0 && *level == BFS_LOWEST_LEVEL) {
            *level = current_level;
            break;
        }

        // Queue consumed, swap queues.
        array_clear(current);
        Node **tmp = current;
        current = next;
        next = tmp;
        current_level++;
    }

    raxFree(visited);
    array_free(next);
    return current;
}
