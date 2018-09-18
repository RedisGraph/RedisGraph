/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef _ALL_PATHS_H_
#define _ALL_PATHS_H_

#include "./path.h"
#include "../graph/graph.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

/* Find all paths of length between minLength and maxLength starting at src. */

/* A path is an array of edges 
 * where edge at position i leads to edge at position i+1. */
void AllPaths
(
    const Graph *g,         // Graph traversed.
    int relationID,         // Edge type to traverse.
    NodeID src,             // Node from which to traverse.
    unsigned int minLen,    // Path minimum length.
    unsigned int maxLen,    // Path max length.
    Path **paths            // Paths found.
);

#endif
