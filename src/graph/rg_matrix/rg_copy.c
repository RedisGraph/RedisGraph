/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#include "RG.h"
#include "rg_utils.h"
#include "rg_matrix.h"
#include "../../util/rmalloc.h"

static void _copyMatrix
(
	const GrB_Matrix in,
	GrB_Matrix out
) {
	GrB_Index   nvals;
	GrB_Info    info =  GrB_SUCCESS;

	UNUSED(info);

	info = GrB_Matrix_nvals(&nvals, in);
	ASSERT(info == GrB_SUCCESS);

	if(nvals > 0) {
		info = GrB_Matrix_apply(out, NULL, NULL, GrB_IDENTITY_BOOL, in,
								GrB_DESC_R);
	} else {
		GrB_Matrix_clear(out);
	}

	ASSERT(info == GrB_SUCCESS);
}

GrB_Info RG_Matrix_copy
(
	RG_Matrix C,
	const RG_Matrix A
) {
	RG_Matrix_checkCompatible(C, A);

	GrB_Matrix  in_m             =  RG_MATRIX_M(A);
	GrB_Matrix  out_m            =  RG_MATRIX_M(C);
	GrB_Matrix  in_delta_plus    =  RG_MATRIX_DELTA_PLUS(A);
	GrB_Matrix  in_delta_minus   =  RG_MATRIX_DELTA_MINUS(A);
	GrB_Matrix  out_delta_plus   =  RG_MATRIX_DELTA_PLUS(C);
	GrB_Matrix  out_delta_minus  =  RG_MATRIX_DELTA_MINUS(C);

	_copyMatrix(in_m, out_m);
	_copyMatrix(in_delta_plus, out_delta_plus);
	_copyMatrix(in_delta_minus, out_delta_minus);

	if(A->transposed) {
		GrB_Index nrows;
		GrB_Matrix_nrows(&nrows, in_m);
		RG_Matrix_new(&C->transposed, GrB_BOOL, nrows, nrows);
		RG_Matrix_copy(C->transposed, A->transposed);
	}

	return GrB_SUCCESS;
}

