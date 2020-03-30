/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "BM_clear_block.h"

#define LAGRAPH_FREE_ALL     \
{                            \
  GrB_free(&Z);              \
}

GrB_Info BlockMatrix_ClearBlock // Clears a single block in a matrix
(
	BlockMatrix B,                // matrix to update
	GrB_Index row,                // block row index
	GrB_Index col                 // block column index
) {
	GrB_Info info;
	GrB_Matrix Z = GrB_NULL;

	if(B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_ClearBlock: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index nblocks_per_row;
	GrB_Index nblocks_per_col;
	LAGRAPH_OK(BlockMatrix_BlocksPerRow(&nblocks_per_row, B));
	LAGRAPH_OK(BlockMatrix_BlocksPerColumn(&nblocks_per_col, B));

	if(row >= nblocks_per_row) {
		LAGRAPH_ERROR("BlockMatrix_SetBlock: row index, out of range", GrB_INDEX_OUT_OF_BOUNDS) ;
	}
	if(col >= nblocks_per_col) {
		LAGRAPH_ERROR("BlockMatrix_SetBlock: column index, out of range", GrB_INDEX_OUT_OF_BOUNDS) ;
	}

	// Compute block position within blocks array.
	int idx = row * nblocks_per_col + col;
	GrB_Matrix block = B->blocks[idx];
	if(block != NULL) {
		LAGRAPH_OK(GrB_free(&block));
	}

	// Clear entry within structure matrix.
	// 1X1 empty matrix.
	LAGRAPH_OK(GrB_Matrix_new(&Z, GrB_BOOL, 1, 1));
	LAGRAPH_OK(GxB_Matrix_subassign(B->s, GrB_NULL, GrB_NULL, Z, &row, 1, &col, 1, GrB_NULL));

	return GrB_SUCCESS;
}
