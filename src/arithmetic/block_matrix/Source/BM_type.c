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

GrB_Info BlockMatrix_type   // get the type of a matrix
(
	GrB_Type *type,         // returns the type of the matrix
	const BlockMatrix B     // matrix to query
) {
	GrB_Info info;
	if(type == NULL || B == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_type: invalid inputs", GrB_NULL_POINTER);
	}

	*type = B->t;
	return GrB_SUCCESS;
}
