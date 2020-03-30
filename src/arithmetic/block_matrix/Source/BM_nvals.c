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

GrB_Info BlockMatrix_nvals  // get the number of entries in a matrix
(
	GrB_Index *nvals,       // matrix has nvals entries
	const BlockMatrix B     // matrix to query
) {
	GrB_Info info;
	if(nvals == NULL || B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_nvals: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index n = 0;
	GrB_Index total_nvals = 0;
	LAGRAPH_OK(GrB_Matrix_nvals(&n, B->s));

	GrB_Index I[n];
	GrB_Index J[n];
	LAGRAPH_OK(GrB_Matrix_extractTuples_BOOL(I, J, NULL, &n, B->s));

	for(GrB_Index i = 0; i < n; i++) {
		// Duplicate block.
		GrB_Matrix block;
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		GrB_Index block_nvals;
		LAGRAPH_OK(BlockMatrix_GetBlock(&block, B, row, col));
		LAGRAPH_OK(GrB_Matrix_nvals(&block_nvals, block));
		total_nvals += block_nvals;
	}

	*nvals = total_nvals;
	return GrB_SUCCESS;
}
