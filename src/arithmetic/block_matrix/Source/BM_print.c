/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "./Utility/utility.h"
#include "../Include/block_matrix.h"

#define LAGRAPH_FREE_ALL     \
{                            \
  GrB_free(&C);              \
}

//------------------------------------------------------------------------------
// BlockMatrix_print: print the contents of a Block matrix
//------------------------------------------------------------------------------
GrB_Info BlockMatrix_fprint     // print and check a BlockMatrix
(
	const BlockMatrix A,        // object to print and check
	GxB_Print_Level pr          // print level
) {
	GrB_Info info;
	GrB_Matrix C = GrB_NULL;
	if(A == NULL) {
		// input arguments invalid
		LAGRAPH_ERROR("BlockMatrix_fprint: invalid inputs", GrB_NULL_POINTER);
	}

	LAGRAPH_OK(BlockMatrix_flatten(&C, A));
	LAGRAPH_OK(GxB_Matrix_fprint(C, "C", pr, stdout));

	LAGRAPH_FREE_ALL
	return GrB_SUCCESS;
}
