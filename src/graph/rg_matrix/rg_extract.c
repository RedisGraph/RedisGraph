/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"

GrB_Info RG_Matrix_extractElement_BOOL     // x = A(i,j)
(
    bool *x,                               // extracted scalar
    const RG_Matrix A,                     // matrix to extract a scalar from
    GrB_Index i,                           // row index
    GrB_Index j                            // column index
) {
	ASSERT(x != NULL);
	ASSERT(A != NULL);

	GrB_Info info;
	GrB_Matrix  m      =  RG_MATRIX_M(A);
	GrB_Matrix  dp     =  RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix  dm     =  RG_MATRIX_DELTA_MINUS(A);

	// if 'delta-plus' exists return dp[i,j]
	info = GrB_Matrix_extractElement(x, dp, i, j);
	if(info == GrB_SUCCESS) {
		return info;
	}

	// if dm[i,j] doesn't exists, return M[i,j]
	info = GrB_Matrix_extractElement(x, dm, i, j);
	if(info != GrB_SUCCESS) {
		info = GrB_Matrix_extractElement(x, m, i, j);
		return info;
	}

	return info;
}

GrB_Info RG_Matrix_extractElement_UINT64   // x = A(i,j)
(
    uint64_t *x,                           // extracted scalar
    const RG_Matrix A,                     // matrix to extract a scalar from
    GrB_Index i,                           // row index
    GrB_Index j                            // column index
) {
	ASSERT(x != NULL);
	ASSERT(A != NULL);

	GrB_Info info;
	GrB_Matrix  m      =  RG_MATRIX_M(A);
	GrB_Matrix  dp     =  RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix  dm     =  RG_MATRIX_DELTA_MINUS(A);

	// if 'delta-plus' exists return dp[i,j]
	info = GrB_Matrix_extractElement(x, dp, i, j);
	if(info == GrB_SUCCESS) {
		return info;
	}

	// if dm[i,j] doesn't exists, return M[i,j]
	info = GrB_Matrix_extractElement(x, dm, i, j);
	if(info != GrB_SUCCESS) {
		info = GrB_Matrix_extractElement(x, m, i, j);
		return info;
	}

	return info;
}

