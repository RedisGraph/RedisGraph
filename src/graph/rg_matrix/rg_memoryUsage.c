/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"

GrB_Info RG_Matrix_memoryUsage  // return # of bytes used for a matrix
(
    size_t *size,               // # of bytes used by the matrix A
    const RG_Matrix A           // matrix to query
) {

	ASSERT(A    != NULL);
	ASSERT(size != NULL);

	// reset size
	*size = 0;

	GrB_Info info;
	size_t n       = 0;  // A's  memory consumption
	size_t M_size  = 0;  // M's  memory consumption
	size_t DP_size = 0;  // DP's memory consumption
	size_t DM_size = 0;  // DM's memory consumption

	GrB_Matrix M  = RG_MATRIX_M(A);
	GrB_Matrix DP = RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix DM = RG_MATRIX_DELTA_MINUS(A);

	// account for transpose matrix memory usage
	if(RG_MATRIX_MAINTAIN_TRANSPOSE(A)) {
		info = RG_Matrix_memoryUsage(&n, A->transposed);
		if(info != GrB_SUCCESS) {
			return info;
		}
	}

	//--------------------------------------------------------------------------
	// compute memory consumption for each internal matrix
	//--------------------------------------------------------------------------

	// M's memory consumption
	info = GxB_Matrix_memoryUsage(&M_size, M);
	if(info != GrB_SUCCESS) {
		return info;
	}
	n += M_size;

	// DP's memory consumption
	info = GxB_Matrix_memoryUsage(&DP_size, DP);
	if(info != GrB_SUCCESS) {
		return info;
	}
	n += DP_size;

	// DM's memory consumption
	info = GxB_Matrix_memoryUsage(&DM_size, DM);
	if(info != GrB_SUCCESS) {
		return info;
	}
	n += DM_size;

	// set output
	*size = n;

	return GrB_SUCCESS;
}

