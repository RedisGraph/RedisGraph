/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "../Include/block_matrix.h"


#define LAGRAPH_FREE_ALL     \
{                            \
}

GrB_Info BlockMatrix_BlocksPerColumn
(
	GrB_Index *nblocks_per_col,   // number of blocks in each column
	const BlockMatrix B           // matrix to query
) {
	GrB_Info info;
	if(B == NULL || nblocks_per_col == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_BlocksPerColumn: invalid inputs", GrB_NULL_POINTER);
	}

	LAGRAPH_OK(GrB_Matrix_ncols(nblocks_per_col, B->s));
	return GrB_SUCCESS;
}
