/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_utils.h"
#include "rg_matrix.h"

GrB_Info RG_Matrix_setElement_BOOL      // C (i,j) = x
(
    RG_Matrix C,                        // matrix to modify
    bool x,                             // scalar to assign to C(i,j)
    GrB_Index i,                        // row index
    GrB_Index j                         // column index
) {
	ASSERT(C != NULL);
	ASSERT(!C->multi_edge); // boolean matrices can't support multi-edge
	RG_Matrix_checkBounds(C, i, j);

	bool v;
	GrB_Info info;

	GrB_Matrix m  = RG_MATRIX_M(C);
	GrB_Matrix dp = RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix dm = RG_MATRIX_DELTA_MINUS(C);

	bool  already_allocated    =  false;  // M[i,j] exists
	bool  marked_for_deletion  =  false;  // dm[i,j] exists
	bool  marked_for_addition  =  false;  // dp[i,j] exists

	if(C->maintain_transpose) {
		info = RG_Matrix_setElement_BOOL(C->transposed, x, j, i);
		ASSERT(info == GrB_SUCCESS);
	}

	info = GrB_Matrix_extractElement(&v, m, i, j);
	already_allocated = (info == GrB_SUCCESS);

	info = GrB_Matrix_extractElement(&v, dm, i, j);
	marked_for_deletion = (info == GrB_SUCCESS);

	info = GrB_Matrix_extractElement(&v, dp, i, j);
	marked_for_addition = (info == GrB_SUCCESS);

	// entry can't be marked for both addition and deletion
	ASSERT((!(marked_for_addition && marked_for_deletion)));

	// entry already exists
	if(already_allocated || marked_for_addition) {
		return GrB_SUCCESS;
	}

	// delta-plus disjoint to delta-minus
	// either both dp[i,j] and dm[i,j] are empty
	// or only one of them is active
	ASSERT((!marked_for_addition && !marked_for_addition) ||
		   (marked_for_addition != marked_for_addition));

	if(marked_for_deletion) {
		// unset delta-minus
		info = GrB_Matrix_removeElement(dm, i, j);
		ASSERT(info == GrB_SUCCESS);

		// restore m
		info = GrB_Matrix_setElement(m, x, i, j);
		ASSERT(info == GrB_SUCCESS);
	} else {
		// add entry to delta-plus
		info = GrB_Matrix_setElement_BOOL(dp, x, i, j);
		ASSERT(info == GrB_SUCCESS);
	}

#ifdef RG_DEBUG
	RG_Matrix_validateState(C, i, j);
#endif

	RG_Matrix_setDirty(C);

	return info;
}

