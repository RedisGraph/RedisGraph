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

GrB_Info BlockMatrix_ncols  // get the number of columns of a matrix
(
	GrB_Index *ncols,       // matrix has ncols
	const BlockMatrix B     // matrix to query
) {
	GrB_Info info;
	if(ncols == NULL || B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_ncols: invalid inputs", GrB_NULL_POINTER);
	}

	GrB_Index bncols;
	LAGRAPH_OK(GrB_Matrix_ncols(&bncols, B->s));

	// ncols = Number of columns in a single block * number of columns in structure matrix.
	*ncols = B->block_ncols * bncols;
	return GrB_SUCCESS;
}
