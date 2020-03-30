/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL     \
{                            \
  BlockMatrix_free(&dup);    \
}

GrB_Info BlockMatrix_dup    // make an exact copy of a matrix
(
	BlockMatrix *C,         // handle of output matrix to create
	const BlockMatrix A     // input matrix to copy
) {
	GrB_Info info;
	BlockMatrix dup = GrB_NULL;

	if(C == NULL || A == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_dup: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index nvals;
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Index blocks_per_row;
	GrB_Index blocks_per_col;
	LAGRAPH_OK(BlockMatrix_nrows(&nrows, A));
	LAGRAPH_OK(BlockMatrix_ncols(&ncols, A));
	LAGRAPH_OK(GrB_Matrix_nvals(&nvals, A->s));
	LAGRAPH_OK(BlockMatrix_BlocksPerRow(&blocks_per_row, A));
	LAGRAPH_OK(BlockMatrix_BlocksPerColumn(&blocks_per_col, A));
	LAGRAPH_OK(BlockMatrix_new(&dup, A->t, nrows, ncols, blocks_per_row * blocks_per_col));

	GrB_Index I[nvals];
	GrB_Index J[nvals];
	LAGRAPH_OK(GrB_Matrix_extractTuples_BOOL(I, J, NULL, &nvals, A->s));

	for(GrB_Index i = 0; i < nvals; i++) {
		// Duplicate block.
		GrB_Matrix block;
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		LAGRAPH_OK(BlockMatrix_GetBlock(&block, A, row, col));

		GrB_Matrix dup_block;
		LAGRAPH_OK(GrB_Matrix_dup(&dup_block, block));
		LAGRAPH_OK(BlockMatrix_SetBlock(dup, dup_block, row, col));
	}

	// Set output.
	*C = dup;
	return GrB_SUCCESS;
}
