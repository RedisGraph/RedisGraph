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

GrB_Info BlockMatrix_free   // free a matrix
(
	BlockMatrix *B          // handle of matrix to free
) {
	GrB_Info info;
	if(B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_free: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Matrix sub;
	GrB_Index i = 0;
	GrB_Index j = 0;
	GrB_Index nblocks;

	LAGRAPH_OK(BlockMatrix_nblocks(&nblocks, *B));

	GrB_Index I[nblocks];
	GrB_Index J[nblocks];

	LAGRAPH_OK(GrB_Matrix_extractTuples_BOOL(I, J, GrB_NULL, &nblocks, (*B)->s));

	// Free individual blocks.
	for(GrB_Index i = 0; i < nblocks; i++) {
		GrB_Matrix block;
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		LAGRAPH_OK(BlockMatrix_GetBlock(&block, *B, row, col));
		LAGRAPH_OK(GrB_free(&block));
	}

	LAGRAPH_OK(GrB_free(&((*B)->s)));    // Free structure matrix.
	LAGRAPH_FREE((*B)->blocks);       // Free blocks array.
	LAGRAPH_FREE(*B);

	return GrB_SUCCESS;
}
