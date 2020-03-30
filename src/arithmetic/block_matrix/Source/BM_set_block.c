/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "./BM_clear_block.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL     \
{                            \
}

GrB_Info BlockMatrix_SetBlock   // Set a single block in a matrix
(
	BlockMatrix B,              // matrix to update
	GrB_Matrix block,           // block to set
	GrB_Index row,              // block row index
	GrB_Index col               // block column index
) {
	GrB_Info info;
	if(B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_SetBlock: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index nrows;
	GrB_Index ncols;
	LAGRAPH_OK(GrB_Matrix_nrows(&nrows, B->s));
	LAGRAPH_OK(GrB_Matrix_ncols(&ncols, B->s));

	if(row >= nrows) {
		LAGRAPH_ERROR("BlockMatrix_SetBlock: row index, out of range", GrB_INDEX_OUT_OF_BOUNDS) ;
	}
	if(col >= ncols) {
		LAGRAPH_ERROR("BlockMatrix_SetBlock: column index, out of range", GrB_INDEX_OUT_OF_BOUNDS) ;
	}

	if(block == NULL) {
		// Remove block.
		LAGRAPH_OK(BlockMatrix_ClearBlock(B, row, col));
		LAGRAPH_FREE_ALL;
		return GrB_SUCCESS;
	}

	// Compute block position within blocks array.
	int idx = row * ncols + col;
	if(B->blocks[idx] != NULL) {
		LAGRAPH_ERROR("BlockMatrix_SetBlock: block already exists, cannot overwrite block",
					  GrB_INVALID_INDEX) ;
	}

	// Mark block in structure matrix.
	LAGRAPH_OK(GrB_Matrix_setElement_BOOL(B->s, true, row, col));

	B->blocks[idx] = block;

	LAGRAPH_FREE_ALL;
	return GrB_SUCCESS;
}
