/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"

GrB_Info RG_Matrix_setElement_UINT64    // C (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    uint64_t x,                         // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) {
	ASSERT(C != NULL);

	GrB_Info info;

	if(C->maintain_transpose) {
		info = RG_Matrix_setElement_UINT64(C->transposed, x, j, i);
		ASSERT(info == GrB_SUCCESS);
	}

	GrB_Matrix m  = RG_MATRIX_MATRIX(C);
	GrB_Matrix dp = RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix dm = RG_MATRIX_DELTA_MINUS(C);

	// check if entry is marked for deletion
	uint64_t v;
	info = GrB_Matrix_extractElement(&v, dm, i, j);	
	if(info == GrB_SUCCESS) {
		// remove entry and issue deletion
		info = GrB_Matrix_removeElement(dm, i, j);
		ASSERT(info == GrB_SUCCESS);

		// TODO: delete entity!
		ASSERT(false);
	}

	// check for multi edge
	if(C->multi_edge) {
		info = GrB_Matrix_extractElement_UINT64(&v, m, i, j);
		if(info == GrB_SUCCESS) {
			// TODO: support multi edge
			ASSERT(false);
		}
		info = GrB_Matrix_extractElement_UINT64(&v, dp, i, j);
		if(info == GrB_SUCCESS) {
			// TODO: support multi edge
			ASSERT(false);
		}

		// TODO: force flush (cheap, no memory movement)
		return info;
	}

	// add entry to delta-plus
	info = GrB_Matrix_setElement_UINT64(dp, x, i, j);
	ASSERT(info == GrB_SUCCESS);
	RG_Matrix_setDirty(C);

	return info;
}

