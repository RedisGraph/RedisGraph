/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include <sys/param.h>
#include "./all_paths.h"

void _AllPaths
(
    const Graph *g,         // Graph traversed.
    Node *frontier,         // Last node reached on path.
    int relationID,         // Edge type to traverse.
    GrB_Matrix relation,    // Edge matrix.
    unsigned int minHops,   // Path minimum length.
    float maxHops,          // Path max length.
    float hop,              // Path current length.
    GrB_Matrix visited,     // Edges visited on path.
    Path *path,             // Current path.
    size_t *pathsCount,     // Number of paths constructed.
    size_t *pathsCap,       // Paths array capacity.
    Path **paths            // Paths constructed.
)
{
        if(hop >= minHops && hop <= maxHops) {
            Path clone = Path_clone(*path);

            if(*pathsCount >= *pathsCap) {
                (*pathsCap) = (*pathsCap) * 2 + 1;
                (*paths) = realloc(*paths, sizeof(Path) * (*pathsCap));
            }

            (*paths)[*pathsCount] = clone;
            (*pathsCount) = (*pathsCount) + 1;
        }

        if(hop >= maxHops) {
            // We've over extended the path.
            return;
        }

        NodeID frontierID = frontier->id;
        Vector *outGoingEdges = NewVector(Edge*, 32);
        Graph_GetNodeEdges(g, frontier, outGoingEdges, GRAPH_EDGE_DIR_OUTGOING, relationID);
        size_t outGoingEdgesCount = Vector_Size(outGoingEdges);

        for(size_t i = 0; i < outGoingEdgesCount; i++) {
            Edge *e;
            Vector_Get(outGoingEdges, i, &e);
            NodeID neighborID = Edge_GetDestNodeID(e);
            
            // See if edge frontier->neighbor been used.
            bool used = false;
            GrB_Matrix_extractElement_BOOL(&used, visited, neighborID, frontierID);
            if(used) continue;

            // Mark edge as visited.
            GrB_Matrix_setElement_BOOL(visited, true, neighborID, frontierID);

            *path = Path_append(*path, e);

            _AllPaths(g, Edge_GetDestNode(e), relationID, relation, minHops, maxHops, hop+1, visited, path, pathsCount, pathsCap, paths);

            Path_pop(*path);

            // Mark edge as unvisited.
            GrB_Matrix_setElement_BOOL(visited, false, neighborID, frontierID);
        }

        Vector_Free(outGoingEdges);
}

size_t AllPaths
(
    const Graph *g,
    int relationID,
    NodeID src,
    unsigned int minLen,
    unsigned int maxLen,
    size_t *pathsCap,
    Path **paths
)
{
    assert(g && minLen >= 0 && minLen <= maxLen && pathsCap && paths);

    GrB_Matrix relation = Graph_GetRelation(g, relationID);    
    /* Avoid revisiting edges along a constructed path by, marking visited edges,
     * for every traversed edge (A)-[]->(B) visited[A,B] is set. */
    GrB_Matrix visited;
    size_t nodeCount = Graph_NodeCount(g);
    GrB_Matrix_new(&visited, GrB_BOOL, nodeCount, nodeCount);

    unsigned int pathLen = MIN(16, maxLen - minLen);
    Path p = Path_new(pathLen);

    size_t pathsCount = 0;
    _AllPaths(g, Graph_GetNode(g, src), relationID, relation, minLen, maxLen, 0, visited, &p, &pathsCount, pathsCap, paths);

    Path_free(p);
    GrB_Matrix_free(&visited);

    return pathsCount;
}
