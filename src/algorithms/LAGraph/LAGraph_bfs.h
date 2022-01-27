// LAGraph, (c) 2021 by The LAGraph Contributors, All Rights Reserved.
// SPDX-License-Identifier: BSD-2-Clause
//
// See additional acknowledgments in the LICENSE file,
// or contact permission@sei.cmu.edu for the full terms.

#pragma once

#include "GraphBLAS.h"

int LG_BreadthFirstSearch_SSGrB  // push-pull BFS, or push-only if AT = NULL
(
	GrB_Vector    *level,     // v(i) is the BFS level of node i in the graph
	GrB_Vector    *parent,    // pi(i) = p+1 if p is the parent of node i.
	GrB_Matrix    A,          // input graph, treated as if boolean in semiring
	GrB_Index     src,        // starting node of the BFS
	GrB_Index     *dest,      // [optional] stop traversing upon reaching dest
	GrB_Index     max_level   // optional limit of # levels to search
);
