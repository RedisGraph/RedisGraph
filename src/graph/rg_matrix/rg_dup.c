/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_matrix.h"
#include "rg_utils.h"

// make an exact copy of a matrix
GrB_Info RG_Matrix_dup
(
	RG_Matrix *C,      // handle of output matrix to create
	const RG_Matrix A  // input matrix to copy
)
{
	// validations
	ASSERT(C != NULL); 
	ASSERT(A != NULL); 

	GrB_Info   info;
	RG_Matrix  C_t = NULL;

	if(RG_MATRIX_MAINTAIN_TRANSPOSE(A)) {
		// duplicate transpose matrix
		info = RG_Matrix_dup(&C_t, A->transposed);
		ASSERT(info == GrB_SUCCESS);
	}

	// allocate duplicate matrix
	GrB_Type  type;
	GrB_Index nrows;
	GrB_Index ncols;

	RG_Matrix_type(&type,   A);
	RG_Matrix_nrows(&nrows, A);
	RG_Matrix_ncols(&ncols, A);

	RG_Matrix clone;
	info = RG_Matrix_new(&clone, type, nrows, ncols);
	ASSERT(info == GrB_SUCCESS);

	GrB_Matrix  A_m            = RG_MATRIX_M(A);
	GrB_Matrix  A_delta_plus   = RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix  A_delta_minus  = RG_MATRIX_DELTA_MINUS(A);

	GrB_Matrix  C_m;
	GrB_Matrix  C_delta_plus;
	GrB_Matrix  C_delta_minus;

	info = GrB_Matrix_dup(&C_m, A_m);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_dup(&C_delta_plus, A_delta_plus);
	ASSERT(info == GrB_SUCCESS);

	info = GrB_Matrix_dup(&C_delta_minus, A_delta_minus);
	ASSERT(info == GrB_SUCCESS);

	//--------------------------------------------------------------------------
	// overwrite C's internal matrices
	//--------------------------------------------------------------------------

	// free clone's original empty internal matrices
	info = GrB_free(&(RG_MATRIX_M(clone)));
	ASSERT(info == GrB_SUCCESS);

	info = GrB_free(&(RG_MATRIX_DELTA_PLUS(clone)));
	ASSERT(info == GrB_SUCCESS);

	info = GrB_free(&(RG_MATRIX_DELTA_MINUS(clone)));
	ASSERT(info == GrB_SUCCESS);

	// assign clone's new internal matrices
	clone->matrix = C_m;
	clone->delta_plus = C_delta_plus;
	clone->delta_minus = C_delta_minus;

	if(RG_MATRIX_MAINTAIN_TRANSPOSE(A)) {
		// assign clone's transposed matrix
		RG_Matrix_free(&clone->transposed);
		clone->transposed = C_t;
	}

	*C = clone;

    return GrB_SUCCESS;
}

