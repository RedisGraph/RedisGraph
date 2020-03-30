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

GrB_Info BlockMatrix_clear  // clear a matrix of all entries;
(                           // type and dimensions remain unchanged
	BlockMatrix A           // matrix to clear
) {
	GrB_Info info;
	if(A == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_clear: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index nvals;
	LAGRAPH_OK(GrB_Matrix_nvals(&nvals, A->s));
	if(nvals == 0) {
		return GrB_SUCCESS;
	}

	GrB_Index I[nvals];
	GrB_Index J[nvals];
	LAGRAPH_OK(GrB_Matrix_extractTuples_BOOL(I, J, NULL, &nvals, A->s));
	LAGRAPH_OK(GrB_Matrix_clear(A->s));

	for(GrB_Index i = 0; i < nvals; i++) {
		GrB_Matrix block;
		GrB_Index row = I[i];
		GrB_Index col = J[i];
		LAGRAPH_OK(BlockMatrix_SetBlock(A, GrB_NULL, row, col));
	}

	return GrB_SUCCESS;
}
