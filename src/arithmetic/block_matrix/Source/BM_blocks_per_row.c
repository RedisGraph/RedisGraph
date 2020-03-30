/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL     \
{                            \
}

GrB_Info BlockMatrix_BlocksPerRow
(
	GrB_Index *nblocks_per_row,   // number of blocks in each row
	const BlockMatrix B           // matrix to query
) {
	GrB_Info info;
	if(B == NULL || nblocks_per_row == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_BlocksPerRow: invalid inputs", GrB_NULL_POINTER);
	}

	LAGRAPH_OK(GrB_Matrix_nrows(nblocks_per_row, B->s));
	return GrB_SUCCESS;
}
