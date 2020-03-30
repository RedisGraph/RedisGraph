/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "BM_index_map.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL     \
{                            \
  GrB_free(&block);          \
}

GrB_Info BlockMatrix_setElement_BOOL    // C (i,j) = x
(
	BlockMatrix C,                      // matrix to modify
	bool x,                             // scalar to assign to C(i,j)
	GrB_Index i,                        // row index
	GrB_Index j                         // column index
) {
	GrB_Matrix block = GrB_NULL;
	GrB_Info info;

	if(C == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_setElement: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index nrows;
	GrB_Index ncols;

	LAGRAPH_OK(BlockMatrix_nrows(&nrows, C));
	LAGRAPH_OK(BlockMatrix_ncols(&ncols, C));

	if(i >= nrows) {
		LAGRAPH_ERROR("BlockMatrix_setElement: row index, out of range", GrB_INVALID_INDEX) ;
	}
	if(j >= ncols) {
		LAGRAPH_ERROR("BlockMatrix_setElement: column index, out of range", GrB_INVALID_INDEX) ;
	}

	GrB_Index block_row_idx;
	GrB_Index block_col_idx;
	GrB_Index block_row_offset;
	GrB_Index block_col_offset;

	LAGRAPH_OK(BlockMatrix_mapEntryToBlock(C, i, j, &block_row_idx, &block_col_idx, &block_row_offset,
										   &block_col_offset));
	LAGRAPH_OK(BlockMatrix_GetBlock(&block, C, block_row_idx, block_col_idx));
	if(block == NULL) {
		// Block is missing, create it.
		LAGRAPH_OK(GrB_Matrix_new(&block, C->t, C->block_nrows, C->block_ncols));
		LAGRAPH_OK(BlockMatrix_SetBlock(C, block, block_row_idx, block_col_idx));
	}

	LAGRAPH_OK(GrB_Matrix_setElement_BOOL(block, x, block_row_offset, block_col_offset));
	return GrB_SUCCESS;
}
