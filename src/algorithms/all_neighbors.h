/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "GraphBLAS.h"
#include "../graph/entities/node.h"

typedef struct {
	EntityID src;                  // traverse begin here
	GrB_Matrix M;                  // topology
	uint minLen;                   // minimum required depth
	uint maxLen;                   // maximum allowed depth
	uint current_level;            // cuurent depth
	EntityID *visited;             // visited nodes
	GxB_MatrixTupleIter **levels;  // array of neighbors iterator
} AllNeighborsCtx;

AllNeighborsCtx *AllNeighborsCtx_New
(
	EntityID src,  // source node from which to traverse
	GrB_Matrix M,  // matrix describing connections
	uint minLen,   // minimum traversal depth
	uint maxLen    // maximum traversal depth
);

EntityID AllNeighborsCtx_NextNeighbor
(
	AllNeighborsCtx *ctx
);

void AllNeighborsCtx_Free
(
	AllNeighborsCtx *ctx
);

