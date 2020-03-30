/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL    \
{                           \
    if(I) {                 \
        free(I);            \
    }                       \
    if(J) {                 \
	    free(J);            \
    }                       \
}

GrB_Info BlockMatrix_flatten    // populate GraphBLAS matrix with the contents of block matrix
(
	GrB_Matrix *C,              // input/output matrix for results
	const BlockMatrix A         // first input: block matrix A
) {
	GrB_Info info;
	GrB_Index *I = NULL;
	GrB_Index *J = NULL;
	GrB_Index row_range[2] = {0};
	GrB_Index col_range[2] = {0};

	if(C == NULL || A == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_flatten: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index nblocks;
	LAGRAPH_OK(GrB_Matrix_nvals(&nblocks, A->s));
	I = malloc(sizeof(GrB_Index) * nblocks);
	J = malloc(sizeof(GrB_Index) * nblocks);
	if(I == NULL || J == NULL) {
		LAGRAPH_ERROR("out of memory", GrB_OUT_OF_MEMORY);
	}

	GrB_Type type;
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix flat;
	LAGRAPH_OK(BlockMatrix_type(&type, A));
	LAGRAPH_OK(BlockMatrix_nrows(&nrows, A));
	LAGRAPH_OK(BlockMatrix_ncols(&ncols, A));
	LAGRAPH_OK(GrB_Matrix_new(&flat, type, nrows, ncols));
	LAGRAPH_OK(GrB_Matrix_extractTuples_BOOL(I, J, NULL, &nblocks, A->s));

	// For each block.
	for(GrB_Index i = 0; i < nblocks; i++) {
		GrB_Matrix block;
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		LAGRAPH_OK(BlockMatrix_GetBlock(&block, A, row, col));
		if(block == NULL) {
			LAGRAPH_ERROR("missing block", GrB_PANIC);
		}

		GrB_Index row_offset = A->block_nrows * row;
		GrB_Index col_offset = A->block_ncols * col;
		row_range[0] = row_offset;
		row_range[1] = MAX(0, (row_offset + A->block_nrows - 1));
		col_range[0] = col_offset;
		col_range[1] = MAX(0, (col_offset + A->block_ncols - 1));
		// C(I,J)<Mask> = accum (C(I,J),A)
		LAGRAPH_OK(GxB_Matrix_subassign(flat, GrB_NULL, GrB_NULL, block, row_range, GxB_RANGE, col_range,
										GxB_RANGE, GrB_NULL));
	};

	*C = flat;
	return GrB_SUCCESS;
}
