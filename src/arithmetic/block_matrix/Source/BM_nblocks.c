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

GrB_Info BlockMatrix_nblocks
(
	GrB_Index *nblocks,       // number of active blocks in matrix
	const BlockMatrix B       // matrix to query
) {
	GrB_Info info;
	if(B == NULL || nblocks == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_nblocks: invalid inputs", GrB_NULL_POINTER);
	}

	LAGRAPH_OK(GrB_Matrix_nvals(nblocks, B->s));
	return GrB_SUCCESS;
}
