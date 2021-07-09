/*
* Copyright 2018-2021 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"

GrB_Info RG_Matrix_resize       // change the size of a matrix
(
    RG_Matrix C,                // matrix to modify
    GrB_Index nrows_new,        // new number of rows in matrix
    GrB_Index ncols_new         // new number of columns in matrix
) {
	ASSERT(C != NULL);
	GrB_Info info;

	if(C->maintain_transpose) {
		info = RG_Matrix_resize(C->transposed, nrows_new, ncols_new);
		ASSERT(info == GrB_SUCCESS);
	}

	GrB_Matrix  m            =  RG_MATRIX_MATRIX(C);
	GrB_Matrix  delta_plus   =  RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix  delta_minus  =  RG_MATRIX_DELTA_MINUS(C);

	info = GrB_Matrix_resize(m, nrows_new, ncols_new);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_resize(delta_plus, nrows_new, ncols_new);
	ASSERT(info == GrB_SUCCESS);
	
	info = GrB_Matrix_resize(delta_minus, nrows_new, ncols_new);
	ASSERT(info == GrB_SUCCESS);
	
	return info;
}

