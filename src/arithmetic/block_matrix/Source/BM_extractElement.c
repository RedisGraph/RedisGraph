/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "./BM_index_map.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL     \
{                            \
}

GrB_Info BlockMatrix_extractElement_BOOL    // x = A(i,j)
(
	bool *x,                                // extracted scalar
	const BlockMatrix A,                    // matrix to extract a scalar from
	GrB_Index i,                            // row index
	GrB_Index j                             // column index
) {
	GrB_Info info;
	if(A == NULL) {
		LAGRAPH_ERROR("BlockMatrix_extractElement_BOOL: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index nrows;
	GrB_Index ncols;
	LAGRAPH_OK(BlockMatrix_nrows(&nrows, A));
	LAGRAPH_OK(BlockMatrix_ncols(&ncols, A));

	if(i >= nrows || j >= ncols) {
		return GrB_INVALID_INDEX;
	}

	GrB_Matrix block;
	GrB_Index block_row_idx;
	GrB_Index block_col_idx;
	GrB_Index block_row_offset;
	GrB_Index block_col_offset;

	LAGRAPH_OK(BlockMatrix_mapEntryToBlock(A, i, j, &block_row_idx, &block_col_idx, &block_row_offset,
										   &block_col_offset));
	LAGRAPH_OK(BlockMatrix_GetBlock(&block, A, block_row_idx, block_col_idx));
	if(block == NULL) {
		return GrB_NO_VALUE;
	}

	LAGRAPH_OK(GrB_Matrix_extractElement_BOOL(x, block, block_row_offset, block_col_offset));

	return GrB_SUCCESS;
}
