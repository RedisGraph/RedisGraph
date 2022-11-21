/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
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
	bool free_matrices;          /* If true, R will ultimately be freed */
} ShortestPathCtx;

void Register_PathFuncs();

