/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <assert.h>
#include <sys/param.h>
#include "./all_paths.h"
#include "../util/arr.h"

void _AllPaths
(
    const Graph *g,         // Graph traversed.
    Node *frontier,         // Last node reached on path.
    GRAPH_EDGE_DIR dir,     // Traversal direction.
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
            (*pathsCap) = (*pathsCap) * 2;
            (*paths) = realloc(*paths, sizeof(Path) * (*pathsCap));
        }

        (*paths)[*pathsCount] = clone;
        (*pathsCount) = (*pathsCount) + 1;
    }

    if(hop >= maxHops) {
        // We've over extended the path.
        return;
    }

    NodeID frontierID = ENTITY_GET_ID(frontier);
    Edge *neighbors = array_new(Edge, 32);

    // TODO: Extremely wastefull when dir is GRAPH_EDGE_DIR_INCOMING
    // See if we can instead pass a transposed matrix.
    Graph_GetNodeEdges(g, frontier, dir, relationID, &neighbors);
    size_t neighborsCount = array_len(neighbors);

    for(size_t i = 0; i < neighborsCount; i++) {
        Edge *e = neighbors + i;
        NodeID neighborID;
        Node neighbor;

        if(dir == GRAPH_EDGE_DIR_OUTGOING) neighborID = Edge_GetDestNodeID(e);
        else neighborID = Edge_GetSrcNodeID(e);

        // See if edge frontier->neighbor been used.
        bool used = false;
        GrB_Matrix_extractElement_BOOL(&used, visited, neighborID, frontierID);
        if(used) continue;

        Graph_GetNode(g, neighborID, &neighbor);

        // Mark edge as visited.
        GrB_Matrix_setElement_BOOL(visited, true, neighborID, frontierID);

        *path = Path_append(*path, *e);

        _AllPaths(g, &neighbor, dir, relationID, relation, minHops, maxHops, hop+1, visited, path, pathsCount, pathsCap, paths);
        
        Path_pop(*path);

        // Mark edge as unvisited.
        GrB_Matrix_setElement_BOOL(visited, false, neighborID, frontierID);
    }

    array_free(neighbors);
}

size_t AllPaths
(
    const Graph *g,
    int relationID,
    NodeID src,
    GRAPH_EDGE_DIR dir,
    unsigned int minLen,
    unsigned int maxLen,
    size_t *pathsCap,
    Path **paths
)
{
    assert(g && minLen >= 0 && minLen <= maxLen && pathsCap && paths);
    GrB_Matrix relation = Graph_GetRelationMatrix(g, relationID);

    /* Avoid revisiting edges along a constructed path by, marking visited edges,
     * for every traversed edge (A)-[]->(B) visited[A,B] is set. */
    GrB_Matrix visited;
    GrB_Matrix_new(&visited, GrB_BOOL, Graph_RequiredMatrixDim(g), Graph_RequiredMatrixDim(g));

    unsigned int pathLen = MIN(16, maxLen - minLen);
    Path p = Path_new(pathLen);

    size_t pathsCount = 0;
    Node srcNode;
    Graph_GetNode(g, src, &srcNode);
    _AllPaths(g, &srcNode, dir, relationID, relation, minLen, maxLen, 0, visited, &p, &pathsCount, pathsCap, paths);

    Path_free(p);
    GrB_Matrix_free(&visited);

    return pathsCount;
}
