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

GrB_Info BlockMatrix_GetBlock
(
	GrB_Matrix *block,          // extracted block
	const BlockMatrix B,        // matrix to extract block from
	GrB_Index row,              // block row index
	GrB_Index col               // block column index
) {
	GrB_Info info;
	if(block == NULL || B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_GetBlock: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index nrows;
	GrB_Index ncols;
	LAGRAPH_OK(GrB_Matrix_nrows(&nrows, B->s));
	LAGRAPH_OK(GrB_Matrix_ncols(&ncols, B->s));

	if(row >= nrows) {
		LAGRAPH_ERROR("BlockMatrix_GetBlock: row index, out of range", GrB_INDEX_OUT_OF_BOUNDS) ;
	}
	if(col >= ncols) {
		LAGRAPH_ERROR("BlockMatrix_GetBlock: column index, out of range", GrB_INDEX_OUT_OF_BOUNDS) ;
	}

	// Compute block position within blocks array.
	int idx = row * ncols + col;
	*block = B->blocks[idx];

	if(*block == NULL) {
		return GrB_NO_VALUE;
	} else {
		return GrB_SUCCESS;
	}
}
