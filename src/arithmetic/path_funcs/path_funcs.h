/*
 * Copyright 2018-2020 Redis Labs Ltd. and Contributors
 *
 * This file is available under the Redis Labs Source Available License Agreement
 */

#pragma once
#include "../../value.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

// Context struct containing traversal data for shortestPath function calls
typedef struct {
	uint minHops;                /* Minimum number of edges traversed by this path */
	uint maxHops;                /* Maximum number of edges traversed by this path */
	const char **reltype_names;  /* Relationship type names */
	int *reltypes;               /* Relationship type IDs */
	uint reltype_count;          /* Number of traversed relationship types */
	GrB_Matrix R;                /* Traversed relationship matrix */
	GrB_Matrix TR;               /* Transpose of traversed relationship matrix */
	bool free_matrices;          /* If true, R and TR will ultimately be freed */
} ShortestPathCtx;

void Register_PathFuncs();

