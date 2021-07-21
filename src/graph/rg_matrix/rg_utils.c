/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"

// check if i and j are within matrix boundries
// i < nrows
// j < ncols
void RG_Matrix_checkBounds
(
	const RG_Matrix C,
	GrB_Index i,
	GrB_Index j
) {
#if RG_DEBUG
	GrB_Matrix m = RG_MATRIX_M(C);
	// check bounds
	GrB_Index nrows;
	GrB_Index ncols;
	GrB_Matrix_nrows(&nrows, m);
	GrB_Matrix_ncols(&ncols, m);
	ASSERT(i < nrows);
	ASSERT(j < ncols);
#endif
}

