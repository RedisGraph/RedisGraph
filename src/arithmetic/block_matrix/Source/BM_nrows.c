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

GrB_Info BlockMatrix_nrows  // get the number of rows of a matrix
(
	GrB_Index *nrows,       // matrix has nrows rows
	const BlockMatrix B     // matrix to query
) {
	GrB_Info info;
	if(nrows == NULL || B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_nrows: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index bnrows;
	LAGRAPH_OK(GrB_Matrix_nrows(&bnrows, B->s));

	// nrows = Number of rows in a single block * number of rows in structure matrix.
	*nrows = B->block_nrows * bnrows;
	return GrB_SUCCESS;
}
